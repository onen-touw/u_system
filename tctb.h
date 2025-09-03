#pragma once

#include "config.h"

namespace ufo
{
    
    namespace detail
    {

        /*  copy of real free-RTOS tskTaskControlBlock for fast task info
         * Task control block.  A task control block (TCB) is allocated for each task,
         * and stores task state information, including a pointer to the task's context
         * (the task's run time environment, including register values)
         */
        struct u_privite_tskTaskControlBlock /* The old naming convention is used to prevent breaking kernel aware debuggers. */
        {
            volatile StackType_t *pxTopOfStack; /*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */

#if (portUSING_MPU_WRAPPERS == 1)
            xMPU_SETTINGS xMPUSettings; /*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE TCB STRUCT. */
#endif

            ListItem_t xStateListItem;                                                                                                     /*< The list that the state list item of a task is reference from denotes the state of that task (Ready, Blocked, Suspended ). */
            ListItem_t xEventListItem;                                                                                                     /*< Used to reference a task from an event list. */
            UBaseType_t uxPriority;                                                                                                        /*< The priority of the task.  0 is the lowest priority. */
            StackType_t *pxStack;                                                                                                          /*< Points to the start of the stack. */
            char pcTaskName[configMAX_TASK_NAME_LEN]; /*< Descriptive name given to the task when created.  Facilitates debugging only. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */

#if (configNUMBER_OF_CORES > 1)
            BaseType_t xCoreID; /*< The core that this task is pinned to */
#endif                          /* configNUMBER_OF_CORES > 1 */

#if ((portSTACK_GROWTH > 0) || (configRECORD_STACK_HIGH_ADDRESS == 1))
            StackType_t *pxEndOfStack; /*< Points to the highest valid address for the stack. */
#endif

#if (portCRITICAL_NESTING_IN_TCB == 1)
            UBaseType_t uxCriticalNesting; /*< Holds the critical section nesting depth for ports that do not maintain their own count in the port layer. */
#endif

#if (configUSE_TRACE_FACILITY == 1)
            UBaseType_t uxTCBNumber;  /*< Stores a number that increments each time a TCB is created.  It allows debuggers to determine when a task has been deleted and then recreated. */
            UBaseType_t uxTaskNumber; /*< Stores a number specifically for use by third party trace code. */
#endif

#if (configUSE_MUTEXES == 1)
            UBaseType_t uxBasePriority; /*< The priority last assigned to the task - used by the priority inheritance mechanism. */
            UBaseType_t uxMutexesHeld;
#endif

#if (configUSE_APPLICATION_TASK_TAG == 1)
            TaskHookFunction_t pxTaskTag;
#endif

#if (configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0)
            void *pvThreadLocalStoragePointers[configNUM_THREAD_LOCAL_STORAGE_POINTERS];
#endif

#if (configGENERATE_RUN_TIME_STATS == 1)
            configRUN_TIME_COUNTER_TYPE ulRunTimeCounter; /*< Stores the amount of time the task has spent in the Running state. */
#endif

#if ((configUSE_NEWLIB_REENTRANT == 1) || (configUSE_C_RUNTIME_TLS_SUPPORT == 1))
            configTLS_BLOCK_TYPE xTLSBlock; /*< Memory block used as Thread Local Storage (TLS) Block for the task. */
#endif

#if (configUSE_TASK_NOTIFICATIONS == 1)
            volatile uint32_t ulNotifiedValue[configTASK_NOTIFICATION_ARRAY_ENTRIES];
            volatile uint8_t ucNotifyState[configTASK_NOTIFICATION_ARRAY_ENTRIES];
#endif

/* See the comments in FreeRTOS.h with the definition of
 * tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE. */
#if (tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE != 0) /*lint !e731 !e9029 Macro has been consolidated for readability reasons. */
            uint8_t ucStaticallyAllocated;           /*< Set to pdTRUE if the task is a statically allocated to ensure no attempt is made to free the memory. */
#endif

#if (INCLUDE_xTaskAbortDelay == 1)
            uint8_t ucDelayAborted;
#endif

#if (configUSE_POSIX_ERRNO == 1)
            int iTaskErrno;
#endif
        } tskTCB;

    } // namespace detail
    


} // namespace ufo
