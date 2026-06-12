#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern char debugLog[2048];

void debugSysEnv(void) {
    TaskStatus_t *tasks;
    UBaseType_t count = uxTaskGetNumberOfTasks();
    tasks = (TaskStatus_t*)malloc(count * sizeof(TaskStatus_t));
    uxTaskGetSystemState(tasks, count, NULL);
    vTaskList(debugLog);
    for (UBaseType_t i = 0; i < count; i++)
    {
        char buf[100];
        snprintf(buf,sizeof(buf),
            "%-16s Core:%d State:%d Prio:%u Stack:%u\n",
            tasks[i].pcTaskName,
            tasks[i].xCoreID,
            tasks[i].eCurrentState,
            tasks[i].uxCurrentPriority,
            tasks[i].usStackHighWaterMark
        );
        strcat(debugLog,buf);
    }

    free(tasks);
}