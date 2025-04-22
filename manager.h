#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <memory>
#include <random>

#include <QObject>

#include "iTask.h"
#include "workersModel.h"
#include "tasksModel.h"
#include "numericTask.h"

class Manager : public QObject
{
    Q_OBJECT

public:
    explicit Manager(QObject *parent = nullptr);

    TasksModel* getTasksModel() const;
    WorkersModel* getWorkersModel() const;

    Q_INVOKABLE static QString recommendedCountWorkers();
    Q_INVOKABLE void addTask(short count, short type = TaskType::NumericRandom);
    Q_INVOKABLE void deleteTask(short taskId);
    Q_INVOKABLE void addWorkers(short count);
    Q_INVOKABLE void stopAllWorkers();
    Q_INVOKABLE void stopWork();

    int getTotalWorkers() const;
    int getTotalTasks() const;

    void safeTasks();
    void loadTasks();
    void clearBackupTasks();

    void safeWorkers();
    void loadWorkers();
    void clearBackupWorkers();

signals:
    void tasksChanged();
    void workersChanged();

public slots:
    void dispatchTasks();

private:
    std::unique_ptr<TasksModel> tasksModel;
    std::unique_ptr<WorkersModel> workersModel;
    bool flagCloseApp = false;

    void addRandomTask();

    template <typename T>
    void addNumericTask(std::mt19937 &gen);
};

template <typename T>
std::unique_ptr<ITask> createTask(QJsonObject &taskObj)
{
    if (typeid(T) == typeid(char)) {
        char m_start = static_cast<char>(taskObj["start"].toString().toStdString().c_str()[0]);
        char myEnd = static_cast<char>(taskObj["end"].toString().toStdString().c_str()[0]);
        char myIncrement = static_cast<char>(taskObj["increment"].toString().toStdString().c_str()[0]);
        auto task = std::make_unique<NumericTask<char>>(m_start, myEnd, myIncrement);
        return task;
    }

    if (typeid(T) == typeid(uchar)) {
        uchar m_start = static_cast<uchar>(taskObj["start"].toString().at(0).unicode());
        uchar myEnd = static_cast<uchar>(taskObj["end"].toString().at(0).unicode());
        uchar myIncrement = static_cast<uchar>(taskObj["increment"].toString().at(0).unicode());
        auto task = std::make_unique<NumericTask<uchar>>(m_start, myEnd, myIncrement);
        return task;
    }

    qint64 startBuf = taskObj["start"].toString().toLongLong();
    qint64 endBuf = taskObj["end"].toString().toLongLong();
    qint64 incrementBuf = taskObj["increment"].toString().toLongLong();

    T m_start = static_cast<T>(startBuf);
    T myEnd = static_cast<T>(endBuf);
    T myIncrement = static_cast<T>(incrementBuf);

    auto task = std::make_unique<NumericTask<T>>(m_start, myEnd, myIncrement);
    return task;
}

template <typename T>
void Manager::addNumericTask(std::mt19937 &gen)
{
    T max_range = std::numeric_limits<T>::max();
    T min_range = std::numeric_limits<T>::lowest();

    std::uniform_int_distribution<T> startDist(min_range / 2, max_range / 2);
    T m_start = startDist(gen);

    std::uniform_int_distribution<T> endDist(m_start + 100, max_range);
    T myEnd = endDist(gen);

    std::uniform_int_distribution<T> incrementDist(1, std::max<T>((myEnd - m_start) / 600, 1));
    T myIncrement = incrementDist(gen);

    int steps = (myEnd - m_start) / myIncrement;
    while (steps < 100 || steps > 600) {
        m_start = startDist(gen);
        myEnd = endDist(gen);
        myIncrement = incrementDist(gen);
        steps = (myEnd - m_start) / myIncrement;
    }

    auto task = std::make_unique<NumericTask<T>>(m_start, myEnd, myIncrement, this);
    int taskId = task->getId();
    connect(task.get(), &ITask::taskFinished, [this]            {emit tasksChanged();} );
    connect(task.get(), &ITask::taskFinished, [this, taskId]    {tasksModel->updateTask(taskId);});
    connect(task.get(), &ITask::taskDelete, this, [this, taskId]{this->deleteTask(taskId);}, Qt::QueuedConnection);
    connect(task.get(), &ITask::progressUpdated, [this, taskId] {tasksModel->updateTask(taskId);});

    tasksModel->addTask(std::move(task));
}
#endif // TASKMANAGER_H
