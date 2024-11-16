#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <memory>

#include <QObject>

#include "workersModel.h"
#include "tasksModel.h"

class Manager : public QObject
{
    Q_OBJECT

public:
    explicit Manager(QObject *parent = nullptr);

    TasksModel* getTasksModel() const;
    WorkersModel* getWorkersModel() const;

    Q_INVOKABLE static QString recommendedCountWorkers();
    Q_INVOKABLE void stopWork();

signals:
    void tasksChanged();
    void workersChanged();

public slots:
    void dispatchTasks();

private:
    std::unique_ptr<TasksModel> tasksModel;
    std::unique_ptr<WorkersModel> workersModel;
    bool flagCloseApp = false;
};
#endif // TASKMANAGER_H
