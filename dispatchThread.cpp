#include "dispatchThread.h"

#include <iostream>

void DispatchThread::run()
{
    taskManager->dispatchTasks();
}

void DispatchThread::stop()
{
    std::lock_guard<std::mutex> lock(mutex);
    flagCloseApp = true;
}
