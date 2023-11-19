#include "stm32f0xx.h"

#ifndef FLASH_KEY1
#define FLASH_KEY1 0x45670123U
#endif
#ifndef FLASH_KEY2
#define FLASH_KEY2 0xCDEF89ABU
#endif

static void flash_unlock(void)
{
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

static void flash_lock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}

static void flash_clear_err(void)
{
    FLASH->SR |= FLASH_SR_PGERR | FLASH_SR_WRPRTERR | FLASH_SR_WRPERR;
}

static void flash_program_u16(uint32_t adr, uint16_t data)
{
    while (FLASH->SR & FLASH_SR_BSY) {
        ;
    }

    *(__IO uint16_t *)adr = data;

    while ((FLASH->SR & FLASH_SR_EOP) != FLASH_SR_EOP) {
        ;
    }
    FLASH->SR |= FLASH_SR_EOP;
}

void flash_erase_page(uint32_t page_addr)
{
    flash_unlock();
    flash_clear_err();

    while (FLASH->SR & FLASH_SR_BSY) {
        ;
    }

    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = page_addr;
    FLASH->CR |= FLASH_CR_STRT;

    while ((FLASH->SR & FLASH_SR_EOP) != FLASH_SR_EOP) {
        ;
    }
    FLASH->SR |= FLASH_SR_EOP;

    FLASH->CR &= ~FLASH_CR_PER;
    flash_lock();
}

void flash_memcpy_u16(void *src, void *dst, uint32_t size)
{
    flash_unlock();
    flash_clear_err();

    FLASH->CR |= FLASH_CR_EOPIE | FLASH_CR_PG;

    uint16_t *s = src;
    uint16_t *d = dst;

    size >>= 1;
    do {
        flash_program_u16((uint32_t)d++, *s++);
    } while (--size);

    FLASH->CR &= ~(FLASH_CR_EOPIE | FLASH_CR_PG);
    flash_lock();
}
