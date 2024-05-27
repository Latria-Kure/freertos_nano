#ifndef PORTMACRO_H
#define PORTMACRO_H

#include "FreeRTOSConfig.h"
#include "stddef.h"
#include "stdint.h"

// clang-format off
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		long
#define portSHORT		short
#define portSTACK_TYPE	uint32_t
#define portBASE_TYPE	long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
	typedef uint16_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffff
#else
	typedef uint32_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffffffffUL

#endif

#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
    #define configUSE_PORT_OPTIMISED_TASK_SELECTION    1
#endif

#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )
/* portNOP() is not required by this port. */
#define portNOP()

#define portINLINE              __inline

#ifndef portFORCE_INLINE
    #define portFORCE_INLINE    inline __attribute__( ( always_inline ) )
#endif

/* Scheduler utilities. */
#define portNVIC_INT_CTRL_REG		( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT		( 1UL << 28UL )

#define portSY_FULL_READ_WRITE		( 15 )


#define portYIELD()                                     \
    {                                                   \
        /* Set a PendSV to request a context switch. */ \
        portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; \
                                                        \
        /* Barriers are normally not required but do ensure the code is completely \
         * within the specified behaviour for the architecture. */ \
        __asm volatile ( "dsb" ::: "memory" );                     \
        __asm volatile ( "isb" );                                  \
    }

/*-----------------------------------------------------------*/
/* Architecture specific optimisations. 
 * By using this optimised method, the uxTopReadyPriority variable is treated as a bit map, not a number.
 * So now there are only 32 possible priorities, and the number of priorities is limited to 32.
 * uxTopPriority is still treated as a number for it will be used to subscript the pxReadyTasksLists. 
 */
#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
    #define configUSE_PORT_OPTIMISED_TASK_SELECTION    1
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1
/* Generic helper function. */
    __attribute__( ( always_inline ) ) static inline uint8_t ucPortCountLeadingZeros( uint32_t ulBitmap )
    {
        uint8_t ucReturn;
        __asm volatile ( "clz %0, %1" : "=r" ( ucReturn ) : "r" ( ulBitmap ) : "memory" );
        return ucReturn;
    }

/* Check the configuration. */
    #if ( configMAX_PRIORITIES > 32 )
        #error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
    #endif

/* Store/clear the ready priorities in a bit map. */
    #define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )    ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
    #define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )     ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

    #define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )    uxTopPriority = ( 31UL - ( uint32_t ) ucPortCountLeadingZeros( ( uxReadyPriorities ) ) )

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */
/*-----------------------------------------------------------*/

/* Critical section management. */
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
#define portSET_INTERRUPT_MASK_FROM_ISR()         ulPortRaiseBASEPRI()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR( x )    vPortSetBASEPRI( x )
#define portDISABLE_INTERRUPTS()                  vPortRaiseBASEPRI()
#define portENABLE_INTERRUPTS()                   vPortSetBASEPRI( 0 )
#define portENTER_CRITICAL()                      vPortEnterCritical()
#define portEXIT_CRITICAL()                       vPortExitCritical()

portFORCE_INLINE static void vPortRaiseBASEPRI( void )
{
    uint32_t ulNewBASEPRI;

    __asm__ __volatile__(
        "   mov %0, %1                                              \n" \
        "   msr basepri, %0                                         \n" \
        "   isb                                                     \n" \
        "   dsb                                                     \n" \
        : "=r" ( ulNewBASEPRI ) : "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ) : "memory"
    );
}

portFORCE_INLINE static uint32_t ulPortRaiseBASEPRI( void )
{
    uint32_t ulOriginalBASEPRI, ulNewBASEPRI;

    __asm__ __volatile__(
        "   mrs %0, basepri                                         \n" \
        "   mov %1, %2                                              \n" \
        "   msr basepri, %1                                         \n" \
        "   isb                                                     \n" \
        "   dsb                                                     \n" \
        : "=r" ( ulOriginalBASEPRI ), "=r" ( ulNewBASEPRI ) : "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ) : "memory"
    );

    /* This return will not be reached but is necessary to prevent compiler
     * warnings. */
    return ulOriginalBASEPRI;
}

portFORCE_INLINE static void vPortSetBASEPRI( uint32_t ulNewMaskValue )
{
    __asm__ __volatile__(
        "   msr basepri, %0 " ::"r" ( ulNewMaskValue ) : "memory"
    );
}
/*-----------------------------------------------------------*/
#endif /* PORTMACRO_H */
