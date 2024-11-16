#include "tasksModel.h"

#include <QThread>
#include <QTimer>

#include "taskGenerator.h"

Q_DECLARE_METATYPE(std::vector<std::shared_ptr<ITask>>)

//public:
TasksModel::TasksModel(QObject *parent) :
    QAbstractListModel(parent)
    , threadTaskModel(new QThread())
{
    qRegisterMetaType<std::vector<std::shared_ptr<ITask>>>("std::vector<std::shared_ptr<ITask>>");

    progressUpdateTimer = new QTimer();
    connect(progressUpdateTimer, &QTimer::timeout, this, [=]() {
        emit progressChanged(getOverallProgress());
    });
    progressUpdateTimer->start(500);

    TaskGenerator *generator = new TaskGenerator();
    generator->moveToThread(threadTaskModel.get());

    connect(this, &TasksModel::requestTaskGeneration, generator, &TaskGenerator::generateTasks);
    connect(generator, &TaskGenerator::tasksGenerated, this, &TasksModel::handleGeneratedTasks);
    connect(threadTaskModel.get(), &QThread::finished, generator, &QObject::deleteLater);

    threadTaskModel->start();
}

TasksModel::~TasksModel()
{
    if (threadTaskModel->isRunning())
    {
        threadTaskModel->quit();
        threadTaskModel->wait();
    }
    delete progressUpdateTimer;
}

void TasksModel::addTask(std::shared_ptr<ITask> task)
{
    int taskId = task->getId();
    connect(task.get(), &ITask::taskFinished, [this, taskId]    { updateTask(taskId); });
    connect(task.get(), &ITask::taskDelete, this, [this, taskId]{ deleteTask(taskId); });
    connect(task.get(), &ITask::progressUpdated, [this, taskId] { updateTask(taskId); });
    connect(task.get(), &ITask::statusChanged, [this, taskId]   { updateTask(taskId); });

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
        if (index(ind).isValid())
            emit dataChanged(index(ind), index(ind));
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
    if (tasks.size() + count >= 1000)
        emit requestTaskGeneration(1000 - tasks.size());
    else
        emit requestTaskGeneration(count);
}

void TasksModel::deleteTask(int taskId)
{
    auto it = std::find_if(tasks.begin(), tasks.end(), [taskId](const auto& task) {
        return task->getId() == taskId; });

    if (it != tasks.end()) {
        if (it->get()->getStatus() != "Ожидает" && it->get()->getStatus() != "Завершена")
            return;

        int index = std::distance(tasks.begin(), it);

        beginRemoveRows(QModelIndex(), index, index);
        tasks.erase(it);
        endRemoveRows();

        emit tasksChanged();
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
    case StatusRole:    return task->getStatus();                           break;
    case TypeRole:      return QString::fromStdString(task->getType());     break;
    }
    return QVariant();
}

void TasksModel::handleGeneratedTasks(const std::vector<std::shared_ptr<ITask>> &generatedTasks) {
    for (auto task : generatedTasks)
        addTask(task);
}

