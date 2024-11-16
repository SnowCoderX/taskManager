#include "taskManager.h"

#include <QThread>
#include<QJsonObject>
#include<QJsonArray>
#include<QFile>
#include<QJsonDocument>
#include<QTimer>

//public:

TaskManager::TaskManager(QObject *parent)
    : QObject(parent),
    tasksModel(std::make_unique<TasksModel>()),
    workersModel(std::make_unique<WorkersModel>()),
    flagCloseApp(false)
{
    dispatchTasks();
}

QString TaskManager::recommendedCountWorkers()  //static
{
    return QString::number(QThread::idealThreadCount());
}

void TaskManager::stopWork()
{
    flagCloseApp = true;
    workersModel->stopAllWorkers();
    //TODO ремув всё нах, деструктурируем по полной
}

TasksModel *TaskManager::getTasksModel() const
{
    return tasksModel.get();
}

WorkersModel *TaskManager::getWorkersModel() const
{
    return workersModel.get();
}

void TaskManager::dispatchTasks()
{
    if (flagCloseApp)
        return;

    std::shared_ptr<Worker> worker = workersModel->getFreeWorker();
    std::shared_ptr<ITask> task = tasksModel->getFreeTask();

    if (worker && task) {
        worker->assignTask(task);
        task->take(worker->getId());
    }

    QTimer::singleShot(1, this, SLOT(dispatchTasks()));
}
