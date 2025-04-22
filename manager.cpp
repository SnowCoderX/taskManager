#include "manager.h"

#include <QThread>
#include<QJsonObject>
#include<QJsonArray>
#include<QFile>
#include<QJsonDocument>
#include<QTimer>

#include <iostream>

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

void Manager::addTask(short count, short type)
{
    for (int i = 0; i < count; ++i) {
        if (getTotalTasks() >= 1000)
            return;
        if (type == TaskType::NumericRandom) {
            addRandomTask();
        }
        if (i % 100 == 0)
            emit tasksChanged();
    }
    emit tasksChanged();
}

void Manager::deleteTask(short taskId)
{
    auto task = tasksModel->getTaskByTaskId(taskId);
    if (auto worker = workersModel->searchWorkerByTaskId(taskId))
        worker->stopTask();

    tasksModel->deleteTask(taskId);
    emit tasksChanged();
    emit workersChanged();
}

void Manager::addWorkers(short count)
{
    for (int i = 0; i < count; ++i) {
        if (getTotalWorkers() >= 1000)
            return;
        auto worker = std::make_unique<Worker>(this);
        worker->start();

        connect(worker.get(), &Worker::taskFinished, this, [this](int workerId) {
            workersModel->updateWorker(workerId);
            emit workersChanged();
        }, Qt::QueuedConnection);

        workersModel->addWorker(std::move(worker));
        emit workersChanged();
    }
}


void Manager::stopAllWorkers()
{
    flagCloseApp = true;

    for (auto& worker : workersModel->getAllWorkers())
        if (worker)
            worker->stop();
    emit workersChanged();
}

void Manager::stopWork()
{
    flagCloseApp = true;
    workersModel->stopAllWorkers();
}

int Manager::getTotalWorkers() const
{
    if (workersModel)
        return workersModel->countWorkersAll();
    return 0;
}

int Manager::getTotalTasks() const
{
    if (tasksModel)
        return tasksModel.get()->getCountTasksAll();
    return 0;
}

void Manager::safeTasks()
{
    QJsonObject saveData;

    QJsonArray tasksArray;
    for (const auto &task : tasksModel->getAllTasks())
        if (auto serializableTask = dynamic_cast<ISerializableTask*>(task))
            if (!task->isCompleted())
                tasksArray.append(serializableTask->serialize());

    saveData["tasks"] = tasksArray;

    QFile FileTasks("tasks_backup.json");
    if (FileTasks.open(QIODevice::WriteOnly)) {
        QJsonDocument saveDoc(saveData);
        FileTasks.write(saveDoc.toJson());
        FileTasks.close();
    }
}

void Manager::loadTasks()
{
    QFile FileTasks("tasks_backup.json");

    if (!FileTasks.open(QIODevice::ReadOnly)) {
        std::cout << "Не удалось открыть файл tasks_backup.json для загрузки задач" << std::endl;
        return;
    }

    QByteArray data = FileTasks.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonArray tasksArray = loadDoc.object()["tasks"].toArray();

    for (const QJsonValue &value : tasksArray) {
        QJsonObject taskObj = value.toObject();
        QString type = taskObj["type"].toString();
        std::unique_ptr<ITask> task;

        if (type == "char")     task = createTask<char>(taskObj);
        if (type == "uchar")    task = createTask<uchar>(taskObj);
        if (type == "short")    task = createTask<short>(taskObj);
        if (type == "ushort")   task = createTask<ushort>(taskObj);
        if (type == "int")      task = createTask<int>(taskObj);
        if (type == "uint")     task = createTask<uint>(taskObj);

        if (task){
            int taskId = task->getId();
            connect(task.get(), &ITask::taskFinished, [this]            {emit tasksChanged();} );
            connect(task.get(), &ITask::taskFinished, [this, taskId]    {tasksModel->updateTask(taskId);});
            connect(task.get(), &ITask::taskDelete, this, [this, taskId]{this->deleteTask(taskId);}, Qt::QueuedConnection);
            connect(task.get(), &ITask::progressUpdated, [this, taskId] {tasksModel->updateTask(taskId);});
            tasksModel->addTask(std::move(task));
            emit tasksChanged();
        }
    }
    FileTasks.close();
}

void Manager::clearBackupTasks()
{
    QFile FileTasks("tasks_backup.json");
    if (FileTasks.open(QIODevice::WriteOnly));
    FileTasks.close();
}

void Manager::safeWorkers()
{
    QJsonObject saveData;
    QJsonArray workersArray;

    for (const auto &worker : workersModel->getAllWorkers()) {
        QJsonObject workerObj;

        workerObj["id"] = worker->getId();
        workersArray.append(workerObj);
    }

    saveData["workers"] = workersArray;

    QFile saveFile("workers_backup.json");
    if (saveFile.open(QIODevice::WriteOnly)) {
        QJsonDocument saveDoc(saveData);
        saveFile.write(saveDoc.toJson());
        saveFile.close();
    }
}

void Manager::loadWorkers()
{
    QFile FileWorkers("workers_backup.json");

    if (!FileWorkers.open(QIODevice::ReadOnly)) {
        std::cout << "Не удалось открыть файл workers_backup.json для загрузки исполнителей" << std::endl;
        return;
    }

    QByteArray data = FileWorkers.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonArray workersArray = loadDoc.object()["workers"].toArray();

    for (const QJsonValue &value : workersArray) {
        QJsonObject workerObj = value.toObject();

        std::unique_ptr<Worker> worker = std::make_unique<Worker>();

        connect(worker.get(), &Worker::taskFinished, this, [this](int workerId) {
            workersModel->updateWorker(workerId);
            emit workersChanged();
        }, Qt::QueuedConnection);

        if (worker) {
            worker->start();
            workersModel->addWorker(std::move(worker));
            emit workersChanged();
        }
    }
    FileWorkers.close();
}

void Manager::clearBackupWorkers()
{
    QFile FileWorkers("workers_backup.json");
    if (FileWorkers.open(QIODevice::WriteOnly));
    FileWorkers.close();
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

//private:

void Manager::addRandomTask()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    int typeChoice = std::uniform_int_distribution<>(0, 5)(gen);
    if (typeChoice == 0)        addNumericTask<char>(gen);
    else if (typeChoice == 1)   addNumericTask<uchar>(gen);
    else if (typeChoice == 2)   addNumericTask<short>(gen);
    else if (typeChoice == 3)   addNumericTask<ushort>(gen);
    else if (typeChoice == 4)   addNumericTask<int>(gen);
    else if (typeChoice == 5)   addNumericTask<uint>(gen);
}
