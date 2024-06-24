#ifndef __CHUNK_H
#define __CHUNK_H

#include "stdint.h"
#include "rfid.h"
#include "access.h"
#include "tools.h"

enum chunk_err {
    CHUNK_ERR_NONE = 0,
    CHUNK_ERR_DOUBLE_CARD,
    CHUNK_ERR_OVERHEAD_COUNT_CARD,
    CHUNK_ERR_NON_VALID_RANGE,
    CHUNK_ERR_CMD_ARG,
    CHUNK_ERR_ACC_LAST_UID,
};

enum chunk_id {
    CHUNK_ID_TYPE_SENSOR = 1,
    CHUNK_ID_UIDS_ARRAY = 2,

    CHUNK_ID_ERR = 3,

    CHUNK_ID_STATUS_LOCKER = 4,

    CHUNK_ID_NEW_ACCESS_COUNT = 5,

    CHUNK_ID_CARDS_TO_WRITE = 6,
    CHUNK_ID_CARDS_TO_READ = 7,
    CHUNK_ID_READ_CARDS = 8,

    CHUNK_ID_SAVED_CARD_COUNT = 9,
    CHUNK_ID_CLEAR_SAVED_CARD = 10,

    CHUNK_ID_ACCESS_TIME = 11,
    CHUNK_ID_ACCESS = 12,
    CHUNK_ID_GET_ACCESS = 13,
    CHUNK_ID_LAST_UID_ACCESS = 14,
    CHUNK_ID_CUR_UID_ACCESS = 15,
    // CHUNK_ID_SAVED_CARDS = 5,
    // CHUNK_ID_SAVED_CARD_COUNT = 6,

    // CHUNK_ID_ACCESS_CARD = 9,
    // CHUNK_ID_ACCESS_VALID = 10,
};

enum chunk_data_type {
    CHUNK_TYPE_NONE = 0,
    CHUNK_TYPE_I8 = 1,
    CHUNK_TYPE_U8 = 2,
    CHUNK_TYPE_I16 = 3,
    CHUNK_TYPE_U16 = 4,
    CHUNK_TYPE_I32 = 5,
    CHUNK_TYPE_U32 = 6,
    CHUNK_TYPE_F32 = 7,
    CHUNK_TYPE_F64 = 8,
    CHUNK_TYPE_STR = 9,
    CHUNK_TYPE_I8_ARR = 10,
    CHUNK_TYPE_U8_ARR = 11,
    CHUNK_TYPE_I16_ARR = 12,
    CHUNK_TYPE_U16_ARR = 13,
    CHUNK_TYPE_I32_ARR = 14,
    CHUNK_TYPE_U32_ARR = 15,
    CHUNK_TYPE_F32_ARR = 16,
    CHUNK_TYPE_F64_ARR = 17,

    CHUNK_TYPE_CARD_UID = 19,
    CHUNK_TYPE_CARD_UID_ARR = 20,
    CHUNK_TYPE_CARD_RANGE = 21,
    CHUNK_TYPE_ACCESS = 22,
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

inline static void *add_chunk_head(void **chunk,
                                   enum chunk_id id,
                                   enum chunk_data_type type,
                                   uint16_t size)
{
    struct chunk *head = (struct chunk *)*chunk;
    *chunk = (void *)((uint32_t)*chunk + size + sizeof(*head));
    head->id = id;
    head->type = type;
    head->data_size = size;
    return head->data;
}

inline static void add_chunk(void **chunk,
                             enum chunk_id id,
                             enum chunk_data_type type,
                             uint16_t size,
                             void *data)
{
    void *chunk_data = add_chunk_head(chunk, id, type, size);
    memcpy_u16(data, chunk_data, size);
}

inline static void add_chunk_u16(void **chunk, enum chunk_id id, uint16_t val)
{
    void *data = add_chunk_head(chunk, id, CHUNK_TYPE_U16, sizeof(val));
    *(uint16_t *)data = val;
}

inline static void add_chunk_u32(void **chunk, enum chunk_id id, uint32_t val)
{
    add_chunk(chunk, id, CHUNK_TYPE_U32, sizeof(val), &val);
}

inline static void add_chunk_err(void **chunk, enum chunk_err err)
{
    add_chunk(chunk, CHUNK_ID_ERR, CHUNK_TYPE_U16, 2, &err);
}

inline static void add_chunk_acc(void **chunk, struct access *acc)
{
    uint16_t is_valid = (acc->card_uid.raw[0] & 0x80) ? 0x00FF : 0x0000;
    struct access_pack pack = {
        .is_valid = is_valid,
    };
    pack.acc = *acc;
    pack.acc.card_uid.raw[0] &= ~0x80;
    add_chunk(chunk, CHUNK_ID_ACCESS, CHUNK_TYPE_ACCESS, sizeof(pack), &pack);
}

#endif
