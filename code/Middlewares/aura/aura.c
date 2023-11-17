#include "aura.h"
#include "uid_hash.h"
#include "assert.h"
#include "crc16.h"
#include "usart_ex.h"

struct header {
    const uint32_t protocol;
    uint32_t cnt;
    uint32_t uid_src;
    const uint32_t uid_dest;
};

static_assert(sizeof(struct header) == 16, "Error in struct header");

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
    uint8_t id;
    uint8_t type;
    uint16_t size;
    uint8_t data[];
};

static_assert(sizeof(struct chunk) == 4, "Error in struct chunk");

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
