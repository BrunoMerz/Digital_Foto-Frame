#include "myDebug.h"
#include "myTime.h"

MyDebug* MyDebug::instance = 0;

MyDebug *MyDebug::getInstance() {
  if (!instance)
  {
      instance = new MyDebug();
  }
  return instance;
}

MyDebug::MyDebug(void) {
    logBuf = "";
    mt = MyTime::getInstance();
}

void MyDebug::log(const char *logLine) {
    if(logBuf.length() > 32768)
        logBuf="";
    logBuf += mt->getTimeStamp();
    logBuf += logLine;
    logBuf += "\n";
}

void MyDebug::log(String logLine) {
    log((const char *)logLine.c_str());
}

void MyDebug::log(int logLine) {
    log(String(logLine));
}

String MyDebug::getLog(void) {
    return logBuf;
}

void MyDebug::debugSysEnv(void) {
    char buf[1024];
    TaskStatus_t *tasks;
    UBaseType_t count = uxTaskGetNumberOfTasks();
    tasks = (TaskStatus_t*)malloc(count * sizeof(TaskStatus_t));
    uxTaskGetSystemState(tasks, count, NULL);
    vTaskList(buf);
    strcat(buf, "\n");
    log(buf);

    for (UBaseType_t i = 0; i < count; i++)
    {
        snprintf(buf,sizeof(buf),
            "%-16s Core:%d State:%d Prio:%u Stack:%u",
            tasks[i].pcTaskName,
            tasks[i].xCoreID,
            tasks[i].eCurrentState,
            tasks[i].uxCurrentPriority,
            tasks[i].usStackHighWaterMark
        );
        log(buf);
    }

    free(tasks);
}