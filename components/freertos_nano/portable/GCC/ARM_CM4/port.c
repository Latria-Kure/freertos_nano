#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"
#include "task.h"

// clang-format off
/**
 * The value of the xPSR register.
 * xPSR.T = 1, to use the PSP stack.
 */
#define portINITIAL_XPSR (0x01000000)

#define portSTART_ADDRESS_MASK ((StackType_t)0xfffffffeUL)

#define portNVIC_SYSPRI2_REG (*((volatile uint32_t*)0xe000ed20))

#define portNVIC_PENDSV_PRI (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 16UL)
#define portNVIC_SYSTICK_PRI (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 24UL)

/* Masks off all bits but the VECTACTIVE bits in the ICSR register. */
#define portVECTACTIVE_MASK                   ( 0xFFUL )


#ifdef configTASK_RETURN_ADDRESS
#define portTASK_RETURN_ADDRESS configTASK_RETURN_ADDRESS
#else
#define portTASK_RETURN_ADDRESS prvTaskExitError
#endif

static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

// clang-format on
static void prvTaskExitError(void)
{
    for (;;) {
        __asm__("nop");
    }
}

/**
 * @brief Initializes the stack for a task.
 * @param pxTopOfStack Pointer to the top of the stack.
 * @param pxCode Pointer to the task function.
 * @param pvParameters Pointer to the task parameters.
 * @return Pointer to the updated top of the stack.
 */
StackType_t* pxPortInitialiseStack(StackType_t* pxTopOfStack, TaskFunction_t pxCode, void* pvParameters)
{
    pxTopOfStack--;
    *pxTopOfStack = portINITIAL_XPSR;

    /* Entry address of the task */
    pxTopOfStack--;
    *pxTopOfStack = ((StackType_t)pxCode) & portSTART_ADDRESS_MASK;

    /* R14 (LR) */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)portTASK_RETURN_ADDRESS;

    /* R12, R3, R2 and R1 initialize as 0 */
    pxTopOfStack -= 5;

    /* R0. Parameters */
    *pxTopOfStack = (StackType_t)pvParameters;

    pxTopOfStack -= 8;

    return pxTopOfStack;
}

void prvStartFirstTask(void)
{
    __asm__ __volatile__(
        "ldr r0, =0xE000ED08\n\t"
        "ldr r0, [r0]\n\t"
        "ldr r0, [r0]\n\t"
        "msr msp, r0\n\t"
        "cpsie i\n\t"
        "cpsie f\n\t"
        "dsb\n\t"
        "isb\n\t"
        "svc 0\n\t"
        "nop\n\t");
}

void vPortSetupTimerInterrupt(void)
{
    HAL_InitTick(TICK_INT_PRIORITY);
}
BaseType_t xPortStartScheduler(void)
{
    portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;

    vPortSetupTimerInterrupt();

    prvStartFirstTask();

    return 0;
}

void vPortSVCHandler(void)
{
    extern TCB_t* volatile pxCurrentTCB;

    __asm__ __volatile__(
        "ldr r3, =pxCurrentTCB\n\t"
        "ldr r1, [r3]\n\t"
        "ldr r0, [r1]\n\t"
        "ldmia r0!, {r4-r11}\n\t"
        "msr psp, r0\n\t"
        "isb\n\t" // Ensure the load of the new stack pointer takes effect before the first instruction is executed.
        "mov r0, #0\n\t"
        "msr basepri, r0\n\t"
        "orr r14, #0xd\n\t" // Exception return. Thread mode, PSP.
        "bx r14\n\t");
}

void xPortPendSVHandler(void)
{
    extern TCB_t* volatile pxCurrentTCB;
    extern void vTaskSwitchContext(void);

    __asm__ __volatile__(
        "   mrs r0, psp                         \n"
        "   isb                                 \n"
        "                                       \n"
        "   ldr r3, =pxCurrentTCB               \n" /* Get the location of the current TCB. */
        "   ldr r2, [r3]                        \n"
        "                                       \n"
        // "   tst r14, #0x10                      \n" /* Is the task using the FPU context?  If so, push high vfp registers. */
        // "   it eq                               \n"
        // "   vstmdbeq r0!, {s16-s31}             \n"
        "                                       \n"
        "   stmdb r0!, {r4-r11}                 \n" /* Save the core registers. */
        "   str r0, [r2]                        \n" /* Save the new top of stack into the first member of the TCB. */
        "                                       \n"
        "   stmdb sp!, {r3, r14}                \n"
        "   mov r0, %0                          \n"
        "   msr basepri, r0                     \n"
        "   dsb                                 \n"
        "   isb                                 \n"
        "   bl vTaskSwitchContext               \n"
        "   mov r0, #0                          \n"
        "   msr basepri, r0                     \n"
        "   ldmia sp!, {r3, r14}                 \n"
        "                                       \n"
        "   ldr r1, [r3]                        \n" /* The first item in pxCurrentTCB is the task top of stack. */
        "   ldr r0, [r1]                        \n"
        "                                       \n"
        "   ldmia r0!, {r4-r11}                 \n" /* Pop the core registers. */
        "                                       \n"
        // "   tst r14, #0x10                      \n" /* Is the task using the FPU context?  If so, pop the high vfp registers too. */
        // "   it eq                               \n"
        // "   vldmiaeq r0!, {s16-s31}             \n"
        "                                       \n"
        "   msr psp, r0                         \n"
        "   isb                                 \n"
        "                                       \n"
#ifdef WORKAROUND_PMU_CM001 /* XMC4000 specific errata workaround. */
#if WORKAROUND_PMU_CM001 == 1
        "           push { r14 }                \n"
        "           pop { pc }                  \n"
#endif
#endif
        "                                       \n"
        "   bx r14                              \n"
        "   nop                                 \n"
        "   .align 4                            \n" ::"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY));
}

void vPortEnterCritical(void)
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;

    /* This is not the interrupt safe version of the enter critical function so
     * assert() if it is being called from an interrupt context.  Only API
     * functions that end in "FromISR" can be used in an interrupt.  Only assert if
     * the critical nesting count is 1 to protect against recursive calls if the
     * assert function also uses a critical section. */
    if (uxCriticalNesting == 1) {
        configASSERT((portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK) == 0);
    }
}
/*-----------------------------------------------------------*/

void vPortExitCritical(void)
{
    configASSERT(uxCriticalNesting);
    uxCriticalNesting--;

    if (uxCriticalNesting == 0) {
        portENABLE_INTERRUPTS();
    }
}

void xPortSysTickHandler(void)
{
    portDISABLE_INTERRUPTS();

    xTaskIncrementTick();

    portENABLE_INTERRUPTS();
}