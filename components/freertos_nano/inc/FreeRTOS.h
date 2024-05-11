#ifndef FREERTOS_H
#define FREERTOS_H

#include "FreeRTOSConfig.h"
#include "list.h"
#include "portable.h"
#include "projdefs.h"

// clang-format off
#ifndef configNUMBER_OF_CORES
    #define configNUMBER_OF_CORES    1
#endif

// clang-format on
typedef struct tskTaskControlBlock {
    volatile StackType_t* pxTopOfStack;

    ListItem_t xStateListItem;

    StackType_t* pxStack;
    char pcTaskName[configMAX_TASK_NAME_LEN];
    TickType_t xTicksToDelay;
} tskTCB;
typedef tskTCB TCB_t;

#endif /* FREERTOS_H */
