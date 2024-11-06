#ifndef ISERIALIZABLETASK_H
#define ISERIALIZABLETASK_H
#include <QJsonObject>

class ISerializableTask
{
public:
    virtual ~ISerializableTask() = default;
    virtual QJsonObject serialize() const = 0;
};

#endif // ISERIALIZABLETASK_H
