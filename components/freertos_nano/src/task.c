#include "task.h"

TCB_t* volatile pxCurrentTCB = NULL;
List_t pxReadyTasksLists[configMAX_PRIORITIES];

static volatile UBaseType_t uxCurrentNumberOfTasks = (UBaseType_t)0U;
static TaskHandle_t xIdleTaskHandle = NULL;
static volatile TickType_t xTickCount = (TickType_t)0U;

static portTASK_FUNCTION(prvIdleTask, pvParameters)
{
    (void)pvParameters;

    for (;;) {
    }
}

static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,
    const char* const pcName,
    const uint32_t ulStackDepth,
    void* const pvParameters,
    TaskHandle_t* const pxCreatedTask,
    TCB_t* pxNewTCB)
{
    StackType_t* pxTopOfStack;
    UBaseType_t x;

    /* Top of stack */
    pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1);
    pxTopOfStack = (StackType_t*)((uint32_t)pxTopOfStack & (uint32_t) ~(0x0007UL)); // 8-byte alignment. Point to the last 8-byte aligned address.

    /* Task name */
    for (x = (UBaseType_t)0; x < (UBaseType_t)configMAX_TASK_NAME_LEN; x++) {
        pxNewTCB->pcTaskName[x] = pcName[x];
        if (pcName[x] == 0x00) {
            break;
        }
    }
    pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN - 1] = '\0';

    /* Initialise list */
    vListInitialiseItem(&(pxNewTCB->xStateListItem));
    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);

    /* Initialise task stack */
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters);

    if ((void*)pxCreatedTask != NULL) {
        *pxCreatedTask = (TaskHandle_t)pxNewTCB;
    }
}

#if (configSUPPORT_STATIC_ALLOCATION == 1)
/**
 * @brief Creates a new task.
 *
 * @param pxTaskCode Pointer to the task entry function.
 * @param pcName     A descriptive name for the task.
 * @param ulStackDepth The size of the task's stack in words.
 * @param pvParameters A parameter that can be passed to the task.
 * @param puxStackBuffer Pointer to the task's stack buffer.
 * @param pxTaskBuffer Pointer to the task's control block.
 * @return The handle of the created task.
 */
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
    const char* const pcName,
    const uint32_t ulStackDepth,
    void* const pvParameters,
    StackType_t* const puxStackBuffer,
    TCB_t* const pxTaskBuffer)
{
    TCB_t* pxNewTCB;
    TaskHandle_t xReturn;

    if ((pxTaskBuffer != NULL) && (puxStackBuffer != NULL)) {
        pxNewTCB = (TCB_t*)pxTaskBuffer;
        pxNewTCB->pxStack = (StackType_t*)puxStackBuffer;
        prvInitialiseNewTask(pxTaskCode, pcName, ulStackDepth, pvParameters, &xReturn, pxNewTCB);
    } else {
        xReturn = NULL;
    }

    return xReturn;
}
#endif

void prvInitialiseTaskLists(void)
{
    UBaseType_t uxPriority;

    for (uxPriority = (UBaseType_t)0U; uxPriority < (UBaseType_t)configMAX_PRIORITIES; uxPriority++) {
        vListInitialise(&(pxReadyTasksLists[uxPriority]));
    }
}

extern TCB_t Task1TCB;
extern TCB_t Task2TCB;
extern TCB_t IdleTaskTCB;
void vApplicationGetIdleTaskMemory(TCB_t** ppxIdleTaskTCBBuffer,
    StackType_t** ppxIdleTaskStackBuffer,
    uint32_t* pulIdleTaskStackSize);

void vTaskStartScheduler(void)
{
#if 1
    /* Create Idll task */
    TCB_t* pxIdleTaskTCBBuffer;
    StackType_t* pxIdleTaskStackBuffer;
    uint32_t ulIdleTaskStackSize;

    vApplicationGetIdleTaskMemory(&pxIdleTaskTCBBuffer,
        &pxIdleTaskStackBuffer,
        &ulIdleTaskStackSize);

    xIdleTaskHandle = xTaskCreateStatic((TaskFunction_t)prvIdleTask,
        (char*)"IDLE",
        (uint32_t)ulIdleTaskStackSize,
        (void*)NULL,
        (StackType_t*)pxIdleTaskStackBuffer,
        (TCB_t*)pxIdleTaskTCBBuffer);

    vListInsertEnd(&(pxReadyTasksLists[0]), &(((TCB_t*)pxIdleTaskTCBBuffer)->xStateListItem));
#endif
    /* Start scheduler */
    pxCurrentTCB = &Task1TCB;

    if (xPortStartScheduler() != pdFALSE) {
        /* Should not reach here */
    }
}

void vTaskSwitchContext(void)
{
#if 0
    if (pxCurrentTCB == &Task1TCB) {
        pxCurrentTCB = &Task2TCB;
    } else {
        pxCurrentTCB = &Task1TCB;
    }
#endif
#if 1
    /* Executing idle task. Check other task's delay ticks and try to switch. */
    if (pxCurrentTCB == &IdleTaskTCB) {
        if (Task1TCB.xTicksToDelay == 0) {
            pxCurrentTCB = &Task1TCB;
        } else if (Task2TCB.xTicksToDelay == 0) {
            pxCurrentTCB = &Task2TCB;
        } else {
            return;
        }
    } else {
        if (pxCurrentTCB == &Task1TCB) {
            if (Task2TCB.xTicksToDelay == 0) {
                pxCurrentTCB = &Task2TCB;
            } else if (pxCurrentTCB->xTicksToDelay != 0) {
                pxCurrentTCB = &IdleTaskTCB;
            } else {
                return;
            }
        } else if (pxCurrentTCB == &Task2TCB) {
            if (Task1TCB.xTicksToDelay == 0) {
                pxCurrentTCB = &Task1TCB;
            } else if (pxCurrentTCB->xTicksToDelay != 0) {
                pxCurrentTCB = &IdleTaskTCB;
            } else {
                return;
            }
        }
    }
#endif
}

void vTaskDelay(const TickType_t xTicksToDelay)
{
    TCB_t* pxTCB = NULL;

    pxTCB = pxCurrentTCB;

    pxTCB->xTicksToDelay = xTicksToDelay;

    portYIELD();
}

void xTaskIncrementTick(void)
{
    TCB_t* pxTCB = NULL;
    BaseType_t i = 0;

    const TickType_t xConstTickCount = xTickCount + 1;
    xTickCount = xConstTickCount;

    for (i = 0; i < configMAX_PRIORITIES; i++) {
        pxTCB = (TCB_t*)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
        if (pxTCB->xTicksToDelay > 0) {
            pxTCB->xTicksToDelay--;
        }
    }

    portYIELD();
}
