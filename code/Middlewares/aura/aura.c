#include "aura.h"
#include "uid_hash.h"
#include "assert.h"
#include "crc16.h"
#include "usart_ex.h"
#include "rfid.h"
#include "locker.h"
#include "keys.h"

#define AURA_HANDLE_ID     7
#define AURA_MAX_DATA_SIZE 128

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
    CHUNK_ID_ADD_CARDS = 2,
    CHUNK_ID_VIEW_CARDS = 3,
    CHUNK_ID_CLEAR_CARDS = 4,
    CHUNK_ID_OPEN_LOCKER = 5,
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

struct data_view {
    uint8_t offset;
    uint8_t count;
};

typedef uint16_t data_add_cards_t;
typedef uint8_t data_clear_cards_t;

// struct data_add_cards {
//     uint8_t cnt;

//     union {
//         struct {
//             uint8_t dbl  :1;
//             uint8_t over :1;
//         };

//         uint8_t val;
//     } err;
// };

struct data_status {
    uint8_t is_open;
    uint8_t card_uid[];
};

uint32_t aura_flags_pack_received = 0;
uint32_t uid = 0;

static struct {
    struct header header;
    struct chunk chunk;
    uint8_t data[AURA_MAX_DATA_SIZE];
    crc16_t crc;
} package __ALIGNED(8);

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
    package.chunk.type = CHUNK_TYPE_U8;
    package.chunk.size = sizeof(struct data_whoami);
    *(struct data_whoami *)package.data = (struct data_whoami){
        .id_handle = AURA_HANDLE_ID,
        .uid_repeaters = {0},
    };
}

static void aura_create_status(void)
{
    package.chunk.type = CHUNK_TYPE_U8;
    package.chunk.size = sizeof(struct data_status) + sizeof(rfid_card_uid.val);
    struct data_status *d = (struct data_status *)package.data;
    d->is_open = (uint8_t)locker_is_open();
    for (uint32_t i = 0; i < sizeof(rfid_card_uid.val); i++) {
        d->card_uid[i] = rfid_card_uid.val[i];
    }
}

static data_add_cards_t add_cards(void)
{
    uint32_t count = package.chunk.size / sizeof(union rfid_card_uid);
    data_add_cards_t data_add_cards = keys_get_count();
    union rfid_card_uid *uid = (union rfid_card_uid *)package.data;
    for (uint32_t i = 0; i < count; i++) {
        data_add_cards &= ~0xFF;
        data_add_cards |= key_save(uid++);
    }
    return data_add_cards;
}

static struct keys_range view_cards(void)
{
    struct data_view *d = (struct data_view *)package.data;
    return keys_get_cards(d->offset, d->count);
}

#define RESP_BUFS_COUNT 3

static struct resp {
    uint32_t cnt;

    struct buf {
        void *p;
        uint32_t size;
    } buf[RESP_BUFS_COUNT];
} resp;

static void aura_create_resp_view_cards(struct keys_range keys)
{
    package.chunk.type = CHUNK_TYPE_U8;
    package.chunk.size = keys.size;
    uint32_t header_chunk_size = sizeof(struct header) + sizeof(struct chunk);
    crc16_t crc = crc16_calc(&package, header_chunk_size);
    package.crc = crc16_calc_continue(crc, keys.p, keys.size);
    // clang-format off
    resp = (struct resp){
        .cnt = 3,
        .buf = {
            [2] = {.p = &package, .size = header_chunk_size},
            [1] = {.p = keys.p, .size = keys.size},
            [0] = {.p = &package.crc, .size = sizeof(crc16_t)}
        }
    };
    // clang-format on
}

static void aura_create_resp_add_cards(data_add_cards_t data)
{
    package.chunk.type = CHUNK_TYPE_U8;
    package.chunk.size = sizeof(data_add_cards_t);
    *(data_add_cards_t *)package.data = data;
}

static void aura_create_resp_clear_cards(data_clear_cards_t data)
{
    package.chunk.type = CHUNK_TYPE_U8;
    package.chunk.size = sizeof(data_clear_cards_t);
    *(data_clear_cards_t *)package.data = data;
}

static void aura_send_response(void)
{
    static uint32_t cnt = 0;
    package.header.cnt = cnt++;
    package.header.uid_dest = package.header.uid_src;
    package.header.uid_src = uid;

    if (resp.cnt) {
        resp.cnt--;
        uart_send_array_dma(resp.buf[resp.cnt].p, resp.buf[resp.cnt].size);
    } else {
        uint32_t pack_size = sizeof(struct header)
                           + sizeof(struct chunk)
                           + package.chunk.size
                           + sizeof(crc16_t);
        crc16_add2pack(&package, pack_size);
        uart_send_array_dma(&package, pack_size);
    }
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
    case CHUNK_ID_ADD_CARDS: {
        data_add_cards_t d = add_cards();
        aura_create_resp_add_cards(d);
    } break;
    case CHUNK_ID_VIEW_CARDS: {
        struct keys_range keys = view_cards();
        aura_create_resp_view_cards(keys);
    } break;
    case CHUNK_ID_CLEAR_CARDS: {
        data_clear_cards_t d = keys_clear();
        aura_create_resp_clear_cards(d);
    } break;
    case CHUNK_ID_OPEN_LOCKER: {
        locker_open();
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
    if (resp.cnt) {
        resp.cnt--;
        uart_send_array_dma(resp.buf[resp.cnt].p, resp.buf[resp.cnt].size);
    } else {
        aura_recv_package();
    }
}
