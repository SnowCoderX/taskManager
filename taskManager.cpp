#include "taskManager.h"

#include <iostream>

#include <QThread>
#include<QJsonObject>
#include<QJsonArray>
#include<QFile>
#include<QJsonDocument>

#include "dispatchThread.h"
#include "addDelThread.h"

//public:

TaskManager::TaskManager(QObject *parent)
    : QObject(parent),
    tasksModel(std::make_unique<TasksModel>(this)),
    workersModel(std::make_unique<WorkersModel>(this)),
    flagCloseApp(false)
{
    dispatchThread = new DispatchThread(this);
    dispatchThread->start();

    // addDelThread = new AddDelThread(this);
    // addDelThread->start();
}

QString TaskManager::recommendedCountWorkers()  //static
{
    return QString::number(QThread::idealThreadCount());
}

void TaskManager::addTask(short count, short type)
{
    for (int i = 0; i < count; ++i) {
        if (getTotalTasks() >= 1000)
            return;
        std::lock_guard<std::mutex> lock(taskMutex);
        if (type == TaskType::NumericRandom) {
            addRandomTask();
            dispatchThread->getCondition().notify_one();
        }
        if (i % 100 == 0)
            emit tasksChanged();
    }
    emit tasksChanged();
}

void TaskManager::deleteTask(short taskId)
{
    auto task = tasksModel->getTaskByTaskId(taskId);
    if (auto worker = workersModel->searchWorkerByTaskId(taskId))
        worker->stopTask();

    //TODO тут из-за мьютекса лок интерфейса
//    std::lock_guard<std::mutex> lock(taskMutex);
    tasksModel->deleteTask(taskId);
    // addDelThread->setDeleteTask(taskId);
    // tasksModel->updateTask(taskId);
    emit tasksChanged();
    emit workersChanged();
}


// void TaskManager::addWorkers(short count) {
//     if (getTotalWorkers() + count <= 1000) {
//         addDelThread->setAddWorkers(count);
//     }
// }

// void TaskManager::deleteWorker(int workerId) {
//     addDelThread->setDeleteWorker(workerId);
// }

void TaskManager::addWorkers(short count)
{
    for (int i = 0; i < count; ++i) {
        if (getTotalWorkers() >= 1000)
            return;
        auto worker = std::make_unique<Worker>(this);
        worker->start();

        connect(worker.get(), &Worker::taskFinished, this, [this](int workerId) {
            workersModel->updateWorker(workerId);
            dispatchThread->getCondition().notify_one(); // Уведомляем для новой диспетчеризации
            emit workersChanged();
        }, Qt::QueuedConnection);

        std::lock_guard<std::mutex> lock(workersMutex);
        workersModel->addWorker(std::move(worker));
        dispatchThread->getCondition().notify_one();
        emit workersChanged();
    }
}

void TaskManager::stopAllWorkers()
{
    std::lock_guard<std::mutex> lock(workersMutex);
    flagCloseApp = true;
    dispatchThread->getCondition().notify_all();

    for (auto& worker : workersModel->getAllWorkers())
        if (worker)
            worker->stop();
    emit workersChanged();
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
    std::unique_lock<std::mutex> lock(taskMutex);
    while (!flagCloseApp) {
        dispatchThread->getCondition().wait(lock);

        if (flagCloseApp)
            break;

        Worker* worker = workersModel->getFreeWorker();
        ITask* task = tasksModel->getFreeTask();

        while (worker && task) {
            worker->assignTask(task);
            task->take(worker->getId());
            worker = workersModel->getFreeWorker();
            task = tasksModel->getFreeTask();
            emit workersChanged();
            emit tasksChanged();
        }
    }
}

void TaskManager::safeTasks()
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

void TaskManager::loadTasks()
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
            connect(task.get(), &ITask::statusChanged, [this]           {tasksModel->sortTasksByStatus();});
            tasksModel->addTask(std::move(task));
            emit tasksChanged();
        }
    }
    FileTasks.close();
}

void TaskManager::clearBackupTasks()
{
    QFile FileTasks("tasks_backup.json");
    if (FileTasks.open(QIODevice::WriteOnly));
        FileTasks.close();
}

void TaskManager::safeWorkers()
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

void TaskManager::loadWorkers()
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
            dispatchThread->getCondition().notify_one();
            emit workersChanged();
        }, Qt::QueuedConnection);

        std::lock_guard<std::mutex> lock(workersMutex);
        if (worker) {
            worker->start();
            workersModel->addWorker(std::move(worker));
            emit workersChanged();
            dispatchThread->getCondition().notify_one();
        }
    }
    FileWorkers.close();
}

void TaskManager::clearBackupWorkers()
{
    QFile FileWorkers("workers_backup.json");
    if (FileWorkers.open(QIODevice::WriteOnly));
    FileWorkers.close();
}


int TaskManager::getTotalWorkers() const
{
    if (workersModel)
        return workersModel.get()->countWorkersAll();
    return 0;
}

int TaskManager::getWaitingWorkers() const
{
    if (workersModel)
        return workersModel.get()->countWorkersByStatus("Ожидает");
    return 0;
}

int TaskManager::getBusyWorkers() const
{
    if (workersModel)
        return workersModel.get()->countWorkersByStatus("Выполняет задачу №");
    return 0;
}

int TaskManager::getTotalTasks() const
{
    if (tasksModel)
        return tasksModel.get()->getCountTasksAll();
    return 0;
}

int TaskManager::getWaitingTasks() const
{
    if (tasksModel)
        return tasksModel.get()->getCountTasksByStatus("Ожидает");
    return 0;
}

int TaskManager::getInProgressTasks() const
{
    if (tasksModel)
        return tasksModel.get()->getCountTasksByStatus("Выполняет исполнитель №");
    return 0;
}

int TaskManager::getCountCompleteTasks() const
{
    if (tasksModel)
        return tasksModel.get()->getCountCompleteTasks();
    return 0;
}

void TaskManager::addInThreadTasks(short count)
{
    addDelThread->setAddTasks(count);
}

//private:

void TaskManager::addRandomTask()
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

