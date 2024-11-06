// #include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QQmlContext>
// #include <QTimer>

#include "taskManager.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    QSettings settings("OS", "Task_Manager");
    int width = settings.value("window/width", 800).toInt();
    int height = settings.value("window/height", 850).toInt();
    int posX = settings.value("window/posX", 930).toInt();
    int posY = settings.value("window/posY", 350).toInt();
    bool flagDarkTheme = settings.value("theme/darkMode", true).toBool();
    QCoreApplication::instance()->thread()->setPriority(QThread::HighPriority);

    TaskManager* taskManager = new TaskManager();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("tasksModel", taskManager->getTasksModel());
    engine.rootContext()->setContextProperty("workersModel", taskManager->getWorkersModel());
    engine.rootContext()->setContextProperty("taskManager", taskManager);
    engine.rootContext()->setContextProperty("flagDarkTheme", flagDarkTheme);
    engine.rootContext()->setContextProperty("startValueOverallProgress", taskManager->getTasksModel()->getOverallProgress());

    QObject::connect(taskManager->getTasksModel(), &TasksModel::progressChanged, &engine, [&](int overallProgress) {
        engine.rootObjects().first()->setProperty("overallProgress", overallProgress);
    });

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    QObject *rootObject = engine.rootObjects().first();
    rootObject->setProperty("width", width);
    rootObject->setProperty("height", height);
    rootObject->setProperty("x", posX);
    rootObject->setProperty("y", posY);

    taskManager->loadTasks();
    taskManager->loadWorkers();

    QTimer autoSaveTimer;
    QObject::connect(&autoSaveTimer, &QTimer::timeout, [&]() {
        settings.setValue("window/width", rootObject->property("width"));
        settings.setValue("window/height", rootObject->property("height"));
        settings.setValue("window/posX", rootObject->property("x"));
        settings.setValue("window/posY", rootObject->property("y"));
        settings.setValue("theme/darkMode", rootObject->property("isDarkTheme").toBool());
        taskManager->safeTasks();
        taskManager->safeWorkers();
    });
    autoSaveTimer.start(5000); //TODO если будет еще время, то переделай на сохранение после завершения фукнций вставки тасок и воркеров

    // Сохраняем состояние окна и темного режима при выходе
    QObject::connect(&app, &QGuiApplication::aboutToQuit, [&]() {
        settings.setValue("window/width", rootObject->property("width"));
        settings.setValue("window/height", rootObject->property("height"));
        settings.setValue("window/posX", rootObject->property("x"));
        settings.setValue("window/posY", rootObject->property("y"));
        settings.setValue("theme/darkMode", rootObject->property("isDarkTheme").toBool());

        taskManager->safeTasks();
        taskManager->safeWorkers();
        taskManager->stopAllWorkers();
    });

    return app.exec();
}
