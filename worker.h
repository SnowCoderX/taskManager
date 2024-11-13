#ifndef WORKER_H
#define WORKER_H

#include <condition_variable>

#include <QThread>

#include "iTask.h"

class Worker : public QThread
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = nullptr);

    void assignTask(ITask* task);
    bool isRun() const;
    QString getStatus() const;
    int getId() const;
    int getTaskId() const;
    void stop();
    void stopTask();

signals:
    void taskFinished(int workerId);
    void changeStatus(int workerId);

protected:
    void run() override;
    int taskId = 0;

private:
    ITask* task = nullptr;
    int workerId;
    std::atomic<bool> running;
    QString status;
    static std::atomic<int> countWorkers;
    std::mutex taskMutex;
    std::condition_variable taskCondition;
};

#endif // WORKER_H
