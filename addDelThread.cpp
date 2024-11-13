#include "addDelThread.h"
#include "worker.h"
#include "taskManager.h"

void AddDelThread::run() {
    std::unique_lock<std::mutex> lock(mutex);
    while (!flagCloseApp) {
        condition.wait(lock, [this] { return countAddTask > 0 || !vecTaskDelete.empty() || flagCloseApp; });

        if (flagCloseApp) break;

        while (countAddTask > 0) {
            auto task = createRandomTask<int>();
            if (task) {
                emit taskAdded(std::move(task));
                --countAddTask;
            } else
                break;
        }

        for (int taskId : vecTaskDelete) {
            // taskManager->removeTaskById(taskId);
            emit taskManager->tasksChanged();
        }
        vecTaskDelete.clear();
    }
}

template <typename T>
std::unique_ptr<ITask> AddDelThread::createRandomTask()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    int typeChoice = std::uniform_int_distribution<>(0, 5)(gen);

    T max_range = std::numeric_limits<T>::max();
    T min_range = std::numeric_limits<T>::lowest();

    std::uniform_int_distribution<T> startDist(min_range / 2, max_range / 2);
    T m_start = startDist(gen);

    std::uniform_int_distribution<T> endDist(m_start + 100, max_range);
    T myEnd = endDist(gen);

    std::uniform_int_distribution<T> incrementDist(1, std::max<T>((myEnd - m_start) / 600, 1));
    T myIncrement = incrementDist(gen);

    int steps = (myEnd - m_start) / myIncrement;
    while (steps < 100 || steps > 600) {
        m_start = startDist(gen);
        myEnd = endDist(gen);
        myIncrement = incrementDist(gen);
        steps = (myEnd - m_start) / myIncrement;
    }

    auto task = std::make_unique<NumericTask<T>>(m_start, myEnd, myIncrement, this);
    int taskId = task->getId();
    connect(task.get(), &ITask::taskFinished, [this]            {emit taskManager->tasksChanged();} );
    connect(task.get(), &ITask::taskFinished, [this, taskId]    {taskManager->tasksModel->updateTask(taskId);});
    connect(task.get(), &ITask::deleteTask, [this, taskId]      {taskManager->deleteTask(taskId);});
    connect(task.get(), &ITask::progressUpdated, [this, taskId] {taskManager->tasksModel->updateTask(taskId);});
    connect(task.get(), &ITask::statusChanged, [this]           {taskManager->tasksModel->sortTasksByStatus();});

    return task;
    // tasksModel->addTask(std::move(task));

}

// Методы для управления добавлением и удалением задач
void AddDelThread::setAddTasks(short count) {
    std::lock_guard<std::mutex> lock(mutex);
    countAddTask += count;
    condition.notify_one();
}

void AddDelThread::setDeleteTask(int taskId) {
    std::lock_guard<std::mutex> lock(mutex);
    vecTaskDelete.push_back(taskId);
    condition.notify_one();
}

void AddDelThread::setAddWorkers(short count) {
    std::lock_guard<std::mutex> lock(mutex);
    countAddWorkers += count;
    condition.notify_one();
}

void AddDelThread::setDeleteWorker(int workerId) {
    std::lock_guard<std::mutex> lock(mutex);
    vecWorkerDelete.push_back(workerId);
    condition.notify_one();
}

// void AddDelThread::setAddTasks(short count)
// {
//     std::lock_guard<std::mutex> lock(mutex);
//     countAddTask = count;
//     condition.notify_one();
// }

// void AddDelThread::setDeleteTask(int taskId) {
//     std::lock_guard<std::mutex> lock(mutex);
//     vecTaskDelete.push_back(taskId);
//     condition.notify_one();
// }


