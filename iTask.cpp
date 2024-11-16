#include <iTask.h>

std::atomic<int> ITask::countTasks{0};

int ITask::getId() const
{
    //так как это интерфейсный класс, то надстройки над ним при уничтожении сначала уничтожат внутренний ITask,
    //а потом уже себя, но в промежутке между этими действиями может быть вызван метод getId, который вызовет
    //крашн в таком случае, поэтому здесь проверка на существование this
    if(!this)
        return 0;

    return taskId;
}

std::string ITask::getWorkerId()
{
    return workerId == 0 ? "Ошибка" : std::to_string(workerId);
}
