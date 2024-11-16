#include "manager.h"

#include <QThread>
#include<QJsonObject>
#include<QJsonArray>
#include<QFile>
#include<QJsonDocument>
#include<QTimer>

//public:

Manager::Manager(QObject *parent)
    : QObject(parent),
    tasksModel(std::make_unique<TasksModel>()),
    workersModel(std::make_unique<WorkersModel>()),
    flagCloseApp(false)
{
    dispatchTasks();
}

QString Manager::recommendedCountWorkers()  //static
{
    return QString::number(QThread::idealThreadCount());
}

void Manager::stopWork()
{
    flagCloseApp = true;
    workersModel->stopAllWorkers();
}

TasksModel *Manager::getTasksModel() const
{
    return tasksModel.get();
}

WorkersModel *Manager::getWorkersModel() const
{
    return workersModel.get();
}

void Manager::dispatchTasks()
{
    if (flagCloseApp)
        return;

    std::shared_ptr<Worker> worker = workersModel->getFreeWorker();
    std::shared_ptr<ITask> task = tasksModel->getFreeTask();

    if (worker && task)
        worker->assignTask(task);

    QTimer::singleShot(1, this, SLOT(dispatchTasks()));
}
