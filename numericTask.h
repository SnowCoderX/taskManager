#ifndef NUMERICTASK_H
#define NUMERICTASK_H

#include <string>

#include <QThread>
#include <QTimer>
#include <QVariant>

#include "iTask.h"

template <typename T>
class NumericTask : public ITask
{
public:
    NumericTask(T start, T end, T increment, QObject *parent = nullptr)
        : ITask(parent), m_start(start), myEnd(end), myProgress(start), myIncrement(increment)
    {
        changeState(TaskState::Wait);
        if (myEnd == m_start)
            changeState(TaskState::Complete);

    }

    void take(int workerId) override
    {
        this->workerId = workerId;
        changeState(TaskState::Active);
    }

    void executeStep() override
    {
        if (myEnd - myIncrement > myProgress)
            myProgress += myIncrement;
        else {
            myProgress = myEnd;
            closeTask();
            return;    
        }

        if ((myEnd - myProgress) / myIncrement % 2 == 0)   //для уменьшения нагрузки на qml
            emit progressUpdated(taskId);
    }

    void deleteTask() override {
        emit taskDelete(taskId);
    }

    bool isCompleted() const override
    {
        if (status == "Завершена")  return true;
        else                        return false;
    }

    int getProgress() const override
    {
        if (myEnd == m_start)
            return 100;

        return std::min(100 ,static_cast<int>((static_cast<double>(myProgress - m_start) / (myEnd - m_start) * 100)));
    }

    QString getStatus() const override
    {
        return status;
    }

    std::string getType() const override
    {
        if (typeid(T) == typeid(char))      return "char";
        if (typeid(T) == typeid(uchar))     return "uchar";
        if (typeid(T) == typeid(short))     return "short";
        if (typeid(T) == typeid(ushort))    return "ushort";
        if (typeid(T) == typeid(int))       return "int";
        if (typeid(T) == typeid(uint))      return "uint";
        return "unknown";
    }

    void changeState(int state) override
    {
        switch (state) {
        case TaskState::Complete:   status = "Завершена";                                                       break;
        case TaskState::Active:     status = "Выполняет исполнитель №" + QString::fromStdString(getWorkerId()); break;
        case TaskState::Wait:       status = "Ожидает";                                                         break;
        default:                    status = "Ошибка";                                                          break;
        }
        emit statusChanged();
    }
private:
    void closeTask() override
    {
        changeState(TaskState::Complete);
        emit taskFinished(taskId);
    }

    T m_start;
    T myEnd;
    T myProgress;
    T myIncrement;
    QString status;
};

#endif // NUMERICTASK_H
