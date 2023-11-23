#include "aura.h"
#include "assert.h"
#include "crc16.h"
#include "keys.h"
#include "locker.h"
#include "rfid.h"
#include "uid_hash.h"
#include "usart_ex.h"
#include "tools.h"
#include "buf.h"

#define AURA_HANDLE_ID     7
#define AURA_MAX_DATA_SIZE 128

struct header {
    uint32_t protocol;
    uint32_t cnt;
    uint32_t uid_src;
    uint32_t uid_dest;
    uint16_t func;
    uint16_t data_size;
};

enum func {
    FUNC_REQ_WHOAMI = 1,
    FUNC_RESP_WHOAMI = 2,
    FUNC_REQ_STATUS = 3,
    FUNC_RESP_STATUS = 4,
    FUNC_REQ_WRITE_DATA = 5,
    FUNC_RESP_WRITE_DATA = 6,
    FUNC_REQ_READ_DATA = 7,
    FUNC_RESP_READ_DATA = 8,
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

    CHUNK_TYPE_CARD_UID = 18,
    CHUNK_TYPE_CARD_UID_ARR = 19,
    CHUNK_TYPE_CARD_RANGE = 20,
};

enum chunk_id {
    CHUNK_ID_TYPE_SENSOR = 1,
    CHUNK_ID_UIDS_ARRAY = 2,

    CHUNK_ID_ERR,

    CHUNK_ID_STATUS_LOCKER,

    CHUNK_ID_CARD_UID,
    CHUNK_ID_CARD_UID_ARR_WRITE,
    CHUNK_ID_CARD_UID_ARR_READ,
    CHUNK_ID_CARD_RANGE,

    CHUNK_ID_CARD_SAVE_COUNT,
    CHUNK_ID_CARD_CLEAR,
};

const enum chunk_data_type table_type[] = {
    [CHUNK_ID_TYPE_SENSOR] = CHUNK_TYPE_U32,
    [CHUNK_ID_STATUS_LOCKER] = CHUNK_TYPE_U16,
    [CHUNK_ID_CARD_UID] = CHUNK_TYPE_CARD_UID,
    [CHUNK_ID_ERR] = CHUNK_TYPE_U16,
    [CHUNK_ID_CARD_SAVE_COUNT] = CHUNK_TYPE_U16,
    [CHUNK_ID_CARD_RANGE] = CHUNK_TYPE_U16,
    [CHUNK_ID_CARD_UID_ARR_READ] = CHUNK_TYPE_CARD_RANGE,
    [CHUNK_ID_CARD_UID_ARR_WRITE] = CHUNK_TYPE_CARD_UID_ARR,
    [CHUNK_ID_CARD_CLEAR] = CHUNK_TYPE_U16,
};

struct chunk_head {
    uint8_t id;
    uint8_t type;
    uint16_t data_size;
};

struct chunk {
    uint8_t id;
    uint8_t type;
    uint16_t data_size;
    uint16_t data[];
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

struct chunk_card_uid_arr {
    struct chunk_head head;
    union rfid_card_uid data[];
};

uint32_t flags_pack_received = 0;
uint32_t uid = 0;

static struct {
    struct header header;
    uint8_t data[AURA_MAX_DATA_SIZE];
    crc16_t crc;
    void *next_chunk;
} pack_req, pack_resp = {
                .header.protocol = 0x41525541U,
};

enum state_recv {
    STATE_RECV_START,
    STATE_RECV_HEADER,
    STATE_RECV_CRC,
};

static enum state_recv state_recv = STATE_RECV_START;

static void aura_recv_package(void)
{
    state_recv = STATE_RECV_START;
    uart_recv_dma(&pack_req, sizeof(struct header));
}

#define RESP_BUFS_COUNT 3

static struct buf_list resp_list;

static void *add_chunk_head(void **chunk, enum chunk_id id, uint16_t size)
{
    struct chunk *head = (struct chunk *)*chunk;
    *chunk = (void *)((uint32_t)*chunk + size + sizeof(*head));
    head->id = id;
    head->type = table_type[id];
    head->data_size = size;
    return head->data;
}

static void add_chunk(void **chunk, enum chunk_id id, uint16_t size, void *data)
{
    void *chunk_data = add_chunk_head(chunk, id, size);
    memcpy_u16(data, chunk_data, size);
}

static void add_chunk_u16(void **chunk, enum chunk_id id, uint16_t val)
{
    void *data = add_chunk_head(chunk, id, sizeof(val));
    *(uint16_t *)data = val;
}

static void add_chunk_u32(void **chunk, enum chunk_id id, uint32_t val)
{
    void *data = add_chunk_head(chunk, id, sizeof(val));
    *(uint32_t *)data = val;
}

static void add_chunk_card_uid(void **chunk, enum chunk_id id, union rfid_card_uid val)
{
    void *data = add_chunk_head(chunk, id, sizeof(val));
    *(union rfid_card_uid *)__UNALIGNED_UINT32_READ(data) = val;
}

static void add_resp_card_uid_arr(void **chunk, enum chunk_id id, struct keys_range val)
{
    add_chunk_head(chunk, id, val.size);
}

// static void aura_create_resp_clear_cards(data_clear_cards_t data)
// {
//     pack.chunk.type = CHUNK_TYPE_U8;
//     pack.chunk.size = sizeof(data_clear_cards_t);
//     *(data_clear_cards_t *)pack.data = data;
// }

static void aura_send_response(uint32_t data_size)
{
    static uint32_t cnt = 0;
    pack_resp.header.cnt = cnt++;
    pack_resp.header.uid_dest = pack_req.header.uid_src;
    pack_resp.header.data_size = data_size;

    if (resp_list.count) {
        crc16_add2list(&resp_list);
        struct buf *b = buf_list_get_next(&resp_list);
        uart_send_dma(b->p, b->size);
    } else {
        uint32_t pack_size = sizeof(struct header) + data_size + sizeof(crc16_t);
        crc16_add2pack(&pack_resp, pack_size);
        uart_send_dma(&pack_resp, pack_size);
    }
}

static uint32_t cmd_whoami(void)
{
    void *next_resp_chunk = pack_resp.data;
    add_chunk_u32(&next_resp_chunk, CHUNK_ID_TYPE_SENSOR, AURA_HANDLE_ID);
    return (uint32_t)next_resp_chunk - (uint32_t)pack_resp.data;
}

static uint32_t cmd_status(void)
{
    void *next_chunk = pack_resp.data;
    uint16_t data = locker_is_open() ? 0x00FF : 0x0000;
    add_chunk_u16(&next_chunk, CHUNK_ID_STATUS_LOCKER, data);
    // add_chunk_card_uid(&next_chunk, CHUNK_ID_CARD_UID, rfid_card_uid);
    // add_chunk(&next_chunk, CHUNK_ID_STATUS_LOCKER, sizeof(data), &data);
    add_chunk(&next_chunk, CHUNK_ID_CARD_UID, sizeof(rfid_card_uid), &rfid_card_uid);
    return (uint32_t)next_chunk - (uint32_t)pack_resp.data;
}

static uint32_t cmd_write_data()
{
    int32_t req_data_size = pack_req.header.data_size;
    void *next_resp_chunk = pack_resp.data;
    void *next_req_chunk = pack_req.data;
    while (req_data_size > 0) {
        struct chunk_head *ch = (struct chunk_head *)next_req_chunk;
        uint32_t chunk_size = ch->data_size + sizeof(struct chunk_head);
        next_req_chunk = (void *)((uint32_t)next_req_chunk + chunk_size);
        req_data_size -= chunk_size;

        switch (ch->id) {
        case CHUNK_ID_CARD_UID_ARR_WRITE: {
            struct chunk_card_uid_arr *c = (struct chunk_card_uid_arr *)ch;
            uint32_t count = c->head.data_size / sizeof(union rfid_card_uid);
            struct keys_res res = keys_save(c->data, count);
            add_chunk_u16(&next_resp_chunk, CHUNK_ID_ERR, res.err);
            add_chunk_u16(&next_resp_chunk, CHUNK_ID_CARD_SAVE_COUNT, res.val);
        } break;
        case CHUNK_ID_STATUS_LOCKER: {
            struct chunk_u16 *c = (struct chunk_u16 *)ch;
            if (c->data == 0x00FF) {
                locker_close();
            } else if (c->data == 0x0000) {
                locker_open();
            }
            uint16_t data = locker_is_open() ? 0x00FF : 0x0000;
            add_chunk_u16(&next_resp_chunk, CHUNK_ID_STATUS_LOCKER, data);
        } break;
        case CHUNK_ID_CARD_CLEAR: {
            struct chunk_u16 *c = (struct chunk_u16 *)ch;
            if (c->data == 0x00FF) {
                keys_clear();
            }
            uint16_t data = keys_get_count();
            add_chunk_u16(&next_resp_chunk, CHUNK_ID_CARD_SAVE_COUNT, data);
        } break;
        }
    }
    return (uint32_t)next_resp_chunk - (uint32_t)pack_resp.data;
}

static uint32_t cmd_read_data()
{
    int32_t req_data_size = pack_req.header.data_size;
    void *next_resp_chunk = pack_resp.data;
    void *next_req_chunk = pack_req.data;

    while (req_data_size) {
        struct chunk_head *ch = (struct chunk_head *)next_req_chunk;
        uint32_t chunk_size = ch->data_size + sizeof(struct chunk_head);
        next_req_chunk = (uint8_t *)next_req_chunk + chunk_size;
        req_data_size -= chunk_size;

        switch (ch->id) {
        case CHUNK_ID_CARD_RANGE: {
            struct chunk_u16 *c = (struct chunk_u16 *)ch;
            struct keys_range keys = keys_get_cards(c->data & 0xFF, c->data >> 8);
            add_resp_card_uid_arr(&next_resp_chunk, CHUNK_ID_CARD_UID_ARR_READ, keys);
            uint32_t header_chunk_size = sizeof(struct header)
                                       + sizeof(struct chunk_card_uid_arr)
                                       + keys.size;

            if (keys.size == 0) {
                resp_list = (struct buf_list){
                    .count = 2,
                    .bufs = {
                             [0] = {.p = &pack_resp, .size = header_chunk_size},
                             [1] = {.p = &pack_resp.crc, .size = sizeof(crc16_t)},
                             },
                };
            } else {
                resp_list = (struct buf_list){
                    .count = 3,
                    .bufs = {
                             [0] = {.p = &pack_resp, .size = header_chunk_size},
                             [1] = {.p = keys.p, .size = keys.size},
                             [2] = {.p = &pack_resp.crc, .size = sizeof(crc16_t)},
                             },
                };
            }
            return sizeof(struct chunk_card_uid_arr) + keys.size;
        } break;
        default:
            return 0;
        }
    }

    return 0;
}

void aura_cmd_process(void)
{
    if (flags_pack_received == 0) {
        return;
    }
    flags_pack_received = 0;

    uint32_t resp_data_size = 0;
    enum func f = pack_req.header.func;
    pack_resp.header.func = f + 1;
    switch (f) {
    case FUNC_REQ_WHOAMI: {
        resp_data_size = cmd_whoami();
    } break;
    case FUNC_REQ_STATUS: {
        resp_data_size = cmd_status();
    } break;
    case FUNC_REQ_WRITE_DATA: {
        resp_data_size = cmd_write_data();
    } break;
    case FUNC_REQ_READ_DATA: {
        resp_data_size = cmd_read_data();
    } break;
    default: {
        aura_recv_package();
        return;
    }
    }

    aura_send_response(resp_data_size);
}

void aura_init(void)
{
    uid = uid_hash();
    pack_resp.header.uid_src = uid;
    aura_recv_package();
}

void uart_recv_dma_callback(void)
{
    switch (state_recv) {
    case STATE_RECV_START: {
        state_recv = STATE_RECV_HEADER;
        uart_recv_dma(pack_req.data, pack_req.header.data_size + sizeof(crc16_t));
    } break;
    case STATE_RECV_HEADER: {
        state_recv = STATE_RECV_CRC;
        uart_stop_recv();
        uint32_t pack_size =
            sizeof(struct header) + pack_req.header.data_size + sizeof(crc16_t);
        if (crc16_is_valid(&pack_req, pack_size)) {
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
    if (buf_list_is_nonempty(&resp_list)) {
        struct buf *b = buf_list_get_next(&resp_list);
        uart_send_dma(b->p, b->size);
    } else {
        aura_recv_package();
    }
}
