#include "tasksModel.h"

#include <iostream>

#include <QThread>
#include <QTimer>

//public:

TasksModel::TasksModel(QObject *parent) : QAbstractListModel(parent)
{
    QTimer *progressUpdateTimer = new QTimer(this);
    connect(progressUpdateTimer, &QTimer::timeout, this, [=]() {
        emit progressChanged(getOverallProgress());
    });
    progressUpdateTimer->start(500);    //для оптимизации, так как все равно анимация 1.6сек
}

void TasksModel::addTask(std::unique_ptr<ITask> task)
{
//    std::lock_guard<std::mutex> lock(mutexTasks);
    beginInsertRows(QModelIndex(), tasks.size(), tasks.size());
    tasks.push_back(std::move(task));
    sortTasksByStatus();
   emit tasksChanged();
//    emit progressChanged(getOverallProgress());
    endInsertRows();
}

void TasksModel::deleteTask(int taskId)
{
    auto it = std::find_if(tasks.begin(), tasks.end(), [taskId](const auto& task) {
        return task->getId() == taskId; });

    if (it != tasks.end()) {
        int index = std::distance(tasks.begin(), it);

        std::lock_guard<std::mutex> lock(mutexTasks);

        beginRemoveRows(QModelIndex(), index, index);
        tasks.erase(it);
        endRemoveRows();

        countCompleteTasks++;
        emit tasksChanged();
        // emit progressChanged(getOverallProgress());
    }
}

int TasksModel::getOverallProgress() const
{
    if (tasks.empty()) return 100;

    int totalProgress = 0;
    for (const auto& task : tasks) {
        std::lock_guard<std::mutex> lock(mutexTasks);
        if (task != nullptr)
            totalProgress += task->getProgress();
    }

    return std::min(100,
                    static_cast<int>((totalProgress + countCompleteTasks * 100) / (tasks.size() + countCompleteTasks)));
}

int TasksModel::getCountCompleteTasks() const
{
    return countCompleteTasks;
}

void TasksModel::updateTask(int taskId)
{
    for (int i = 0; i < tasks.size(); ++i) {
        std::lock_guard<std::mutex> lock(mutexTasks);
        if (tasks[i] != nullptr)
            if (tasks[i]->getId() == taskId) {
                emit dataChanged(index(i), index(i));
                // emit progressChanged(getOverallProgress());
                emit tasksChanged();
                break;
        }
    }
}

void TasksModel::sortTasksByStatus()
{
    //TODO вернись потом если время останется
    // std::sort(tasks.begin(), tasks.end(), [](const std::unique_ptr<ITask> &a, const std::unique_ptr<ITask> &b) {
    //     const std::string statusA = a->getStatus();
    //     const std::string statusB = b->getStatus();

    //     auto getStatusPriority = [](const std::string &status) {
    //         if (status.find("Выполняет исполнитель") != std::string::npos)
    //             return 0;
    //         if (status == "Ожидает")
    //             return 1;
    //         if (status == "Завершено")
    //             return 2;
    //         return 3;  // На случай неизвестного статуса
    //     };

    //     int priorityA = getStatusPriority(statusA);
    //     int priorityB = getStatusPriority(statusB);

    //     if (priorityA != priorityB)
    //         return priorityA < priorityB;

    //     return a->getId() < b->getId();
    // });

    // emit dataChanged(index(0), index(tasks.size() - 1));
}

ITask* TasksModel::getFreeTask()
{
    for (const auto& task : tasks){
        std::lock_guard<std::mutex> lock(mutexTasks);
        if (task != nullptr)
            if (task->getStatus() == "Ожидает")
                return task.get();
    }

    return nullptr;
}

ITask* TasksModel::getTaskByTaskId (int taskId)
{
    for (const auto& task : tasks){
        std::lock_guard<std::mutex> lock(mutexTasks);
        if (task != nullptr)
            if (task->getId() == taskId)
                return task.get();
    }

    return nullptr;
}

int TasksModel::getCountTasksByStatus(const std::string &status) const
{
    return std::count_if(tasks.begin(), tasks.end(), [&](const auto& task) {
        return task->getStatus().find(status) != std::string::npos;
    });
}

int TasksModel::getCountTasksAll() const
{
    return tasks.size();
}

std::vector<ITask*> TasksModel::getAllTasks() const
{
    std::vector<ITask*> allTasks;
    for (const auto& task : tasks)
        allTasks.push_back(task.get());

    return allTasks;
}

//protected:

QHash<int, QByteArray> TasksModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TaskIdRole] = "taskId";
    roles[ProgressRole] = "progress";
    roles[StatusRole] = "status";
    roles[TypeRole] = "type";
    return roles;
}

int TasksModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : tasks.size();
}

QVariant TasksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= tasks.size()) return QVariant();

    const auto& task = tasks[index.row()];
    switch (role) {
    case TaskIdRole:    return task->getId();                               break;
    case ProgressRole:  return task->getProgress();                         break;
    case StatusRole:    return QString::fromStdString(task->getStatus());   break;
    case TypeRole:      return QString::fromStdString(task->getType());     break;
    }
    return QVariant();
}
