#ifndef NUMERICTASK_H
#define NUMERICTASK_H

#include <string>
// #include <iostream>

#include <QThread>
#include <QTimer>
#include <QVariant>

#include "iTask.h"
#include "iSerializableTask.h"

template <typename T>
class NumericTask : public ITask, public ISerializableTask
{
public:
    NumericTask(T start, T end, T increment, QObject *parent = nullptr)
        : ITask(parent), m_start(start), m_end(end), m_progress(start), m_increment(increment)
    {
        changeState(TaskState::Wait);
        if (m_end == m_start)
            changeState(TaskState::Complete);

    }

    void take(int workerId) override
    {
        this->workerId = workerId;
        changeState(TaskState::Active);
    }

    void executeStep() override
    {
        m_progress += m_increment;
        QThread::msleep(50);

        if (m_progress >= m_end) {
            m_progress = m_end;
            closeTask();
        }
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
        if (m_end == m_start)
            return 100;

        return std::min(100 ,static_cast<int>((static_cast<double>(m_progress - m_start) / (m_end - m_start) * 100)));
    }

    std::string getStatus() const override
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

    QJsonObject serialize() const override
    {
        QJsonObject taskData;
        // taskData["taskId"] = taskId;
        taskData["type"] = QString::fromStdString(getType());
        taskData["start"] = QVariant::fromValue(m_progress).toString();
        taskData["end"] = QVariant::fromValue(m_end).toString();
        taskData["increment"] = QVariant::fromValue(m_increment).toString();
        // taskData["status"] = QString::fromStdString(status);
        return taskData;
    }

private:
    void changeState(int state)
    {
        if (state == TaskState::Complete)   status = "Завершена";
        else if (state == TaskState::Active)status = "Выполняет исполнитель №" + getWorkerId();
        else if (state == TaskState::Wait)  status = "Ожидает";
        emit statusChanged();
    }

    void closeTask() override
    {
        changeState(TaskState::Complete);
        emit taskFinished(taskId);
    }

    T m_start;
    T m_end;
    T m_progress;
    T m_increment;
    std::string status;
};

#endif // NUMERICTASK_H
