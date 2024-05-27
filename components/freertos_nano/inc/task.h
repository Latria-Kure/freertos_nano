#ifndef TASK_H
#define TASK_H

#include "FreeRTOS.h"

// clang-format off
#define tskIDLE_PRIORITY			       ( ( UBaseType_t ) 0U )
#define taskYIELD() portYIELD()

typedef void* TaskHandle_t;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
    const char* const pcName,
    const uint32_t ulStackDepth,
    void* const pvParameters,
    UBaseType_t uxPriority,
    StackType_t* const puxStackBuffer,
    TCB_t* const pxTaskBuffer);
#endif /* configSUPPORT_STATIC_ALLOCATION */

void prvInitialiseTaskLists(void);
void vTaskStartScheduler(void);
void vTaskSwitchContext(void);
void vTaskDelay(const TickType_t xTicksToDelay);
// clang-format off
#define taskENTER_CRITICAL()                 portENTER_CRITICAL()
#if ( configNUMBER_OF_CORES == 1 )
    #define taskENTER_CRITICAL_FROM_ISR()    portSET_INTERRUPT_MASK_FROM_ISR()
#else
    #define taskENTER_CRITICAL_FROM_ISR()    portENTER_CRITICAL_FROM_ISR()
#endif

#define taskEXIT_CRITICAL()                    portEXIT_CRITICAL()
#if ( configNUMBER_OF_CORES == 1 )
    #define taskEXIT_CRITICAL_FROM_ISR( x )    portCLEAR_INTERRUPT_MASK_FROM_ISR( x )
#else
    #define taskEXIT_CRITICAL_FROM_ISR( x )    portEXIT_CRITICAL_FROM_ISR( x )
#endif

#define taskDISABLE_INTERRUPTS()    portDISABLE_INTERRUPTS()
#endif