#ifndef WORKERSMODEL_H
#define WORKERSMODEL_H

#include <vector>
#include <memory>

#include <QAbstractListModel>

#include "worker.h"

class WorkersModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit WorkersModel(QObject* parent = nullptr);

    enum WorkerRoles
    {
        IdRole = Qt::UserRole + 1,
        StatusRole
    };

    void addWorker(std::unique_ptr<Worker> worker);
    void updateWorker(int workerId);
    Worker* getFreeWorker();
    std::vector<Worker*> getAllWorkers() const;
    Worker* searchWorkerByTaskId(int taskId);
    int countWorkersByStatus(const std::string& status) const;
    int countWorkersAll() const;

signals:
    void workersChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    std::vector<std::unique_ptr<Worker>> workers;
    mutable std::mutex mutexWorkers;
};

#endif // WORKERSMODEL_H
