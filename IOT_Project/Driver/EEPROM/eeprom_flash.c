/**
 *******************************************************************************
 * @file    eeprom.c
 * @author  zhao 
 * @version V1.0
 * @date    2018-6-20
 * @since   2018-6-20
 * @ref     STM32F10x_AN2594_FW_V3.1.0 - MCD Application Team
 * @brief   This file provides all the EEPROM emulation firmware functions.
 *
 * Page map
 * +---------+-------------+----------------------------+
 * | byte    | length      | description                |
 * +---------+-------------+----------------------------+ <- PAGE_BASE_ADDRESS
 * | [0: 1]  | 16 bits     | page status:               |
 * |         |             |   0xFFFF: ERASED           |
 * |         |             |   0xEEEE: RECEIVE_DATA     |
 * |         |             |   0x0000: VALID_PAGE       |
 * +---------+-------------+----------------------------+
 * | [2: 7]  | 48 bits     | reserved                   |
 * +---------+-------------+----------------------------+
 * | [8: 15] | 64 bits     | block 1                    |
 * +---------+-------------+----------------------------+
 * |         |             | ...                        |  ↓block grows
 * +---------+-------------+----------------------------+
 * |         | 64 bits     | block n                    | ------+
 * +---------+-------------+----------------------------+       |
 * |                                                    |       | data offset
 * +---------+-------------+----------------------------+       | (16 bits)
 * |         | 16 bits     | 0xCCCC                     |       |
 * +---------+-------------+----------------------------+ <-----+
 * |         | data length | data n (2 bytes alignment) |
 * +---------+-------------+----------------------------+
 * |         |             | ...                        |  ↑data grows
 * +---------+-------------+----------------------------+
 * |         | 16 bits     | 0xCCCC                     |
 * +---------+-------------+----------------------------+
 * |         | data length | data 1 (2 bytes alignment) |
 * +---------+-------------+----------------------------+  <- PAGE_END_ADDRESS
 *
 * Block map (64 bits)
 * +----------+---------+-----------------------------------------+
 * | bits     | length  | description                             |
 * +----------+---------+-----------------------------------------+
 * | [0: 15]  | 16 bits | block status:                           |
 * |          |         |   0xAAAA: VALID_BLOCK                   |
 * |          |         |   0x0000: EXPIRED_BLOCK                 |
 * +----------+---------+-----------------------------------------+
 * | [16: 23] | 8 bits  | data key                                |
 * +----------+---------+-----------------------------------------+
 * | [24: 31] | 8 bits  | data checksum                           |
 * +----------+---------+-----------------------------------------+
 * | [32: 47] | 16 bits | data offset (base on PAGE_BASE_ADDRESS) |
 * +----------+---------+-----------------------------------------+
 * | [48: 63] | 16 bits | data length (bytes)                     |
 * +----------+---------+-----------------------------------------+
 *
 *******************************************************************************
 */

#include "eeprom_flash.h"


/**
 * Private functions
 */
static uint16_t EE_Format(void);
static uint16_t EE_FindValidPage(uint8_t operation);
static uint16_t EE_WriteByVerify(uint8_t key, const uint8_t *pdata, uint16_t size);
static uint16_t EE_PageTransfer(uint16_t page);
static uint16_t EE_CheckData(void);


/**
 * Function EE_Init
 * Restore the pages to a known good state in case of page's status corruption
 * after a power loss.
 * Page status combinations and actions to be taken as follows:
 * +--------------+-----------------------------------------------------------------------------------+
 * |              |                                     PAGE_0                                        |
 * |    PAGE_1    +-------------------------+----------------------------+----------------------------+
 * |              | ERASED                  | RECEIVE_DATA               | VALID_PAGE                 |
 * +--------------+-------------------------+----------------------------+----------------------------+
 * |              | First EEPROM access or  | Erase PAGE_1 and mark      | OK                         |
 * | ERASED       | invalid state.          | PAGE_0 as valid.           | Run check data routine.    |
 * |              | Erase both pages and    | Run check data routine.    |                            |
 * |              | mark PAGE_0 as valid.   |                            |                            |
 * |--------------+-------------------------+----------------------------+----------------------------+
 * |              | Erase PAGE_0 and mark   | Invalid state.             | Transfer the last updated  |
 * |              | PAGE_1 as valid.        | Erase both pages and       | data from PAGE_0 to PAGE_1 |
 * | RECEIVE_DATA | Run check data routine. | format PAGE_0.             | and mark PAGE_1 as valid   |
 * |              |                         |                            | and erase PAGE_0.          |
 * |              |                         |                            | Run check data routine.    |
 * +--------------+-------------------------+----------------------------+----------------------------+
 * |              | OK                      | Transfer the last updated  | Invalid state.             |
 * |              | Run check data routine. | data from PAGE_1 to PAGE_0 | Erase both pages and mark  |
 * | VALID_PAGE   |                         | and mark PAGE_0 as valid   | PAGE_0 as valid.           |
 * |              |                         | and erase PAGE_1.          |                            |
 * |              |                         | Run check data routine.    |                            |
 * +--------------+-------------------------+----------------------------+----------------------------+
 *
 * @param  none
 * @return success or error status
 *         - FLASH_COMPLETE: on success
 *         - error code:     on error
 */
uint16_t EE_Init(void) {
  uint16_t status = FLASH_COMPLETE;
  uint16_t ps0 = 0; // PAGE_0 status
  uint16_t ps1 = 0; // PAGE_1 status

  ps0 = (*(__IO uint16_t*)PAGE_0_BASE_ADDRESS);
  ps1 = (*(__IO uint16_t*)PAGE_1_BASE_ADDRESS);

//  printf("PAGE_0 status: %x, PAGE_1 status: %x\n", ps0, ps1);

  switch(ps0) {
    /* --------------------------- PAGE_0 erased ---------------------------- */
    case ERASED:
      if(ps1 == VALID_PAGE) {                                    // PAGE_1 valid
        return EE_CheckData();
      }
      else if(ps1 == RECEIVE_DATA) {                           // PAGE_1 receive
        /* erase PAGE_0 */
        status = FLASH_ErasePage(PAGE_0_BASE_ADDRESS);
        if(status != FLASH_COMPLETE) return status;

        /* mark PAGE_1 as valid */
        status = FLASH_ProgramHalfWord(PAGE_1_BASE_ADDRESS, VALID_PAGE);
        if(status != FLASH_COMPLETE) return status;
      }
      else {                                                    // invalid state
        /* erase both pages and mark PAGE_0 as valid */
        return EE_Format();
      }
      break;

    /* --------------------------- PAGE_0 receive --------------------------- */
    case RECEIVE_DATA:
      if(ps1 == VALID_PAGE) {                                    // PAGE_1 valid
        /* transfer the last updated data from PAGE_1 to PAGE_0
           and mark PAGE_0 as valid and erase PAGE_1
         */
        status = EE_PageTransfer(PAGE_1);
        if(status != FLASH_COMPLETE) return status;
      }
      else if(ps1 == ERASED) {                                  // PAGE_1 erased
        /* erase PAGE_1 */
        status = FLASH_ErasePage(PAGE_1_BASE_ADDRESS);
        if(status != FLASH_COMPLETE) return status;

        /* mark PAGE_0 as valid */
        status = FLASH_ProgramHalfWord(PAGE_0_BASE_ADDRESS, VALID_PAGE);
        if(status != FLASH_COMPLETE) return status;
      }
      else {                                                    // invalid state
        /* erase both pages and mark PAGE_0 as valid */
        return EE_Format();
      }
      break;

    /* --------------------------- PAGE_0 valid ----------------------------- */
    case VALID_PAGE:
      if(ps1 == VALID_PAGE) {                                   // invalid state
        /* erase both pages and mark PAGE_0 as valid */
        return EE_Format();
      }
      else if(ps1 == ERASED) {                                  // PAGE_1 erased
        return EE_CheckData();
      }
      else {                                                   // PAGE_1 receive
        /* transfer the last updated data from PAGE_0 to PAGE_1
           and mark PAGE_1 as valid and erase PAGE_0
         */
        status = EE_PageTransfer(PAGE_0);
        if(status != FLASH_COMPLETE) return status;
      }
      break;

    /* ------------------------------ default ------------------------------- */
    default:
      /* erase both pages and set PAGE_0 as valid */
      return EE_Format();
  }

  return EE_CheckData();
}

/**
 * Function EE_ReadData
 * Get the last stored data correspond to the passed data key.
 * @param  key:   8 bits key of the stored data
 * @param  pdata: pointer to the variable that will contain the read value
 * @param  size:  data size in bytes
 * @return success or error status
 *         - 0: if data found
 *         - 1: if data not found
 *         - EE_NO_VALID_PAGE:       if no valid page found
 *         - EE_DATA_SIZE_ERROR:     if the passed size not equal to data length
 *         - EE_DATA_CHECKSUM_ERROR: if data checksum error
 */
uint16_t EE_ReadData(uint8_t key, uint8_t* pdata, uint16_t size) {
  uint16_t page = PAGE_0;
  uint32_t base = EEPROM_START_ADDRESS;
  uint32_t address = EEPROM_START_ADDRESS;
  uint16_t value = 0;
  uint16_t dataOffset = 0;
  uint16_t dataLength = 0;
  uint8_t dataChecksum = 0;
  uint8_t checksum = 0;
  uint16_t flag = 1; // data not found

  /* get valid page for read operation */
  page = EE_FindValidPage(READ_FROM_VALID_PAGE);
  if(page == EE_NO_VALID_PAGE) return page;

  /* page base address */
  base = EEPROM_START_ADDRESS + (uint32_t)(page * PAGE_SIZE);

  /* block address */
  address = base + 8;

  /* check each valid block starting from begining */
  while(address < base + PAGE_SIZE) {
    value = (*(__IO uint16_t*)address); // block status

    /* in case the valid block found */
    if(value == VALID_BLOCK) {
      value = (*(__IO uint16_t*)(address + 2)); // key & checksum

      /* in case the key matched (valid data found) */
      if((value & 0x00FF) == key) {
        dataChecksum = (uint8_t)(value >> 8);
        dataOffset = (*(__IO uint16_t*)(address + 4));
        dataLength = (*(__IO uint16_t*)(address + 6));

        /* check data size */
        if(dataLength != size || (base + dataOffset + size) > (base + PAGE_SIZE)) {
          return EE_DATA_SIZE_ERROR;
        }

//        printf("read data from: %x\n", base + dataOffset);

        /* read data */
        for(; size > 0; size--) {
          value = (*(__IO uint8_t*)(base + dataOffset));
          checksum = (checksum << 1) || (checksum >> 7);
          checksum += value;
          *(pdata++) = value;
          dataOffset++;
        }

        /* verify the checksum */
        if(checksum != dataChecksum) {
          return EE_DATA_CHECKSUM_ERROR;
        }

        flag = 0;
        break;
      }
    }
    else if(value != EXPIRED_BLOCK) { // end of block area
      break;
    }
    address += 8; // next block
  }

  return flag;
}

/**
 * Function EE_WriteData
 * Writes/upadtes data in EEPROM.
 * @param  key:   8 bits key of the data
 * @param  pdata: pointer to the data that will be written
 * @param  size:  data size in bytes
 * @return success or error status
 *         - FLASH_COMPLETE: on success
 *         - error code:     on error
 */
uint16_t EE_WriteData(uint8_t key, const uint8_t *pdata, uint16_t size) {
  uint16_t status = FLASH_COMPLETE;

  status = EE_WriteByVerify(key, pdata, size);

  /* in case the valid page is full, then perform page transfer */
  if(status == EE_PAGE_FULL) {
    status = EE_PageTransfer(EE_FindValidPage(WRITE_IN_VALID_PAGE));
    if(status != FLASH_COMPLETE) return status;

    /* write again */
    status = EE_WriteByVerify(key, pdata, size);
  }

  return status;
}

/**
 * Function EE_Format
 * Erase both pages and mark PAGE_0 as valid.
 * @param  none
 * @return status of the last operation
 */
static uint16_t EE_Format(void) {
  FLASH_Status status = FLASH_COMPLETE;

//  printf("format page.\n");

  /* erase PAGE_0 */
  status = FLASH_ErasePage(PAGE_0_BASE_ADDRESS);
  if(status != FLASH_COMPLETE) return status;

  /* mark PAGE_0 as valid */
  status = FLASH_ProgramHalfWord(PAGE_0_BASE_ADDRESS, VALID_PAGE);
  if(status != FLASH_COMPLETE) return status;

  /* erase PAGE_1 */
  status = FLASH_ErasePage(PAGE_1_BASE_ADDRESS);
  return status;
}

/**
 * Function EE_FindValidPage
 * Find valid page for write or read operation.
 * @param  operation: operation to achieve on the valid page
 *         @arg READ_FROM_VALID_PAGE: read operation from valid page
 *         @arg WRITE_IN_VALID_PAGE:  write operation to valid page
 * @return valid page number (PAGE_0 or PAGE_1) or EE_NO_VALID_PAGE
 */
static uint16_t EE_FindValidPage(uint8_t operation) {
  uint16_t ps0 = 0; // PAGE_0 status
  uint16_t ps1 = 0; // PAGE_1 status

  ps0 = (*(__IO uint16_t*)PAGE_0_BASE_ADDRESS);
  ps1 = (*(__IO uint16_t*)PAGE_1_BASE_ADDRESS);

  switch(operation) {
    /* write operation */
    case WRITE_IN_VALID_PAGE:
      if(ps1 == VALID_PAGE) {
        if(ps0 == RECEIVE_DATA) { // transfer PAGE_1 -> PAGE_0
          return PAGE_0;
        }
        else {
          return PAGE_1;
        }
      }
      else if(ps0 == VALID_PAGE) {
        if(ps1 == RECEIVE_DATA) { // transfer PAGE_0 -> PAGE_1
          return PAGE_1;
        }
        else {
          return PAGE_0;
        }
      }
      else {
        return EE_NO_VALID_PAGE;
      }

    /* read operation */
    case READ_FROM_VALID_PAGE:
      if(ps0 == VALID_PAGE) {
        return PAGE_0;
      }
      else if(ps1 == VALID_PAGE) {
        return PAGE_1;
      }
      else {
        return EE_NO_VALID_PAGE;
      }

    default:
      return PAGE_0;
  }
}

/**
 * Function EE_WriteByVerify
 * Verify if valid page is full and write data to EEPROM.
 * @param  key:   8 bits key of the data
 * @param  pdata: pointer to the data that will be written
 * @param  size:  data size in bytes
 * @return success or error status
 *         - FLASH_COMPLETE:     on success
 *         - EE_NO_VALID_PAGE:   if no valid page was found
 *         - EE_PAGE_FULL:       if valid page is full
 *         - EE_DATA_SIZE_ERROR: if passed data size is zero
 *         - error code:         on write flash error
 */
static uint16_t EE_WriteByVerify(uint8_t key, const uint8_t *pdata, uint16_t size) {
  uint16_t status = FLASH_COMPLETE;
  uint16_t page = PAGE_0;
  uint32_t base = EEPROM_START_ADDRESS;
  uint32_t address = EEPROM_START_ADDRESS;
  uint32_t dataAddress = EEPROM_START_ADDRESS;
  uint16_t value = 0;
  uint8_t checksum = 0;
  uint16_t i = 0;

  if(size == 0) return EE_DATA_SIZE_ERROR;

  /* get valid page for write operation */
  page = EE_FindValidPage(WRITE_IN_VALID_PAGE);
  if(page == EE_NO_VALID_PAGE) return page;

  /* page base address */
  base = EEPROM_START_ADDRESS + (uint32_t)(page * PAGE_SIZE);

  /* block address */
  address = base + 8;

  /* check each valid block starting from begining */
  while(address < base + PAGE_SIZE) {
    value = (*(__IO uint16_t*)address); // block head

    /* in case the valid block found */
    if(value == VALID_BLOCK) {
      value = (*(__IO uint16_t*)(address + 2)); // key & checksum

      /* in case the key matched, then set the block expired */
      if((value & 0x00FF) == key) {
        status = FLASH_ProgramHalfWord(address, EXPIRED_BLOCK);
        if(status != FLASH_COMPLETE) return status;
      }
    }
    else if(value != EXPIRED_BLOCK) { // end of block area
      break;
    }
    address += 8; // next block
  }

  /* get the last empty data address (2 bytes alignment) */
  if(address == (base + 8)) {
    dataAddress = base + PAGE_SIZE - 2;
  }
  else {
    value = (*(__IO uint16_t*)(address - 4)); // data offset
    if(value >= PAGE_SIZE) {
      return EE_PAGE_FULL; // trigger the page transfering action
    }
    dataAddress = base + value - 4;
  }

  /* check if has enough space for the new block and data */
  if((address + 8 + 2 + size) > dataAddress) { // accurate: dataAddress + 2
    return EE_PAGE_FULL;
  }

  /* check empty space for new block */
  for(i = 0; i < 4; i++) {
    value = (*(__IO uint16_t*)(address + i * 2));
    if(value != 0xFFFF) return EE_PAGE_FULL;
  }

  /* check empty space for new data (2 bytes alignment) */
  for(i = 0; i < size; i += 2) {
    value = (*(__IO uint16_t*)dataAddress);
    if(value != 0xFFFF) return EE_PAGE_FULL;
    dataAddress -= 2; // pointer to data flag
  }

  /* check empty space for data flag */
  value = (*(__IO uint16_t*)dataAddress);
  if(value != 0xFFFF) return EE_PAGE_FULL;

  /* write new block */
  status = FLASH_ProgramHalfWord(address, VALID_BLOCK); // block status
  if(status != FLASH_COMPLETE) return status;

  status = FLASH_ProgramHalfWord(address + 4, (dataAddress - base + 2)); // data offset (2 bytes after data flag)
  if(status != FLASH_COMPLETE) return status;

  status = FLASH_ProgramHalfWord(address + 6, size); // data length
  if(status != FLASH_COMPLETE) return status;

  /* write new data begin flag (0xCCCC)*/
  status = FLASH_ProgramHalfWord(dataAddress, DATA_FLAG);
  if(status != FLASH_COMPLETE) return status;
  dataAddress += 2;

//  printf("write data at: %x, space left: %d\n", dataAddress, (dataAddress - address - 8));

  /* write new data */
  for(i = 0; i < size; i++) {
    checksum = (checksum << 1) || (checksum >> 7);
    checksum += *(pdata + i);

    /* 2 bytes alignment */
    if((i & (uint16_t)0x1) == 0) {
      value = (i == (size - 1)) ? (uint16_t)(*(pdata + i)) : (*(uint16_t*)(pdata + i));
      status = FLASH_ProgramHalfWord(dataAddress, value);
      if(status != FLASH_COMPLETE) return status;
      dataAddress += 2;
    }
  }

  /* write key & checksum */
  value = ((uint16_t)checksum << 8 | key);
  status = FLASH_ProgramHalfWord(address + 2, value);
  return status;
}

/**
 * Function EE_PageTransfer
 * Transfer the last updated data from the passed page to an empty target page
 * and mark the target page as valid and erase passed page.
 * @param  page: page to be transfered
 * @return success or error status
 *         - FLASH_COMPLETE: on success
 *         - error code:     on error
 */
static uint16_t EE_PageTransfer(uint16_t page) {
  uint16_t status = FLASH_COMPLETE;
  uint32_t sourceBase = EEPROM_START_ADDRESS;
  uint32_t targetBase = EEPROM_START_ADDRESS;
  uint32_t address = EEPROM_START_ADDRESS;
  uint16_t value = 0;
  uint64_t block = 0;
  uint16_t *pcell = 0; // pointer to block cell in 2 bytes
  uint16_t dataOffset = 0;
  uint16_t dataLength = 0;
  uint32_t targetL = 0;
  uint32_t targetH = 0;
  uint16_t i = 0;

//  printf("transfer page%d\n", page);

  if(page == PAGE_0) {
    sourceBase = PAGE_0_BASE_ADDRESS;
    targetBase = PAGE_1_BASE_ADDRESS;
  }
  else if(page == PAGE_1) {
    sourceBase = PAGE_1_BASE_ADDRESS;
    targetBase = PAGE_0_BASE_ADDRESS;
  }
  else {
    return EE_NO_VALID_PAGE;
  }

  address = (uint32_t)(sourceBase + 8);         // source block address
  targetL = (uint32_t)(targetBase + 8);         // target block address
  targetH = (uint32_t)(targetBase + PAGE_SIZE); // target data address

  /* confirm target page erased */
  value = (*(__IO uint16_t*)targetBase);
  if(value != ERASED) {
    status = FLASH_ErasePage(targetBase);
    if(status != FLASH_COMPLETE) return status;
  }

  /* mark target page RECEIVE_DATA status */
  status = FLASH_ProgramHalfWord(targetBase, RECEIVE_DATA);
  if(status != FLASH_COMPLETE) return status;

  /* check each valid block in source page starting from begining */
  while(address < sourceBase + PAGE_SIZE) {
    value = (*(__IO uint16_t*)address); // block status

    /* in case of VALID_BLOCK */
    if(value == VALID_BLOCK) {
      block = (*(__IO uint64_t*)(address));
      pcell = (uint16_t*)(&block);
      dataOffset = (*(pcell + 2));
      dataLength = (*(pcell + 3));

      /* continue if data size error */
      if(dataLength == 0) {
        continue;
      }
      if((sourceBase + dataOffset + dataLength) > (sourceBase + PAGE_SIZE)) {
        continue;
      }

      /* calculate target data address */
      targetH = targetH - dataLength;
      if((targetH & (uint32_t)0x1) == 1) {
        targetH--; // 2 bytes alignment
      }

      if((targetH - targetL) < 10) continue; // block + data flag = 10 bytes

      /* update data offset cell in block */
      *(pcell + 2) = targetH - targetBase;

      /* write target block */
      for(i = 0; i < 4; i++) {
        status = FLASH_ProgramHalfWord(targetL + i * 2, *(pcell + i));
        if(status != FLASH_COMPLETE) return status;
      }
      targetL += 8;                                              // next targetL

      /* copy data */
      for(i = 0; i < dataLength; i += 2) {
        value = (*(__IO uint16_t*)(sourceBase + dataOffset + i));
        status = FLASH_ProgramHalfWord(targetH + i, value);
        if(status != FLASH_COMPLETE) return status;
      }

      /* write data begin flag (0xCCCC) */
      targetH = targetH - 2;                                     // next targetH
      status = FLASH_ProgramHalfWord(targetH, DATA_FLAG);
      if(status != FLASH_COMPLETE) return status;
    }
    else if(value != EXPIRED_BLOCK) { // end of block area
      break;
    }
    address += 8; // next source block
  }

  /* mark targer page as valid */
  status = FLASH_ProgramHalfWord(targetBase, VALID_PAGE);
  if(status != FLASH_COMPLETE) return status;

  /* erase source page */
  status = FLASH_ErasePage(sourceBase);
  return status;
}

/**
 * Function EE_CheckData
 * TODO: Verify data by calculating the checksum.
 */
static uint16_t EE_CheckData(void) {
  return FLASH_COMPLETE;
}

/**
 * Extensions
 */
uint16_t memcpy_to_eeprom_with_checksum(uint8_t key, const void *pdata, uint16_t size) {
  return EE_WriteData(key, (uint8_t*)pdata, size);
}

uint16_t memcpy_from_eeprom_with_checksum(void *pdata, uint8_t key, uint16_t size) {
  return EE_ReadData(key, (uint8_t*)pdata, size);
}

