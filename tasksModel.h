#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include <memory>
#include <vector>
#include <mutex>

#include <QAbstractListModel>

#include "iTask.h"

class TasksModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TasksModel(QObject *parent = nullptr);

    enum TaskRoles
    {
        TaskIdRole = Qt::UserRole + 1,
        ProgressRole,
        StatusRole,
        TypeRole
    };


    void addTask(std::unique_ptr<ITask> task);
    void deleteTask(int taskId);
    ITask* getFreeTask();
    ITask* getTaskByTaskId (int taskId);   //TODO возможно не пригодится
    int countTasksByStatus(const std::string& status) const;
    int countTasksAll() const { return tasks.size();}
    std::vector<ITask*> getAllTasks() const;

    Q_INVOKABLE void updateTask(int taskId);
    Q_INVOKABLE void sortTasksByStatus();
    Q_INVOKABLE int getOverallProgress() const;

signals:
    void progressChanged(int overallProgress);
    void tasksChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    std::vector<std::unique_ptr<ITask>> tasks;
    std::mutex mutexTasks;
};

#endif // TASKSMODEL_H
