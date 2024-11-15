#include "worker.h"
// #include <QTimer>
#include <iostream>

std::atomic<int> Worker::countWorkers{0};

//public:

Worker::Worker(QObject *parent)
    : QThread(parent), workerId(++countWorkers), running(false), status("Ожидает")
{

}

void Worker::assignTask(std::shared_ptr<ITask> task)
{
    std::lock_guard<std::mutex> lock(taskMutex);
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
    running = false;
    status = "Ожидает";
    emit changeStatus(workerId);
    taskCondition.notify_one();
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

        if(task && running)
            while (!task->isCompleted() && running) {
                task->executeStep();
                QThread::msleep(100);
            }

        task->deleteTask();
        QThread::sleep(5);
        running = false;
        status = QString("Ожидает");
        emit taskFinished(workerId);    //TODO переименовать сигнал
    }
}
