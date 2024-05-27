#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// clang-format off
#define configUSE_16_BIT_TICKS                      0
#define configMAX_TASK_NAME_LEN                     (16)
#define configSUPPORT_STATIC_ALLOCATION             1
#define configMAX_PRIORITIES                        (5)
#define configUSE_PREEMPTION                        1
#define configUSE_TIME_SLICING                      1

#define configKERNEL_INTERRUPT_PRIORITY             255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY        191

#define xPortPendSVHandler                          PendSV_Handler
#define xPortSysTickHandler                         SysTick_Handler
#define vPortSVCHandler                             SVC_Handler

#define configUSE_PORT_OPTIMISED_TASK_SELECTION    1
#define configASSERT( x )         \
    if( ( x ) == 0 )              \
    {                             \
        taskDISABLE_INTERRUPTS(); \
        for( ; ; )                \
        ;                         \
    }
#endif /* FREERTOS_CONFIG_H */
