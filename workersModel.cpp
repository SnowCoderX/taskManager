#include "workersModel.h"

//public:

WorkersModel::WorkersModel(QObject* parent)
    : QAbstractListModel(parent)
{

}

void WorkersModel::addWorker(std::unique_ptr<Worker> worker)
{
    connect(worker.get(), &Worker::taskFinished, this, [this](int workerId) {
        updateWorker(workerId);
    });

    connect(worker.get(), &Worker::changeStatus, this, [this](int workerId) {
        updateWorker(workerId);
    });

    std::lock_guard<std::mutex> lock(mutexWorkers);
    beginInsertRows(QModelIndex(), workers.size(), workers.size());
    workers.push_back(std::move(worker));
    emit workersChanged();
    endInsertRows();
}

void WorkersModel::updateWorker(int workerId)
{
    for (int i = 0; i < workers.size(); ++i) {
        if (workers[i]->getId() == workerId) {
            emit dataChanged(index(i), index(i));
            break;
        }
    }
}

Worker* WorkersModel::getFreeWorker()
{
    std::lock_guard<std::mutex> lock(mutexWorkers);
    for (const auto &worker : workers)
        if (!worker->isRun())
            return worker.get();

    return nullptr;
}

std::vector<Worker*> WorkersModel::getAllWorkers() const
{
    std::lock_guard<std::mutex> lock(mutexWorkers);
    std::vector<Worker*> allWorkers;
    for (const auto& worker : workers)
        allWorkers.push_back(worker.get());

    return allWorkers;
}

Worker* WorkersModel::searchWorkerByTaskId(int taskId)
{
    std::lock_guard<std::mutex> lock(mutexWorkers);
    for (const auto& worker : workers)
        if (worker->getTaskId() == taskId)
            return worker.get();

    return nullptr;
}

int WorkersModel::countWorkersByStatus(const std::string &status) const
{
    std::lock_guard<std::mutex> lock(mutexWorkers);
    return std::count_if(workers.begin(), workers.end(), [&](const auto& worker) {
        return worker->getStatus().find(status) != std::string::npos;
    });
}

int WorkersModel::countWorkersAll() const
{
    return workers.size();
}

//protected:

int WorkersModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return workers.size();
}

QVariant WorkersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= workers.size())
        return QVariant();

    const auto& worker = workers[index.row()];
    switch (role) {
    case IdRole:        return worker->getId();                             break;
    case StatusRole:    return QString::fromStdString(worker->getStatus()); break;
    }
}

QHash<int, QByteArray> WorkersModel::roleNames() const
{
    return {
        { IdRole, "workerId" },
        { StatusRole, "workerStatus" }
    };
}
