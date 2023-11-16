#include "uid_hash.h"
#include "stm32f0xx_ll_utils.h"

// Magic numbers for 32-bit hashing.  Copied from Murmur3.
const uint32_t c1 = 0xcc9e2d51;
const uint32_t c2 = 0x1b873593;

static uint32_t Fetch32(const char *p)
{
    union {
        uint32_t word;
        uint8_t bytes[4];
    } res;

    for (uint32_t i = 0; i < 4; i++) {
        res.bytes[i] = *p++;
    }
    return res.word;
}

static uint32_t Rotate32(uint32_t val, int32_t shift)
{
    // Avoid shifting by 32: doing so yields an undefined result.
    return shift == 0 ? val : ((val >> shift) | (val << (32 - shift)));
}

// A 32-bit to 32-bit integer hash copied from Murmur3.
static uint32_t fmix(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

static uint32_t Mur(uint32_t a, uint32_t h)
{
    // Helper from Murmur3 for combining two 32-bit values.
    a *= c1;
    a = Rotate32(a, 17);
    a *= c2;
    h ^= a;
    h = Rotate32(h, 19);
    return h * 5 + 0xe6546b64;
}

static uint32_t Hash32Len5to12(const char *s, uint32_t len)
{
    uint32_t a = (uint32_t)len, b = a * 5, c = 9, d = b;
    a += Fetch32(s);                    // add first 4 bytes
    b += Fetch32(s + len - 4);          // last ones
    c += Fetch32(s + ((len >> 1) & 4)); // and fourth if len > 7
    return fmix(Mur(c, Mur(b, Mur(a, d))));
}

uint32_t uid_hash(void)
{
    union uidun {
        uint32_t num[3];
        char str[12];
    } uid;

    uid.num[0] = LL_GetUID_Word2();
    uid.num[1] = LL_GetUID_Word1();
    uid.num[2] = LL_GetUID_Word0();

    return Hash32Len5to12((const char *)uid.str, 12);
}
