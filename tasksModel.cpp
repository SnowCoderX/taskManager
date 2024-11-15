#include "tasksModel.h"

#include <iostream>

#include <QThread>
#include <QTimer>

#include "numericTask.h"
//public:

TasksModel::TasksModel(QObject *parent) : QAbstractListModel(parent)
{
    // Таймер для обновления прогресса
    QTimer *progressUpdateTimer = new QTimer();
    connect(progressUpdateTimer, &QTimer::timeout, this, [=]() {
        emit progressChanged(getOverallProgress());
    });
    progressUpdateTimer->start(500);  // Обновление каждые 500 мс
}

void TasksModel::addTask(std::shared_ptr<ITask> task)
{
    if (tasks.size() >= 1000)
        return;

    int taskId = task->getId();
    connect(task.get(), &ITask::taskFinished, [this, taskId]    { updateTask(taskId); });
    connect(task.get(), &ITask::taskDelete, this, [this, taskId]{ deleteTask(taskId); });
    connect(task.get(), &ITask::progressUpdated, [this, taskId] { updateTask(taskId); });

    beginInsertRows(QModelIndex(), tasks.size(), tasks.size());
    tasks.push_back(std::move(task));
    endInsertRows();
    emit tasksChanged();
}

int TasksModel::getOverallProgress() const
{
    if (tasks.empty()) return 100;

    int totalProgress = 0;
    for (const auto& task : tasks) {
        // std::lock_guard<std::mutex> lock(mutexTasks);
        if (task != nullptr)
            totalProgress += task->getProgress();
    }

    return std::min(100,
                    static_cast<int>(totalProgress / (tasks.size())));
}

void TasksModel::updateTask(int taskId)
{
    auto it = std::find_if(tasks.begin(), tasks.end(), [taskId](const auto& task) {
        return task->getId() == taskId; });

    if (it != tasks.end()) {
        int ind = std::distance(tasks.begin(), it);
        emit dataChanged(index(ind), index(ind));
        // emit progressChanged(getOverallProgress());
        emit tasksChanged();
    }
}

std::shared_ptr<ITask> TasksModel::getFreeTask()
{
    for (const auto& task : tasks){
        if (task != nullptr)
            if (task->getStatus() == "Ожидает")
                return task;
    }

    return nullptr;
}

int TasksModel::getCountTasksByStatus(const QString &status) const
{
    return std::count_if(tasks.begin(), tasks.end(), [&](const auto& task) {
        return task->getStatus().contains(status);
    });
}

void TasksModel::addNumericTask(short count)
{
    for (int i = 0; i < count; ++i) {
        // std::lock_guard<std::mutex> lock(taskMutex);
        std::random_device rd;
        std::mt19937 gen(rd());

        int typeChoice = std::uniform_int_distribution<>(0, 5)(gen);
        if (typeChoice == 0)        addRandomNumericTask<char>(gen);
        else if (typeChoice == 1)   addRandomNumericTask<uchar>(gen);
        else if (typeChoice == 2)   addRandomNumericTask<short>(gen);
        else if (typeChoice == 3)   addRandomNumericTask<ushort>(gen);
        else if (typeChoice == 4)   addRandomNumericTask<int>(gen);
        else if (typeChoice == 5)   addRandomNumericTask<uint>(gen);

        emit tasksChanged();
    }
}

void TasksModel::deleteTask(int taskId)
{
    auto it = std::find_if(tasks.begin(), tasks.end(), [taskId](const auto& task) {
        return task->getId() == taskId; });

    if (it != tasks.end()) {
        int index = std::distance(tasks.begin(), it);

        beginRemoveRows(QModelIndex(), index, index);
        tasks.erase(it);
        endRemoveRows();

        emit tasksChanged();
        // emit progressChanged(getOverallProgress());
    }
}

int TasksModel::getCountTasksAll() const
{
    return static_cast<int>(tasks.size());
}

int TasksModel::getCountWaitingTasks()
{
    if (tasks.size() == 0)
        return 0;

    return getCountTasksByStatus("Ожидает");
}

int TasksModel::getCountInProgressTasks()
{
    if (tasks.size() == 0)
        return 0;

    return getCountTasksByStatus("Выполняет исполнитель №");
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
    case StatusRole:    return task->getStatus();   break;
    case TypeRole:      return QString::fromStdString(task->getType());     break;
    }
    return QVariant();
}

template <typename T>
void TasksModel::addRandomNumericTask(std::mt19937 &gen)
{
    T min_range = std::numeric_limits<T>::lowest();
    T max_range = std::numeric_limits<T>::max();

    T m_start = min_range;

    int maxSteps = 300;
    if (std::is_same<T, char>::value || std::is_same<T, unsigned char>::value)
    {
        maxSteps = static_cast<int>((max_range - min_range) / 2); // Безопасный диапазон шагов для char/uchar
        maxSteps = std::clamp(maxSteps, 50, 300);                // Ограничение в пределах 50–300
    }

    std::uniform_int_distribution<int> stepsDist(50, maxSteps);
    int steps = stepsDist(gen);

    T myIncrement = (max_range - m_start) / steps;
    if (myIncrement == 0)
        myIncrement = 1;

    T myEnd = m_start + (myIncrement * steps);

    auto task = std::make_shared<NumericTask<T>>(m_start, myEnd, myIncrement, this);

    addTask(std::move(task));
}


