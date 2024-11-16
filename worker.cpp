#include "worker.h"

std::atomic<int> Worker::countWorkers{0};

//public:

Worker::Worker(QObject *parent)
    : QThread(parent), workerId(++countWorkers), running(false), status("Ожидает")
{

}

void Worker::assignTask(std::shared_ptr<ITask> task)
{
    std::lock_guard<std::mutex> lock(taskMutex);
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
    running = false;
    status = "Ожидает";

    //МОМЕНТ ДЛЯ ДИСКУСИИ: По идее, если задача уже выполнена, но она в ожидании своих "завершающих" 5 секунд и
    //в этот момент удаляется воркер, то задача недозавершилась получается корректно, а завершением задачи ведь
    //типа занимается воркер, поэтому я считаю логичным ставить и в этом случае статус у задачи "ожидает", так
    //как несмотря на то, что она выполнена, она еще корректно не завершена и она ждет воркера чтобы он завершил
    if (task)
        task->changeState(TaskState::Wait);

    emit changeStatus(workerId);
    terminate();
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
                QThread::msleep(200);
            }

        QThread::sleep(5);
        running = false;
        task->deleteTask();
        status = QString("Ожидает");
        emit changeStatus(workerId);
    }
}
