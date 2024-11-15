#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <memory>
#include <condition_variable>
#include <random>

#include <QObject>

#include "workersModel.h"
#include "tasksModel.h"

// class DispatchThread;

class TaskManager : public QObject
{
    Q_OBJECT
    // friend DispatchThread;

public:
    explicit TaskManager(QObject *parent = nullptr);

    TasksModel* getTasksModel() const;
    WorkersModel* getWorkersModel() const;

    void safeTasks();
    void loadTasks();
    void clearBackupTasks();

    void safeWorkers();
    void loadWorkers();
    void clearBackupWorkers();

    Q_INVOKABLE static QString recommendedCountWorkers();
    Q_INVOKABLE void stopAllWorkers();

signals:
    void tasksChanged();
    void workersChanged();

public slots:
    void dispatchTasks();

private:
    std::unique_ptr<TasksModel> tasksModel;
    std::unique_ptr<WorkersModel> workersModel;
    mutable std::mutex taskMutex;
    mutable std::mutex workersMutex;
    std::condition_variable dispetchCondition;
    // DispatchThread* dispatchThread = nullptr;
    bool flagCloseApp = false;
};
#endif // TASKMANAGER_H
