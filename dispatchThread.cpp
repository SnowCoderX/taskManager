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

std::condition_variable &DispatchThread::getCondition()
{
    return condition;
}

std::mutex &DispatchThread::getMutex()
{
    return mutex;
}

bool DispatchThread::getFlagCloseApp() const
{
    return flagCloseApp;
}
