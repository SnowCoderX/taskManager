#ifndef DISPATCHTHREAD_H
#define DISPATCHTHREAD_H

#include<condition_variable>
#include<mutex>

#include<QThread>

#include<taskManager.h>

class DispatchThread : public QThread
{
    Q_OBJECT

public:
    explicit DispatchThread(TaskManager* taskManager)
        : taskManager(taskManager), flagCloseApp(false) {}

    void run() override;
    void stop();

    std::condition_variable& getCondition();
    std::mutex& getMutex();
    bool getFlagCloseApp() const;

private:
    TaskManager* taskManager = nullptr;
    bool flagCloseApp;
    std::condition_variable condition;
    std::mutex mutex;
};

#endif // DISPATCHTHREAD_H
