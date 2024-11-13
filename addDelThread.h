#ifndef ADDDELTHREAD_H
#define ADDDELTHREAD_H

#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>

#include <QObject>

#include <iTask.h>

class TaskManager;
class Worker;

class AddDelThread : public QObject {
    Q_OBJECT

public:
    explicit AddDelThread(TaskManager* taskManager) : taskManager(taskManager), countAddWorkers(0), flagCloseApp(false) {}

    void run();

    void setAddWorkers(short count);
    void setDeleteWorker(int workerId);

    void setAddTasks(short count) ;
    void setDeleteTask(int taskId);

    // Сигналы
signals:
    void taskAdded(std::shared_ptr<ITask> task);
    void workersChanged();

private:
    TaskManager* taskManager;
    short countAddTask = 0;
    std::mutex mutex;
    std::condition_variable condition;
    std::vector<int> vecWorkerDelete;
    std::vector<short> vecTaskDelete;
    short countAddWorkers;
    bool flagCloseApp;

    template <typename T>
    std::unique_ptr<ITask> createRandomTask();
};

#endif // ADDDELTHREAD_H
