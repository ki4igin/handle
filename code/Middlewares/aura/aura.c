#include "aura.h"
#include "chunk.h"
#include "assert.h"
#include "crc16.h"
#include "keys.h"
#include "locker.h"
#include "rfid.h"
#include "uid_hash.h"
#include "usart_ex.h"
#include "tools.h"
#include "buf.h"
#include "tim_ex.h"
#include "access.h"

#define RESPONSE_DELAY_ON  1

#define AURA_HANDLE_ID     7
#define AURA_MAX_DATA_SIZE 256

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

uint32_t flags_pack_received = 0;
uint32_t uid = 0;

static struct {
    struct header header;
    uint8_t data[AURA_MAX_DATA_SIZE];
    crc16_t crc;
    void *next_chunk;
} pack_req, pack_resp = {.header.protocol = 0x41525541U};

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

static void aura_send_response(uint32_t data_size)
{
    static uint32_t cnt = 0;
    pack_resp.header.cnt = cnt++;
    pack_resp.header.uid_dest = pack_req.header.uid_src;
    pack_resp.header.data_size = data_size;

    if (resp_list.count) {
        // Это тупой костыль, но по другому я не придумал прокинуть размер
        // ключей при чтении из flash
        pack_resp.header.data_size = sizeof(struct chunk_card_uid_arr)
                                   + resp_list.bufs[1].size;
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
    struct access *acc = access_circ_get_last(access_circ);
    add_chunk_acc(&next_chunk, acc);
    return (uint32_t)next_chunk - (uint32_t)pack_resp.data;
}

static void parse_write_chunk(const struct chunk_head *ch,
                              void **next_resp_chunk)
{
    switch (ch->id) {
    case CHUNK_ID_CARD_UID_ARR_WRITE: {
        struct chunk_card_uid_arr *c = (struct chunk_card_uid_arr *)ch;
        uint32_t count = c->head.data_size / sizeof(union rfid_card_uid);
        struct keys_res res = keys_save(c->data, count);
        add_chunk_u16(next_resp_chunk, CHUNK_ID_ERR, res.err);
        add_chunk_u16(next_resp_chunk, CHUNK_ID_CARD_SAVE_COUNT, res.val);
    } break;
    case CHUNK_ID_STATUS_LOCKER: {
        struct chunk_u16 *c = (struct chunk_u16 *)ch;
        if (c->data == 0x00FF) {
            locker_open();
        } else if (c->data == 0x0000) {
            locker_close();
        }
        uint16_t data = locker_is_open() ? 0x00FF : 0x0000;
        add_chunk_u16(next_resp_chunk, CHUNK_ID_STATUS_LOCKER, data);
    } break;
    case CHUNK_ID_CARD_CLEAR: {
        struct chunk_u16 *c = (struct chunk_u16 *)ch;
        if (c->data == 0x00FF) {
            keys_clear();
        }
        uint16_t data = keys_get_count();
        add_chunk_u16(next_resp_chunk, CHUNK_ID_CARD_SAVE_COUNT, data);
    } break;
    case CHUNK_ID_ACCESS_TIME: {
        struct chunk_u32 *c = (struct chunk_u32 *)ch;
        uint32_t new_time = c->data;
        tim1s_set(new_time);
        add_chunk_u32(next_resp_chunk, CHUNK_ID_ACCESS_TIME, new_time);
    } break;
    default:
    }
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
        parse_write_chunk(ch, &next_resp_chunk);
    }
    return (uint32_t)next_resp_chunk - (uint32_t)pack_resp.data;
}

static void parse_read_chunk(const struct chunk_head *ch,
                             void **next_resp_chunk)
{
    switch (ch->id) {
    case CHUNK_ID_CARD_RANGE: {
        struct chunk_u16 *c = (struct chunk_u16 *)ch;
        struct keys_range keys = keys_get_cards(c->data & 0xFF, c->data >> 8);
        add_chunk_head(next_resp_chunk,
                       CHUNK_ID_CARD_UID_ARR_READ,
                       CHUNK_TYPE_CARD_UID_ARR,
                       keys.size);
        uint32_t header_chunk_size = sizeof(struct header)
                                   + sizeof(struct chunk_card_uid_arr);

        // clang-format off
        if (keys.size) {
            resp_list = (struct buf_list){
                .count = 3,
                .bufs = {
                    [0] = {.p = &pack_resp, .size = header_chunk_size},
                    [1] = {.p = keys.p, .size = keys.size},
                    [2] = {.p = &pack_resp.crc, .size = sizeof(crc16_t)},
                },
            };
        }
        // clang-format on
    } break;
    case CHUNK_ID_ACCESS_COUNT: {
        struct chunk_u16 *c = (struct chunk_u16 *)ch;
        uint32_t offset = c->data & 0xFF;
        uint32_t count = (c->data >> 8);
        count = (count < 8) ? count : 8;
        for (uint32_t i = 0; i < count; i++) {
            uint32_t idx = i + offset;
            struct access *acc = access_circ_get_from_end(access_circ, idx);
            add_chunk_acc(next_resp_chunk, acc);
        }
    } break;
    default:
    }
}

static uint32_t cmd_read_data()
{
    int32_t req_data_size = pack_req.header.data_size;
    void *next_resp_chunk = pack_resp.data;
    void *next_req_chunk = pack_req.data;

    while (req_data_size > 0) {
        const struct chunk_head *ch = (struct chunk_head *)next_req_chunk;
        uint32_t chunk_size = ch->data_size + sizeof(struct chunk_head);
        next_req_chunk = (void *)((uint32_t)next_req_chunk + chunk_size);
        req_data_size -= chunk_size;
        parse_read_chunk(ch, &next_resp_chunk);
    }

    return (uint32_t)next_resp_chunk - (uint32_t)pack_resp.data;
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
#if RESPONSE_DELAY_ON
    LL_mDelay(2);
#endif

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
