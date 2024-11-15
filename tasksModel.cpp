#include "tasksModel.h"

#include <iostream>

#include <QThread>
#include <QTimer>

#include "numericTask.h"
//public:

TasksModel::TasksModel(QObject *parent) : QAbstractListModel(parent)
{
    // threadTaskModel = new QThread();
    // moveToThread(threadTaskModel);
    // threadTaskModel->start();

    // Таймер для обновления прогресса
    QTimer *progressUpdateTimer = new QTimer();
    connect(progressUpdateTimer, &QTimer::timeout, this, [=]() {
        emit progressChanged(getOverallProgress());
    });
    progressUpdateTimer->start(500);  // Обновление каждые 500 мс
}

void TasksModel::addTask(std::unique_ptr<ITask> task)
{
    if (tasks.size() >= 1000)
        return;
    beginInsertRows(QModelIndex(), tasks.size(), tasks.size());
    tasks.push_back(std::move(task));
    // sortTasksByStatus();
//    emit progressChanged(getOverallProgress());
    endInsertRows();
    emit tasksChanged();
}

int TasksModel::getOverallProgress() const
{
    // return 0;   //TODO убираем чтобы сфокусироваться на главном
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
    for (int i = 0; i < tasks.size(); ++i) {
        // std::lock_guard<std::mutex> lock(mutexTasks);
        if (tasks[i] != nullptr)
            if (tasks[i]->getId() == taskId) {
                emit dataChanged(index(i), index(i));
                // emit progressChanged(getOverallProgress());
                emit tasksChanged();
                break;
        }
    }
}

ITask* TasksModel::getFreeTask()
{
    for (const auto& task : tasks){
        // std::lock_guard<std::mutex> lock(mutexTasks);
        if (task != nullptr)
            if (task->getStatus() == "Ожидает")
                return task.get();
    }

    return nullptr;
}

int TasksModel::getCountTasksByStatus(const QString &status) const
{
    return std::count_if(tasks.begin(), tasks.end(), [&](const auto& task) {
        return task->getStatus().contains(status);
    });
}
//TODO когда заносятся таски после добавления воркеров, то не запускается диспетчеризация автоматом
void TasksModel::addNumericTask(short count)
{
    //TODO перенос в модель
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

        // dispatchThread->getCondition().notify_one();
        // if (i % 100 == 0)
            emit tasksChanged();
    }
}

void TasksModel::deleteTask(int taskId)
{
    auto it = std::find_if(tasks.begin(), tasks.end(), [taskId](const auto& task) {
        return task->getId() == taskId; });

    if (it != tasks.end()) {
        int index = std::distance(tasks.begin(), it);

        // std::lock_guard<std::mutex> lock(mutexTasks);

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
    // Определяем минимальное и максимальное значения для типа T
    T min_range = std::numeric_limits<T>::lowest();
    T max_range = std::numeric_limits<T>::max();

    // Устанавливаем начальное значение как минимум диапазона
    T m_start = min_range;

    // Определяем максимальное количество шагов, чтобы избежать переполнения
    int maxSteps = 300;
    if (std::is_same<T, char>::value || std::is_same<T, unsigned char>::value)
    {
        maxSteps = static_cast<int>((max_range - min_range) / 2); // Безопасный диапазон шагов для char/uchar
        maxSteps = std::clamp(maxSteps, 50, 300);                // Ограничение в пределах 50–300
    }

    // Выбираем случайное количество шагов в пределах безопасного диапазона
    std::uniform_int_distribution<int> stepsDist(50, maxSteps);
    int steps = stepsDist(gen);

    // Вычисляем инкремент на основе диапазона и количества шагов
    T myIncrement = (max_range - m_start) / steps;
    if (myIncrement == 0)
        myIncrement = 1;

    // Вычисляем конечное значение
    T myEnd = m_start + (myIncrement * steps);

    // Создаем задачу
    auto task = std::make_unique<NumericTask<T>>(m_start, myEnd, myIncrement, this);
    int taskId = task->getId();

    // Подключаем сигналы задачи
    connect(task.get(), &ITask::taskFinished, [this, taskId]    { updateTask(taskId); });
    connect(task.get(), &ITask::taskDelete, this, [this, taskId]{ deleteTask(taskId); });
    connect(task.get(), &ITask::progressUpdated, [this, taskId] { updateTask(taskId); });

    // Добавляем задачу
    addTask(std::move(task));
}


