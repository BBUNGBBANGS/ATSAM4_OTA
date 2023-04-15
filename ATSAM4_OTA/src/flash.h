#ifndef _FLASH_H
#define _FLASH_H

#include "main.h"

/* Start and end addresses of the user application. */
#define FLASH_APP1_START_ADDRESS ((uint32_t)0x00406000u)
#define FLASH_APP2_START_ADDRESS ((uint32_t)0x00413000u)
#define FLASH_REPROGRAM_STATUS_ADDRESS ((uint32_t)0x00405000u)
#define FLASH_PAGE_SIZE     (512u)
#define FLASH_COPY_SIZE     (0xD000u)
typedef void (application_t)(void);
typedef struct
{
    uint32_t stack_addr;     // Stack Pointer
    application_t *func_p;        // Program Counter
} Jump_Application_t;
/* Status report for the functions. */
typedef enum 
{
    FLASH_OK = 0x00u, /**< The action was successful. */
    FLASH_ERROR = 0xFFu  /**< Generic error. */
} Flash_Status_t;

extern Flash_Status_t Flash_Init(void);
extern Flash_Status_t Flash_Erase(uint32_t address);
extern Flash_Status_t Flash_Write(uint32_t address, uint32_t *data, uint32_t size);
extern void Flash_Jump_To_Application(void);
extern void Flash_Copy_App2_To_App1(void);
#endif /* FLASH_H_ */
