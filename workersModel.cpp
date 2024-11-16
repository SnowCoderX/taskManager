#include "workersModel.h"

//public:

WorkersModel::WorkersModel(QObject* parent)
    : QAbstractListModel(parent)
{

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

std::shared_ptr<Worker> WorkersModel::getFreeWorker()
{
    for (const auto &worker : workers)
        if (!worker->isRun())
            return worker;

    return nullptr;
}

std::shared_ptr<Worker> WorkersModel::searchWorkerByTaskId(int taskId)
{
    for (const auto& worker : workers)
        if (worker->getTaskId() == taskId)
            return worker;

    return nullptr;
}

int WorkersModel::countWorkersByStatus(const QString &status) const
{
    return std::count_if(workers.begin(), workers.end(), [&](const auto& worker) {
        return worker->getStatus().contains(status);
    });
}

void WorkersModel::stopAllWorkers()
{
    for (auto& worker : workers)
        if (worker)
            worker->stop();
}

int WorkersModel::getTotalWorkers() const
{
    return workers.size();
}

int WorkersModel::getWaitingWorkers() const
{
    return countWorkersByStatus("Ожидает");
}

int WorkersModel::getBusyWorkers() const
{
    return countWorkersByStatus("Выполняет задачу №");
}

void WorkersModel::addWorkers(short count)
{
    if (workers.size() >= 1000)
        return;

    for (int i = 0; i < count; ++i) {
        auto worker = std::make_shared<Worker>();
        worker->start();

        connect(worker.get(), &Worker::changeStatus, this, [this](int workerId) {
            updateWorker(workerId);
            emit workersChanged();
        });

        beginInsertRows(QModelIndex(), workers.size(), workers.size());
        workers.push_back(std::move(worker));
        emit workersChanged();
        endInsertRows();
    }
}

void WorkersModel::deleteWorker(int workerId)
{
    auto it = std::find_if(workers.begin(), workers.end(), [workerId](const auto& worker) {
        return worker->getId() == workerId; });

    if (it != workers.end()) {
        int index = std::distance(workers.begin(), it);
        if (it->get()->isRunning()) {
            it->get()->stop();
            it->get()->quit();
            it->get()->wait();
        }

        beginRemoveRows(QModelIndex(), index, index);
        workers.erase(it);
        endRemoveRows();

        emit  workersChanged();
    }
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
    case IdRole:        return worker->getId();     break;
    case StatusRole:    return worker->getStatus(); break;
    }
}

QHash<int, QByteArray> WorkersModel::roleNames() const
{
    return {
        { IdRole, "workerId" },
        { StatusRole, "workerStatus" }
    };
}
