#include "keys.h"
#include "flash.h"
#include "rfid.h"
#include "tools.h"
#include "utils.h"

#define KEYS_PAGE_NUM      31
#define KEYS_PAGE_SIZE     FLASH_PAGE_SIZE
#define KEYS_PAGE_ADDR     (flash_get_page_addr(KEYS_PAGE_NUM))
#define KEYS_PAGE_ADDR_END (KEYS_PAGE_ADDR + KEYS_PAGE_SIZE - 1)

#define KEYS_MAX_COUNT     (KEYS_PAGE_SIZE / sizeof(union rfid_card_uid))

#if defined __ARMCC_VERSION
#define __place2flash __attribute__((section(".ARM.__at_0x08007C00")))
#elif defined __GNUC__
#define __place2flash __attribute__((section(".keys")))
#endif

// clang-format off
// const union rfid_card_uid keys[] __place2flash = {
//     {.len = 4, .val = {0x75, 0xDA, 0x4C, 0x9A, }},
//     {.len = 7, .val = {0x75, 0xDA, 0x4C, 0x9A, 0x75, 0xDA, 0x4C,}},
//     {.len = 4, .val = {0x75, 0xD0, 0x4C, 0x9A, }},
// };

// clang-format on

struct keys_head {
    uint32_t cnt;
    union rfid_card_uid *new_key;
} keys_head;

static uint32_t addr_is_invalid(union rfid_card_uid *uid)
{
    uint32_t addr = (uint32_t)uid;
    return (addr + sizeof(*uid) - 1) > KEYS_PAGE_ADDR_END;
}

static union rfid_card_uid *key_get(uint32_t idx)
{
    return (union rfid_card_uid *)KEYS_PAGE_ADDR + idx;
}

void keys_init(void)
{
    union rfid_card_uid *uid = key_get(0);
    while (1) {
        if (addr_is_invalid(uid)) {
            break;
        }
        if ((uid->raw[0] == 0xFFFFFFFF) || (uid->raw[0] == 0x00000000)) {
            break;
        }
        uid++;
        keys_head.cnt++;
    }
    keys_head.new_key = uid;
}

uint32_t key_is_valid(union rfid_card_uid *uid)
{
    for (uint32_t i = 0; i < keys_head.cnt; i++) {
        const union rfid_card_uid *uid_ref = key_get(i);
        for (uint32_t j = 0; j < arr_len(uid->raw); j++) {
            if (uid_ref->raw[j] != uid->raw[j]) {
                goto int_for_end;
            }
        }
        return 1;
int_for_end:
        __NOP();
        // if ((uid->raw[0] == uid_ref->raw[0])
        //     && (uid->raw[1] == uid_ref->raw[1])) {
        //     return 1;
        // }
    }
    return 0;
}

uint32_t key_save(union rfid_card_uid *uid)
{
    uint32_t err = (key_is_valid(uid) << 8)
                 | (addr_is_invalid(keys_head.new_key) << 9);

    if (err == 0) {
        flash_memcpy_u16(uid, keys_head.new_key, sizeof(*uid));
        keys_head.new_key++;
        keys_head.cnt++;
    }
    return err | keys_head.cnt;
}

uint32_t keys_get_count(void)
{
    return keys_head.cnt;
}

uint32_t keys_clear(void)
{
    flash_erase_page(KEYS_PAGE_ADDR);
    keys_head.new_key = key_get(0);
    keys_head.cnt = 0;
    return KEYS_MAX_COUNT;
}

struct keys_range keys_get_cards(uint32_t offset, uint32_t count)
{
    if (offset > keys_head.cnt) {
        return (struct keys_range){0};
    }
    union rfid_card_uid *p = key_get(offset);
    count = MIN(count, keys_head.cnt - offset);

    return (struct keys_range){
        .p = (uint8_t *)p,
        .size = count * sizeof(union rfid_card_uid)};
}
