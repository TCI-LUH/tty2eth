#include "storage.h"

#include "stm32h7xx_hal.h"

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define STORAGE_TEST 0

extern char _sstorage;
extern char _estorage;
extern char _sistorage;
__attribute__((section(".storage"))) int magic = 153;


void initStorage()
{
    memcpy(&_sstorage, &_sistorage, &_estorage - &_sstorage);
}

void storageTest()
{
    #if STORAGE_TEST 
    while (1)
    {
        printf("magic @%p %d@%p\n", &_sstorage, magic++, &magic);
        osDelay(300);

        if(magic % 100 == 0)
            saveStorage();
    }
    #endif
}

void saveStorage()
{
    printf("save storage\n");
    size_t size = &_estorage - &_sstorage;
    if(size == 0)
    {
        printf("skip saving\n");
        return;
    }

    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector        = FLASH_SECTOR_7;
    EraseInitStruct.Banks         = FLASH_BANK_1;
    EraseInitStruct.NbSectors     = 1;
    uint32_t err = 0;
    HAL_FLASHEx_Erase(&EraseInitStruct, &err);

    for(size_t offset = 0; offset < size; offset += 32)
    {
        // printf("flash: %x <- %x of total: %db\n",  &_sistorage + offset, &_sstorage + offset, size);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, (uint32_t)&_sistorage + offset, (uint32_t)(&_sstorage + offset));
    }
    HAL_FLASH_Lock();
}
