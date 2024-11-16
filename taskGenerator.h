#ifndef TASKGENERATOR_H
#define TASKGENERATOR_H
#include <vector>
#include <memory>
#include <random>

#include <QObject>

#include "iTask.h"
#include "numericTask.h"

class TaskGenerator : public QObject {
    Q_OBJECT
public:
    explicit TaskGenerator(QObject *parent = nullptr) : QObject(parent) {}

    template <typename T>
    static std::shared_ptr<ITask> generateRandomNumericTask(std::mt19937 &gen) {
        T min_range = std::numeric_limits<T>::lowest();
        T max_range = std::numeric_limits<T>::max();

        T m_start = min_range;
        std::uniform_int_distribution<int> stepsDist(25, 150);
        int steps = stepsDist(gen);

        T myIncrement = (max_range - m_start) / steps;
        if (myIncrement == 0)
            myIncrement = 1;

        T myEnd = m_start + (myIncrement * steps);

        return std::make_shared<NumericTask<T>>(m_start, myEnd, myIncrement);
    }

signals:
    void tasksGenerated(const std::vector<std::shared_ptr<ITask>> &tasks);

public slots:
    void generateTasks(short count) {
        std::vector<std::shared_ptr<ITask>> generatedTasks;

        std::random_device rd;
        std::mt19937 gen(rd());

        for (short i = 0; i < count; ++i) {
            if (generatedTasks.size() >= 1000)
                break;

            int typeChoice = std::uniform_int_distribution<>(0, 5)(gen);
            switch (typeChoice) {
            case 0: generatedTasks.push_back(generateRandomNumericTask<char>(gen));     break;
            case 1: generatedTasks.push_back(generateRandomNumericTask<uchar>(gen));    break;
            case 2: generatedTasks.push_back(generateRandomNumericTask<short>(gen));    break;
            case 3: generatedTasks.push_back(generateRandomNumericTask<ushort>(gen));   break;
            case 4: generatedTasks.push_back(generateRandomNumericTask<int>(gen));      break;
            case 5: generatedTasks.push_back(generateRandomNumericTask<uint>(gen));     break;
            default:                                                                    break;
            }
        }

        emit tasksGenerated(generatedTasks);
    }
};

#endif // TASKGENERATOR_H
