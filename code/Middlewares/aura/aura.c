#include "aura.h"
#include "uid_hash.h"
#include "assert.h"
#include "crc16.h"
#include "usart_ex.h"
#include "rfid.h"
#include "locker.h"

struct header {
    const uint32_t protocol;
    uint32_t cnt;
    uint32_t uid_src;
    uint32_t uid_dest;
};

static_assert(sizeof(struct header) == 16, "Error in struct header");

enum chunk_type {
    CHUNK_TYPE_NONE = 0,
    CHUNK_TYPE_I8 = 1,
    CHUNK_TYPE_U8 = 2,
    CHUNK_TYPE_I16 = 3,
    CHUNK_TYPE_U16 = 4,
    CHUNK_TYPE_I32 = 5,
    CHUNK_TYPE_U32 = 6,
    CHUNK_TYPE_F32 = 7,
    CHUNK_TYPE_f64 = 8,
    CHUNK_TYPE_STR = 9,
};

enum chunk_id {
    CHUNK_ID_WHOIM = 0,
    CHUNK_ID_STATUS = 1,
};

struct chunk {
    uint8_t id;
    uint8_t type;
    uint16_t size;
};

struct data_whoami {
    uint32_t id_handle;
    uint32_t uid_repeaters[4];
};

struct data_status {
    uint8_t is_open;
    uint8_t card_uid_len;
    uint8_t card_uid[];
};

struct crc16 {
    uint16_t val;
};

uint32_t aura_flags_pack_received = 0;

static struct {
    struct header header;
    struct chunk chunk;
    uint8_t data[128];
} package;

// static_assert(sizeof(struct chunk) == 4, "Error in struct chunk");

// clang-format off
struct package_whoami {
    struct header header;
    uint16_t crc;
} package_whoami = {
    .header = {
        .protocol = 0x41525541U,
        .cnt = 0,
        .uid_src = 0,
        .uid_dest = 0,
    },
    .crc = 0,
};

// clang-format on

uint32_t uid = 0;

void aura_init(void)
{
    uid = uid_hash();
}

void aura_create_whoami(void)
{
    package.chunk.type = CHUNK_TYPE_U32;
    package.chunk.size = sizeof(struct data_whoami);
    *(struct data_whoami *)package.data = (struct data_whoami){
        .id_handle = 7,
        .uid_repeaters = {0},
    };
}

void aura_create_status(void)
{
    package.chunk.type = CHUNK_TYPE_U8;
    package.chunk.size = sizeof(struct data_status) + rfid_card_uid.len;
    struct data_status *d = (struct data_status *)package.data;
    d->is_open = (uint8_t)locker_is_open();
    d->card_uid_len = rfid_card_uid.len;
    for (uint32_t i = 0; i < rfid_card_uid.len; i++) {
        d->card_uid[i] = rfid_card_uid.val[i];
    }
}

void aura_cmd_process(void)
{
    if (aura_flags_pack_received == 0) {
        return;
    }

    aura_flags_pack_received = 0;
    package.header.uid_dest = package.header.uid_src;
    package.header.uid_src = uid;

    struct chunk *c = &package.chunk;
    switch (c->id) {
    case CHUNK_ID_WHOIM: {
        aura_create_whoami();
    } break;
    case CHUNK_ID_STATUS: {
        aura_create_status();
    } break;
    }

    uint32_t pack_size = sizeof(struct header)
                       + sizeof(struct chunk)
                       + package.chunk.size
                       + sizeof(struct crc16);
    crc16_add2pack(&package, pack_size);
    uart_send_array_dma(&package, pack_size);
}

enum state_recv {
    STATE_RECV_START,
    STATE_RECV_CHUNK,
    STATE_RECV_CRC,
    STATE_RECV_NOPE,
};

uint32_t state_recv = STATE_RECV_START;

void aura_recv_package(void)
{
    state_recv = STATE_RECV_START;
    uart_recv_array_dma(&package, sizeof(struct header) + sizeof(struct chunk));
}

void uart_recv_dma_callback(void)
{
    switch (state_recv) {
    case STATE_RECV_START: {
        state_recv = STATE_RECV_CHUNK;
        struct chunk *c = &package.chunk;
        uart_recv_array_dma(c, c->size + sizeof(struct crc16));
    } break;
    case STATE_RECV_CRC: {
        state_recv = STATE_RECV_NOPE;
        uart_stop_recv();
        uint32_t pack_size = sizeof(struct header)
                           + sizeof(struct chunk)
                           + package.chunk.size
                           + sizeof(struct crc16);
        if (crc16_is_valid(&package, pack_size)) {
            aura_flags_pack_received = 1;
        }

    } break;
    case STATE_RECV_NOPE: {
    } break;
    }
}

void uart_recv_timeout_callback(void)
{
    uart_stop_recv();
    aura_recv_package();
}

void uart_send_dma_callback(void)
{
    aura_recv_package();
}
