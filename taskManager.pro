QT += core gui quick concurrent
CONFIG += c++17

SOURCES += \
    #addDelThread.cpp \
    dispatchThread.cpp \
    main.cpp \
    iTask.cpp \
    taskManager.cpp \
    worker.cpp \
    tasksModel.cpp \
    workersModel.cpp

HEADERS += \
    #addDelThread.h \
    dispatchThread.h \
    iSerializableTask.h \
    #TaskFactory.h \
    taskManager.h \
    iTask.h \
    numericTask.h \
    worker.h \
    tasksModel.h \
    workersModel.h

RESOURCES += qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
