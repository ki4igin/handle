#include "aura.h"
#include "uid_hash.h"
#include "assert.h"
#include "crc16.h"
#include "usart_ex.h"

struct header {
    const uint32_t protocol;
    uint16_t cnt;
    const uint8_t dist;

    union flags {
        struct {
            uint8_t req_ask :1;
            uint8_t data    :1;
            uint8_t whoami  :1;
        };

        uint8_t val;
    } flags;

    uint32_t uid_src;
    const uint32_t uid_dest;
    const uint32_t path[4];
};

static_assert(sizeof(struct header) == 32, "Error in struct header");

enum chunk_type {
    CHUNK_TYPE_CHAR = 1,
    CHUNK_TYPE_UCHAR = 2,
    CHUNK_TYPE_SHORT = 3,
    CHUNK_TYPE_USHORT = 4,
    CHUNK_TYPE_INT = 5,
    CHUNK_TYPE_UINT = 6,
    CHUNK_TYPE_FLOAT = 7,
    CHUNK_TYPE_DOUBLE = 8,
    CHUNK_TYPE_STRING = 9,
};

enum chunk_id {
    CHUNK_ID_WHOIM = 128,
};

struct chunk {
    uint16_t id;
    uint16_t type;
    uint16_t size;
    uint8_t data[];
};

static_assert(sizeof(struct chunk) == 6, "Error in struct chunk");

// clang-format off
struct package_whoami {
    struct header header;
    uint16_t crc;
} package_whoami = {
    .header = {
        .protocol = 0x41525541U,
        .cnt = 0,
        .dist = 0,
        .flags.whoami = 1,
        .uid_src = 0,
        .uid_dest = 0,
        .path = {0},
    },
    .crc = 0,
};

// clang-format on

uint32_t uid = 0;

void aura_init(void)
{
    uid = uid_hash();
    package_whoami.header.uid_src = uid;
}

void aura_send_whoami(void)
{
    struct package_whoami *p = &package_whoami;
    struct header *h = &p->header;
    h->cnt++;
    uint16_t crc = crc16(0, h, sizeof(*h));
    p->crc = crc;
    uart_send_array_dma(p, sizeof(*p));
}
