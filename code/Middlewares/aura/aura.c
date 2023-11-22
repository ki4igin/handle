#include "aura.h"
#include "assert.h"
#include "crc16.h"
#include "keys.h"
#include "locker.h"
#include "rfid.h"
#include "uid_hash.h"
#include "usart_ex.h"

#define AURA_HANDLE_ID     7
#define AURA_MAX_DATA_SIZE 128

struct header {
    const uint32_t protocol;
    uint32_t cnt;
    uint32_t uid_src;
    uint32_t uid_dest;
    uint16_t func;
    uint16_t data_size;
};

enum func {
    FUNC_REQ_WHOAMI,
    FUNC_RESP_WHOAMI,
    FUNC_REQ_STATUS,
    FUNC_RESP_STATUS,
    FUNC_WRITE_DATA,
    FUNC_READ_DATA,
};

static_assert(sizeof(struct header) == 20, "Error in struct header");

enum chunk_data_type {
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
    CHUNK_TYPE_I8_ARR = 10,
    CHUNK_TYPE_U8_ARR = 11,
    CHUNK_TYPE_I16_ARR = 12,
    CHUNK_TYPE_U16_ARR = 13,
    CHUNK_TYPE_I32_ARR = 14,
    CHUNK_TYPE_U32_ARR = 15,
    CHUNK_TYPE_F32_ARR = 16,
    CHUNK_TYPE_f64_ARR = 17,

    CHUNK_TYPE_UID_CARD = 18,
    CHUNK_TYPE_UID_CARD_ARR = 19,
};

enum chunk_id {
    CHUNK_ID_TYPE_SENSOR = 1,
    CHUNK_ID_UIDS_ARRAY = 2,
    CHUNK_ID_STATUS_LOCKER = 3,
    CHUNK_ID_UID_CARD = 4,

    // CHUNK_ID_ADD_CARDS = 2,
    // CHUNK_ID_VIEW_CARDS = 3,
    // CHUNK_ID_CLEAR_CARDS = 4,
    // CHUNK_ID_OPEN_LOCKER = 5,
};

struct chunk_head {
    uint8_t id;
    uint8_t type;
    uint16_t data_size;
};

struct chunk_u16 {
    struct chunk_head head;
    uint16_t data;
};

struct chunk_u32 {
    struct chunk_head head;
    uint32_t data;
};

struct chunk_card_uid {
    struct chunk_head head;
    union rfid_card_uid data;
};

struct data_view {
    uint8_t offset;
    uint8_t count;
};

typedef uint16_t data_add_cards_t;
typedef uint16_t data_clear_cards_t;

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

uint32_t flags_pack_received = 0;
uint32_t uid = 0;

static struct {
    struct header header;
    uint8_t data[AURA_MAX_DATA_SIZE];
    crc16_t crc;
} pack __ALIGNED(8);

enum state_recv {
    STATE_RECV_START,
    STATE_RECV_HEADER,
    STATE_RECV_CRC,
};

static enum state_recv state_recv = STATE_RECV_START;

static void aura_recv_package(void)
{
    state_recv = STATE_RECV_START;
    uart_recv_dma(&pack, sizeof(struct header));
}

static void aura_create_whoami(void)
{
    pack.header.func = FUNC_RESP_WHOAMI;
    pack.header.data_size = sizeof(struct chunk_u32);
    struct chunk_u32 *c = (struct chunk_u32 *)pack.data;
    // c->head.id = CHUNK_ID_TYPE_SENSOR;
    // c->head.type = CHUNK_TYPE_U32;
    // c->head.data_size = sizeof(uint32_t);
    // c->data = AURA_HANDLE_ID;
    *c = (struct chunk_u32){
        .head = {
                 .id = CHUNK_ID_TYPE_SENSOR,
                 .type = CHUNK_TYPE_U32,
                 .data_size = sizeof(c->data),
                 },
        .data = AURA_HANDLE_ID,
    };
}

static void aura_create_status(void)
{
    pack.header.func = FUNC_RESP_STATUS;
    pack.header.data_size = sizeof(struct chunk_u16)
                          + sizeof(struct chunk_card_uid);
    struct chunk_u16 *c16 = (struct chunk_u16 *)pack.data;
    // c->head.id = CHUNK_ID_TYPE_SENSOR;
    // c->head.type = CHUNK_TYPE_U32;
    // c->head.data_size = sizeof(uint32_t);
    // c->data = AURA_HANDLE_ID;
    *c16++ = (struct chunk_u16){
        .head = {
                 .id = CHUNK_ID_STATUS_LOCKER,
                 .type = CHUNK_TYPE_U16,
                 .data_size = sizeof(c16->data),
                 },
        .data = locker_is_open() ? 0x00FF : 0x0000,
    };
    struct chunk_card_uid *c_uid = (struct chunk_card_uid *)c16;
    *c_uid++ = (struct chunk_card_uid){
        .head = {
                 .id = CHUNK_ID_UID_CARD,
                 .type = CHUNK_TYPE_UID_CARD,
                 .data_size = sizeof(c_uid->data),
                 },
        .data = rfid_card_uid,
    };
}

// static data_add_cards_t add_cards(void)
// {
//     uint32_t count = pack.chunk.size / sizeof(union rfid_card_uid);
//     data_add_cards_t data_add_cards = keys_get_count();
//     union rfid_card_uid *uid = (union rfid_card_uid *)pack.data;
//     for (uint32_t i = 0; i < count; i++) {
//         data_add_cards &= ~0xFF;
//         data_add_cards |= key_save(uid++);
//     }
//     return data_add_cards;
// }

// static struct keys_range view_cards(void)
// {
//     struct data_view *d = (struct data_view *)pack.data;
//     return keys_get_cards(d->offset, d->count);
// }

#define RESP_BUFS_COUNT 3

static struct resp {
    uint32_t cnt;

    struct buf {
        void *p;
        uint32_t size;
    } buf[RESP_BUFS_COUNT];
} resp;

// static void aura_create_resp_view_cards(struct keys_range keys)
// {
//     pack.chunk.type = CHUNK_TYPE_U8;
//     pack.chunk.size = keys.size;
//     uint32_t header_chunk_size = sizeof(struct header) + sizeof(struct chunk);
//     crc16_t crc = crc16_calc(&pack, header_chunk_size);
//     pack.crc = crc16_calc_continue(crc, keys.p, keys.size);
//     // clang-format off
//     resp = (struct resp){
//         .cnt = 3,
//         .buf = {
//             [2] = {.p = &pack, .size = header_chunk_size},
//             [1] = {.p = keys.p, .size = keys.size},
//             [0] = {.p = &pack.crc, .size = sizeof(crc16_t)}
//         }
//     };
//     // clang-format on
// }

// static void aura_create_resp_add_cards(data_add_cards_t data)
// {
//     pack.chunk.type = CHUNK_TYPE_U8;
//     pack.chunk.size = sizeof(data_add_cards_t);
//     *(data_add_cards_t *)pack.data = data;
// }

// static void aura_create_resp_clear_cards(data_clear_cards_t data)
// {
//     pack.chunk.type = CHUNK_TYPE_U8;
//     pack.chunk.size = sizeof(data_clear_cards_t);
//     *(data_clear_cards_t *)pack.data = data;
// }

static void aura_send_response(void)
{
    static uint32_t cnt = 0;
    pack.header.cnt = cnt++;
    pack.header.uid_dest = pack.header.uid_src;
    pack.header.uid_src = uid;

    if (resp.cnt) {
        resp.cnt--;
        uart_send_dma(resp.buf[resp.cnt].p, resp.buf[resp.cnt].size);
    } else {
        uint32_t pack_size =
            sizeof(struct header) + pack.header.data_size + sizeof(crc16_t);
        crc16_add2pack(&pack, pack_size);
        uart_send_dma(&pack, pack_size);
    }
}

void aura_cmd_process(void)
{
    if (flags_pack_received == 0) {
        return;
    }
    flags_pack_received = 0;

    enum func f = pack.header.func;
    switch (f) {
    case FUNC_REQ_WHOAMI: {
        aura_create_whoami();
    } break;
    case FUNC_REQ_STATUS: {
        aura_create_status();
    } break;
    // case CHUNK_ID_ADD_CARDS: {
    //     data_add_cards_t d = add_cards();
    //     aura_create_resp_add_cards(d);
    // } break;
    // case CHUNK_ID_VIEW_CARDS: {
    //     struct keys_range keys = view_cards();
    //     aura_create_resp_view_cards(keys);
    // } break;
    // case CHUNK_ID_CLEAR_CARDS: {
    //     data_clear_cards_t d = keys_clear();
    //     aura_create_resp_clear_cards(d);
    // } break;
    // case CHUNK_ID_OPEN_LOCKER: {
    //     locker_open();
    //     aura_create_status();
    // } break;
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
        state_recv = STATE_RECV_HEADER;
        uart_recv_dma(pack.data, pack.header.data_size + sizeof(crc16_t));
    } break;
    case STATE_RECV_HEADER: {
        state_recv = STATE_RECV_CRC;
        uart_stop_recv();
        uint32_t pack_size =
            sizeof(struct header) + pack.header.data_size + sizeof(crc16_t);
        if (crc16_is_valid(&pack, pack_size)) {
            flags_pack_received = 1;
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
        uart_send_dma(resp.buf[resp.cnt].p, resp.buf[resp.cnt].size);
    } else {
        aura_recv_package();
    }
}
