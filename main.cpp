#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QQmlContext>

#include "manager.h"

void printHelp() {
    std::cout << "Доступные параметры:\n";
    std::cout << "-t <taskCount>     : Добавить указанное количество задач.\n";
    std::cout << "-w <workersCount>  : Добавить указанное количество работников.\n";
    std::cout << "-rt <0|1>          : Включить/выключить восстановление задач (0 - отключить, 1 - включить).\n";
    std::cout << "-rw <0|1>          : Включить/выключить восстановление работников (0 - отключить, 1 - включить).\n";
    std::cout << "-h, -help          : Показать это сообщение.\n";
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    //// ↓↓↓ справка ↓↓↓ ////
    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];

        if (arg == "-h" || arg == "-help" || arg == "--h" || arg == "--help") {
            printHelp();
            return 0;
        }
    }
    //// ↑↑↑ справка ↑↑↑ ////

    QGuiApplication app(argc, argv);

    QSettings settings("OS", "Task_Manager");
    int width = settings.value("window/width", 800).toInt();
    int height = settings.value("window/height", 850).toInt();
    int posX = settings.value("window/posX", 930).toInt();
    int posY = settings.value("window/posY", 350).toInt();
    bool flagDarkTheme = settings.value("theme/darkMode", true).toBool();
    QCoreApplication::instance()->thread()->setPriority(QThread::HighPriority);

    Manager manager;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("tasksModel", manager.getTasksModel());
    engine.rootContext()->setContextProperty("workersModel", manager.getWorkersModel());
    engine.rootContext()->setContextProperty("manager", &manager);
    engine.rootContext()->setContextProperty("flagDarkTheme", flagDarkTheme);
    engine.rootContext()->setContextProperty("startValueOverallProgress", manager.getTasksModel()->getOverallProgress());

    QObject::connect(manager.getTasksModel(), &TasksModel::progressChanged, &engine, [&](int overallProgress) {
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

    //// ↓↓↓ аргументы запуска и восстановление ↓↓↓ ////
    bool flagRecoveryTasks = true;
    bool flagRecoveryWorkers = true;

    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];

        if (arg == "-h" || arg == "-help" || arg == "--h" || arg == "--help") {
            printHelp();
            return 0;
        }

        if (arg == "-t" && (i + 1) < argc) {
            bool ok;
            int taskCount = QString(argv[i + 1]).toInt(&ok);
            if (ok)
                manager.addTask(taskCount);
        }

        if (arg == "-w" && (i + 1) < argc) {
            bool ok;
            int workersCount = QString(argv[i + 1]).toInt(&ok);
            if (ok)
                manager.addWorkers(workersCount);
        }

        if (arg == "-rt" && (i + 1) < argc) {
            bool ok;
            int recovery = QString(argv[i + 1]).toInt(&ok);
            if (ok)
                if (!recovery)
                    flagRecoveryTasks = false;
                else
                    std::cout << "Ошибка: Неверный формат аргумента -rt\n";

        }

        if (arg == "-rw" && (i + 1) < argc) {
            bool ok;
            int recovery = QString(argv[i + 1]).toInt(&ok);
            if (ok)
                if (!recovery)
                    flagRecoveryWorkers = false;
                else
                    std::cout << "Ошибка: Неверный формат аргумента -rw\n";
        }
    }
    //// ↑↑↑ аргументы запуска и восстановление ↑↑↑ ////

    if (flagRecoveryTasks)
        manager.loadTasks();

    if (flagRecoveryWorkers)
        manager.loadWorkers();

    QObject::connect(&app, &QGuiApplication::aboutToQuit, [&]() {
        settings.setValue("window/width", rootObject->property("width"));
        settings.setValue("window/height", rootObject->property("height"));
        settings.setValue("window/posX", rootObject->property("x"));
        settings.setValue("window/posY", rootObject->property("y"));
        settings.setValue("theme/darkMode", rootObject->property("isDarkTheme").toBool());

        manager.safeTasks();
        manager.safeWorkers();
        manager.stopAllWorkers();
    });


    return app.exec();
}
