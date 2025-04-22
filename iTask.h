#ifndef ITASK_H
#define ITASK_H

#include <atomic>

#include <QObject>

enum TaskState
{
    Complete,
    Active,
    Wait
};

enum TaskType
{
    NumericRandom
};

class ITask : public QObject
{
    Q_OBJECT

public:
    explicit ITask(QObject *parent = nullptr) : QObject(parent), taskId(++countTasks) {}
    virtual ~ITask() = default;

    virtual void take(int workerId) = 0;
    virtual void executeStep() = 0;
    virtual void deleteTask() = 0;
    virtual bool isCompleted() const = 0;
    virtual int getProgress() const = 0;
    int getId() const;
    virtual std::string getWorkerId();
    virtual QString getStatus() const = 0;
    virtual std::string getType() const = 0;
    virtual void changeState(int state) = 0;

signals:
    void progressUpdated(int taskId);
    void taskFinished(int taskId);
    void taskDelete(int taskId);
    void statusChanged();

protected:
    virtual void closeTask() = 0;
    static std::atomic<int> countTasks;
    int taskId;
    int workerId = 0;
};

#endif // ITASK_H
