#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <memory>
#include <condition_variable>
#include <random>

#include <QObject>

#include "workersModel.h"
#include "tasksModel.h"
#include "numericTask.h"

class DispatchThread;
class AddDelThread;

class TaskManager : public QObject
{
    Q_OBJECT
    friend AddDelThread;
    friend DispatchThread;

public:
    explicit TaskManager(QObject *parent = nullptr);

    TasksModel* getTasksModel() const;
    WorkersModel* getWorkersModel() const;
    void dispatchTasks();

    void safeTasks();
    void loadTasks();

    void safeWorkers();
    void loadWorkers();

    Q_INVOKABLE static QString recommendedCountWorkers();
    Q_INVOKABLE void addTask(short count, short type = TaskType::NumericRandom);
    Q_INVOKABLE void addInThreadTasks(short count);
    Q_INVOKABLE void deleteTask(short taskId);
    Q_INVOKABLE void addWorkers(short count);
    Q_INVOKABLE void stopAllWorkers();

    //Статистика исполнителей
    Q_PROPERTY(int totalWorkers READ getTotalWorkers NOTIFY workersChanged)
    Q_PROPERTY(int waitingWorkers READ getWaitingWorkers NOTIFY workersChanged)
    Q_PROPERTY(int busyWorkers READ getBusyWorkers NOTIFY workersChanged)
    int getTotalWorkers() const;
    int getWaitingWorkers() const;
    int getBusyWorkers() const;

    //Статистика задач
    Q_PROPERTY(int totalTasks READ getTotalTasks NOTIFY tasksChanged)
    Q_PROPERTY(int waitingTasks READ getWaitingTasks NOTIFY tasksChanged)
    Q_PROPERTY(int inProgressTasks READ getInProgressTasks NOTIFY tasksChanged)
    int getTotalTasks() const;
    int getWaitingTasks() const;
    int getInProgressTasks() const;


signals:
    void tasksChanged();
    void workersChanged();

private:
    std::unique_ptr<TasksModel> tasksModel;
    std::unique_ptr<WorkersModel> workersModel;
    mutable std::mutex taskMutex;
    mutable std::mutex workersMutex;
    std::condition_variable dispetchCondition;
    DispatchThread* dispatchThread = nullptr;
    AddDelThread* addDelThread = nullptr;

    bool flagCloseApp = false;

    // void dispatchTasks();
    void addRandomTask();

    template <typename T>
    void addNumericTask(std::mt19937 &gen);
};

//-------------------------------------------------------------------------------//

template <typename T>
std::unique_ptr<ITask> createTask(QJsonObject &taskObj)
{
    if (typeid(T) == typeid(char)) {
        char m_start = static_cast<char>(taskObj["start"].toString().toStdString().c_str()[0]);
        char m_end = static_cast<char>(taskObj["end"].toString().toStdString().c_str()[0]);
        char m_increment = static_cast<char>(taskObj["increment"].toString().toStdString().c_str()[0]);
        auto task = std::make_unique<NumericTask<char>>(m_start, m_end, m_increment);
        return task;
    }

    if (typeid(T) == typeid(uchar)) {
        uchar m_start = static_cast<uchar>(taskObj["start"].toString().at(0).unicode());
        uchar m_end = static_cast<uchar>(taskObj["end"].toString().at(0).unicode());
        uchar m_increment = static_cast<uchar>(taskObj["increment"].toString().at(0).unicode());
        auto task = std::make_unique<NumericTask<uchar>>(m_start, m_end, m_increment);
        return task;
    }

    qint64 startBuf = taskObj["start"].toString().toLongLong();
    qint64 endBuf = taskObj["end"].toString().toLongLong();
    qint64 incrementBuf = taskObj["increment"].toString().toLongLong();

    T m_start = static_cast<T>(startBuf);
    T m_end = static_cast<T>(endBuf);
    T m_increment = static_cast<T>(incrementBuf);

    auto task = std::make_unique<NumericTask<T>>(m_start, m_end, m_increment);
    return task;
}

template <typename T>
void TaskManager::addNumericTask(std::mt19937 &gen)
{
    T max_range = std::numeric_limits<T>::max();
    T min_range = std::numeric_limits<T>::lowest();

    std::uniform_int_distribution<T> startDist(min_range / 2, max_range / 2);
    T m_start = startDist(gen);

    std::uniform_int_distribution<T> endDist(m_start + 100, max_range);
    T m_end = endDist(gen);

    std::uniform_int_distribution<T> incrementDist(1, std::max<T>((m_end - m_start) / 600, 1));
    T m_increment = incrementDist(gen);

    int steps = (m_end - m_start) / m_increment;
    while (steps < 100 || steps > 600) {
        m_start = startDist(gen);
        m_end = endDist(gen);
        m_increment = incrementDist(gen);
        steps = (m_end - m_start) / m_increment;
    }

    auto task = std::make_unique<NumericTask<T>>(m_start, m_end, m_increment, this);
    int taskId = task->getId();
    connect(task.get(), &ITask::taskFinished, [this]            {emit tasksChanged();} );
    connect(task.get(), &ITask::taskFinished, [this, taskId]    {tasksModel->updateTask(taskId);});
    // connect(task.get(), &ITask::taskFinished, [this, taskId]    {this->deleteTask(taskId);});
    connect(task.get(), &ITask::progressUpdated, [this, taskId] {tasksModel->updateTask(taskId);});
    connect(task.get(), &ITask::statusChanged, [this]           {tasksModel->sortTasksByStatus();});

    tasksModel->addTask(std::move(task));
}
#endif // TASKMANAGER_H
