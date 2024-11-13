#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <memory>
#include <condition_variable>
#include <random>

#include <QObject>

#include "workersModel.h"
#include "tasksModel.h"

class DispatchThread;

class TaskManager : public QObject
{
    Q_OBJECT
    friend DispatchThread;

public:
    explicit TaskManager(QObject *parent = nullptr);

    TasksModel* getTasksModel() const;
    WorkersModel* getWorkersModel() const;
    void dispatchTasks();

    void safeTasks();
    void loadTasks();
    void clearBackupTasks();

    void safeWorkers();
    void loadWorkers();
    void clearBackupWorkers();

    Q_INVOKABLE static QString recommendedCountWorkers();
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
    bool flagCloseApp = false;
};
#endif // TASKMANAGER_H
