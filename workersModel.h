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

    Q_INVOKABLE void addWorkers(short count);
    void updateWorker(int workerId);
    Worker* getFreeWorker();
    std::vector<Worker*> getAllWorkers() const;
    Worker* searchWorkerByTaskId(int taskId);
    int countWorkersByStatus(const QString& status) const;
    int countWorkersAll() const;

    Q_PROPERTY(int totalWorkers READ getTotalWorkers NOTIFY workersChanged)
    Q_PROPERTY(int waitingWorkers READ getWaitingWorkers NOTIFY workersChanged)
    Q_PROPERTY(int busyWorkers READ getBusyWorkers NOTIFY workersChanged)
    int getTotalWorkers() const;
    int getWaitingWorkers() const;
    int getBusyWorkers() const;

signals:
    void workersChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QThread* threadWorkerModel;
    std::vector<std::unique_ptr<Worker>> workers;
    mutable std::mutex mutexWorkers;
};

#endif // WORKERSMODEL_H
