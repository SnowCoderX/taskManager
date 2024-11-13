#ifndef TASKFACTORY_H
#define TASKFACTORY_H
// TaskFactory.h
#include "NumericTask.h"
#include <QJsonObject>
#include <memory>

class TaskFactory {
public:
    static std::unique_ptr<ISerializableTask> createTask(const QJsonObject &taskObj) {
        QString type = taskObj["type"].toString();

        if (type == "int") {
            auto task = std::make_unique<NumericTask<int>>(int start, int end, int increment);
            task->deserialize(taskObj);
            return task;
        } else if (type == "uint") {
            auto task = std::make_unique<NumericTask<uint>>();
            task->deserialize(taskObj);
            return task;
        } else if (type == "short") {
            auto task = std::make_unique<NumericTask<short>>();
            task->deserialize(taskObj);
            return task;
        } else if (type == "ushort") {
            auto task = std::make_unique<NumericTask<ushort>>();
            task->deserialize(taskObj);
            return task;
        } else if (type == "char") {
            auto task = std::make_unique<NumericTask<char>>();
            task->deserialize(taskObj);
            return task;
        } else if (type == "uchar") {
            auto task = std::make_unique<NumericTask<uchar>>();
            task->deserialize(taskObj);
            return task;
        }

        // Неизвестный тип задачи
        qWarning("Unknown task type: %s", qUtf8Printable(type));
        return nullptr;
    }
};

#endif // TASKFACTORY_H
