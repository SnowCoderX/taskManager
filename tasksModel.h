#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include <memory>
#include <vector>
#include <mutex>
#include <random>

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
    ITask* getFreeTask();
    int getCountTasksByStatus(const QString& status) const;

    Q_PROPERTY(int totalTasks READ getCountTasksAll NOTIFY tasksChanged)
    Q_PROPERTY(int waitingTasks READ getCountWaitingTasks NOTIFY tasksChanged)
    Q_PROPERTY(int inProgressTasks READ getCountInProgressTasks NOTIFY tasksChanged)

    Q_INVOKABLE void addNumericTask(short count);
    Q_INVOKABLE void deleteTask(int taskId);
    Q_INVOKABLE int getCountTasksAll() const;
    Q_INVOKABLE int getCountWaitingTasks();
    Q_INVOKABLE int getCountInProgressTasks();

    Q_INVOKABLE void updateTask(int taskId);
    Q_INVOKABLE int getOverallProgress() const;

signals:
    void progressChanged(int overallProgress);
    void tasksChanged();
    void emSort();

protected:
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QThread* threadTaskModel;
    std::vector<std::unique_ptr<ITask>> tasks;
    mutable std::mutex mutexTasks;

    template <typename T>
    void addRandomNumericTask(std::mt19937 &gen);
};

#endif // TASKSMODEL_H
