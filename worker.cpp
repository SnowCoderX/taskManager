#include "iostream"

#include "worker.h"

#define watch(x) std::cout << (#x) << " is " << (x) << " " << __PRETTY_FUNCTION__ << " " <<  __LINE__ << std::endl;

std::atomic<int> Worker::countWorkers{0};

//public:

Worker::Worker(QObject *parent)
    : QThread(parent), workerId(++countWorkers), running(false), status("Ожидает")
{

}

void Worker::assignTask(std::shared_ptr<ITask> task)
{
    task->take(getId());
    this->task = task;
    taskId = task->getId();
    running = true;
    status = "Выполняет задачу №" +  QString::number(task->getId());
    emit changeStatus(workerId);
    taskCondition.notify_one();
}

bool Worker::isRun() const
{
    return running;
}

QString Worker::getStatus() const
{
    return status;
}

int Worker::getId() const
{
    return workerId;
}

int Worker::getTaskId() const
{
    return taskId;
}

void Worker::stop()
{
    watch(workerId);
    running = false;
    status = "Ожидает";

    if (task)
        task->changeState(TaskState::Wait);

    emit changeStatus(workerId);
    terminate();
}

void Worker::stopTask()
{
    running = false;
    status = "Ожидает";
    emit changeStatus(workerId);
    taskCondition.notify_one();
}

//protected:

void Worker::run()
{
    std::unique_lock<std::mutex> lock(taskMutex);
    while (true) {
        taskCondition.wait(lock, [this]() { return task != nullptr && running; });

        watch(workerId);
        if(task && running)
            while (!task->isCompleted() && running) {
                task->executeStep();
                QThread::msleep(200);
            }

        QThread::sleep(5);
        running = false;
        task->deleteTask();
        watch(workerId);
        status = QString("Ожидает");
        emit changeStatus(workerId);
    }
}
