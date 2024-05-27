#include "task.h"
// clang-format off
TCB_t* volatile pxCurrentTCB = NULL;
List_t pxReadyTasksLists[configMAX_PRIORITIES];

static volatile UBaseType_t uxCurrentNumberOfTasks      = (UBaseType_t)0U;
static UBaseType_t uxTaskNumber 					    = ( UBaseType_t ) 0U;
static TaskHandle_t xIdleTaskHandle                     = NULL;
static volatile TickType_t xTickCount                   = (TickType_t)0U;
static volatile UBaseType_t uxTopReadyPriority 		    = tskIDLE_PRIORITY;

// clang-format on

#if (configUSE_PORT_OPTIMISED_TASK_SELECTION == 0)

/* If configUSE_PORT_OPTIMISED_TASK_SELECTION is 0 then task selection is
 * performed in a generic way that is not optimised to any particular
 * microcontroller architecture. */

/* uxTopReadyPriority holds the priority of the highest priority ready
 * state task. */
#define taskRECORD_READY_PRIORITY(uxPriority)    \
    do {                                         \
        if ((uxPriority) > uxTopReadyPriority) { \
            uxTopReadyPriority = (uxPriority);   \
        }                                        \
    } while (0) /* taskRECORD_READY_PRIORITY */

/*-----------------------------------------------------------*/

#if (configNUMBER_OF_CORES == 1)
#define taskSELECT_HIGHEST_PRIORITY_TASK()                                              \
    do {                                                                                \
        UBaseType_t uxTopPriority = uxTopReadyPriority;                                 \
                                                                                        \
        /* Find the highest priority queue that contains ready tasks. */                \
        while (listLIST_IS_EMPTY(&(pxReadyTasksLists[uxTopPriority])) != pdFALSE) {     \
            configASSERT(uxTopPriority);                                                \
            --uxTopPriority;                                                            \
        }                                                                               \
                                                                                        \
        /* listGET_OWNER_OF_NEXT_ENTRY indexes through the list, so the tasks of        \
         * the  same priority get an equal share of the processor time. */              \
        listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, &(pxReadyTasksLists[uxTopPriority])); \
        uxTopReadyPriority = uxTopPriority;                                             \
    } while (0) /* taskSELECT_HIGHEST_PRIORITY_TASK */
#else /* if ( configNUMBER_OF_CORES == 1 ) */

#define taskSELECT_HIGHEST_PRIORITY_TASK(xCoreID) prvSelectHighestPriorityTask(xCoreID)

#endif /* if ( configNUMBER_OF_CORES == 1 ) */

/*-----------------------------------------------------------*/

/* Define away taskRESET_READY_PRIORITY() and portRESET_READY_PRIORITY() as
 * they are only required when a port optimised method of task selection is
 * being used. */
#define taskRESET_READY_PRIORITY(uxPriority)
#define portRESET_READY_PRIORITY(uxPriority, uxTopReadyPriority)

#else /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

/* If configUSE_PORT_OPTIMISED_TASK_SELECTION is 1 then task selection is
 * performed in a way that is tailored to the particular microcontroller
 * architecture being used. */

/* A port optimised version is provided.  Call the port defined macros. */
#define taskRECORD_READY_PRIORITY(uxPriority) portRECORD_READY_PRIORITY((uxPriority), uxTopReadyPriority)

/*-----------------------------------------------------------*/

#define taskSELECT_HIGHEST_PRIORITY_TASK()                                              \
    do {                                                                                \
        UBaseType_t uxTopPriority;                                                      \
                                                                                        \
        /* Find the highest priority list that contains ready tasks. */                 \
        portGET_HIGHEST_PRIORITY(uxTopPriority, uxTopReadyPriority);                    \
        configASSERT(listCURRENT_LIST_LENGTH(&(pxReadyTasksLists[uxTopPriority])) > 0); \
        listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, &(pxReadyTasksLists[uxTopPriority])); \
    } while (0)

/*-----------------------------------------------------------*/

/* A port optimised version is provided, call it only if the TCB being reset
 * is being referenced from a ready list.  If it is referenced from a delayed
 * or suspended list then it won't be in a ready list. */

#define taskRESET_READY_PRIORITY(uxPriority)                          \
    do {                                                              \
        portRESET_READY_PRIORITY((uxPriority), (uxTopReadyPriority)); \
    } while (0)
#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

#define prvAddTaskToReadyList(pxTCB)                                                           \
    do {                                                                                       \
        taskRECORD_READY_PRIORITY((pxTCB)->uxPriority);                                        \
        vListInsertEnd(&(pxReadyTasksLists[(pxTCB)->uxPriority]), &((pxTCB)->xStateListItem)); \
    } while (0)

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
    UBaseType_t uxPriority,
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

    /* Initialise priority */
    if (uxPriority >= (UBaseType_t)configMAX_PRIORITIES) {
        uxPriority = (UBaseType_t)configMAX_PRIORITIES - 1U;
    }
    pxNewTCB->uxPriority = uxPriority;

    /* Initialise task stack */
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters);

    if ((void*)pxCreatedTask != NULL) {
        *pxCreatedTask = (TaskHandle_t)pxNewTCB;
    }
}

static void prvAddNewTaskToReadyList(TCB_t* pxNewTCB)
{
    taskENTER_CRITICAL();
    {
        uxCurrentNumberOfTasks++;

        if (pxCurrentTCB == NULL) {
            pxCurrentTCB = pxNewTCB;

            if (uxCurrentNumberOfTasks == (UBaseType_t)1) {
                prvInitialiseTaskLists();
            }

        } else {
            if (pxNewTCB->uxPriority > pxCurrentTCB->uxPriority) {
                pxCurrentTCB = pxNewTCB;
            }
        }
        prvAddTaskToReadyList(pxNewTCB);
    }
    taskEXIT_CRITICAL();
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
TaskHandle_t
xTaskCreateStatic(TaskFunction_t pxTaskCode,
    const char* const pcName,
    const uint32_t ulStackDepth,
    void* const pvParameters,
    UBaseType_t uxPriority,
    StackType_t* const puxStackBuffer,
    TCB_t* const pxTaskBuffer)
{
    TCB_t* pxNewTCB;
    TaskHandle_t xReturn;

    if ((pxTaskBuffer != NULL) && (puxStackBuffer != NULL)) {
        pxNewTCB = (TCB_t*)pxTaskBuffer;
        pxNewTCB->pxStack = (StackType_t*)puxStackBuffer;
        prvInitialiseNewTask(pxTaskCode, pcName, ulStackDepth, pvParameters, uxPriority, &xReturn, pxNewTCB);
        prvAddNewTaskToReadyList(pxNewTCB);
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
        (UBaseType_t)tskIDLE_PRIORITY,
        (StackType_t*)pxIdleTaskStackBuffer,
        (TCB_t*)pxIdleTaskTCBBuffer);

    if (xPortStartScheduler() != pdFALSE) {
        /* Should not reach here */
    }
}

void vTaskSwitchContext(void)
{
    taskSELECT_HIGHEST_PRIORITY_TASK();
}

void vTaskDelay(const TickType_t xTicksToDelay)
{
    TCB_t* pxTCB = NULL;

    pxTCB = pxCurrentTCB;

    pxTCB->xTicksToDelay = xTicksToDelay;

    // not implemented yet
    // uxListRemove(&(pxTCB->xStateListItem));
    taskRESET_READY_PRIORITY(pxTCB->uxPriority);

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

            if (pxTCB->xTicksToDelay == 0) {
                taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
            }
        }
    }

    portYIELD();
}
