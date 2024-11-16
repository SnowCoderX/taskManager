#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QQmlContext>

#include "manager.h"

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

    QObject::connect(&app, &QGuiApplication::aboutToQuit, [&]() {
        settings.setValue("window/width", rootObject->property("width"));
        settings.setValue("window/height", rootObject->property("height"));
        settings.setValue("window/posX", rootObject->property("x"));
        settings.setValue("window/posY", rootObject->property("y"));
        settings.setValue("theme/darkMode", rootObject->property("isDarkTheme").toBool());

        manager.stopWork();
    });

    return app.exec();
}
