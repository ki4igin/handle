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

uint32_t aura_flags_pack_received = 0;
uint32_t uid = 0;

static struct {
    struct header header;
    struct chunk chunk;
    uint8_t data[128];
} package;

enum state_recv {
    STATE_RECV_START,
    STATE_RECV_CHUNK,
    STATE_RECV_CRC,
};

static enum state_recv state_recv = STATE_RECV_START;

static void aura_recv_package(void)
{
    state_recv = STATE_RECV_START;
    uart_recv_array_dma(&package, sizeof(struct header) + sizeof(struct chunk));
}

static void aura_create_whoami(void)
{
    package.chunk.type = CHUNK_TYPE_U32;
    package.chunk.size = sizeof(struct data_whoami);
    *(struct data_whoami *)package.data = (struct data_whoami){
        .id_handle = 7,
        .uid_repeaters = {0},
    };
}

static void aura_create_status(void)
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

static void aura_send_response(void)
{
    static uint32_t cnt = 0;
    package.header.cnt = cnt++;
    package.header.uid_dest = package.header.uid_src;
    package.header.uid_src = uid;
    uint32_t pack_size = sizeof(struct header)
                       + sizeof(struct chunk)
                       + package.chunk.size
                       + sizeof(crc16_t);
    crc16_add2pack(&package, pack_size);
    uart_send_array_dma(&package, pack_size);
}

void aura_cmd_process(void)
{
    if (aura_flags_pack_received == 0) {
        return;
    }
    aura_flags_pack_received = 0;

    struct chunk *c = &package.chunk;
    switch (c->id) {
    case CHUNK_ID_WHOIM: {
        aura_create_whoami();
    } break;
    case CHUNK_ID_STATUS: {
        aura_create_status();
    } break;
    default: {
        aura_recv_package();
        return;
    }
    }

    aura_send_response();
}

void aura_init(void)
{
    uid = uid_hash();
    aura_recv_package();
}

void uart_recv_dma_callback(void)
{
    switch (state_recv) {
    case STATE_RECV_START: {
        state_recv = STATE_RECV_CHUNK;
        struct chunk *c = &package.chunk;
        uart_recv_array_dma(package.data, c->size + sizeof(crc16_t));
    } break;
    case STATE_RECV_CHUNK: {
        state_recv = STATE_RECV_CRC;
        uart_stop_recv();
        uint32_t pack_size = sizeof(struct header)
                           + sizeof(struct chunk)
                           + package.chunk.size
                           + sizeof(crc16_t);
        if (crc16_is_valid(&package, pack_size)) {
            aura_flags_pack_received = 1;
        } else {
            aura_recv_package();
        }

    } break;
    case STATE_RECV_CRC: {
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
