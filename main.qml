import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import QtQuick.Controls.Styles 1.4

Window {
    visible: true
    title: "Многопоточность в Qt"
    minimumWidth: 800
    minimumHeight: 600

    // Загружается из C++
    property bool isDarkTheme: flagDarkTheme
    property int overallProgress: startValueOverallProgress

    property color backgroundColor: isDarkTheme ? "#2d2d2d" : "#F7F9FC"
    property color textColor: isDarkTheme ? "#e0e0e0" : "#333333"
    property color buttonColor: isDarkTheme ? "#3e3e3e" : "#E0E7FF"
    property color progressBarColor: isDarkTheme ? "#7fbc41" : "#4A90E2"

    property int itemSize: 55

    Rectangle {
        anchors.fill: parent
        color: backgroundColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            ProgressBar {
                id: overallProgressBar
                Layout.fillWidth: true
                minimumValue: 0
                maximumValue: 100
                value: overallProgress
                Behavior on value {
                        NumberAnimation {
                            duration: 1700
                        }
                    }

                style: ProgressBarStyle {
                    progress: Rectangle {
                        color: "#47c779"
                        radius: 4
                    }
                }
              }

            Text {
                id: overallProgressBarText
                text: "Общий прогресс выполнения: " + overallProgressBar.value.toFixed(2) + "%"
                anchors.top: overallProgressBar.bottom
                Layout.alignment: Qt.AlignHCenter
                color: textColor
            }

            RowLayout {
                height: 23
                spacing: 10
                anchors.top: overallProgressBarText.bottom

                // Левый список задач с кнопками
                ColumnLayout {
                    // Layout.minimumWidth: 100
                    spacing: 5

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 370
                        // GradientRectangle { anchors.fill: parent}
                        Text {
                            text: "Задачи"
                            color: textColor
                            Layout.alignment: Qt.AlignLeft
                        }
                        Item { Layout.fillWidth: true } //spacer
                        Button {
                            text: "+1"
                            onClicked: tasksModel.addNumericTask(1)
                            Layout.preferredWidth: 40
                            Layout.alignment: Qt.AlignRight
                            style: ButtonStyle {
                                    background: Rectangle {
                                        implicitWidth: 100
                                        implicitHeight: 25
                                        border.width: 2
                                        border.color: "#ffaaaa"
                                        radius: 4
                                        gradient: Gradient {
                                            GradientStop { position: 0 ; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
                                            GradientStop { position: 1 ; color: control.pressed ? "#dcc" : "#ccc" }
                                        }
                                    }
                                }
                        }
                        Button {
                            text: "+10"
                            onClicked: tasksModel.addNumericTask(10)
                            Layout.preferredWidth: 40
                            Layout.alignment: Qt.AlignRight
                            style: ButtonStyle {
                                    background: Rectangle {
                                        implicitWidth: 100
                                        implicitHeight: 25
                                        border.width: 2
                                        border.color: "#ffaaaa"
                                        radius: 4
                                        gradient: Gradient {
                                            GradientStop { position: 0 ; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
                                            GradientStop { position: 1 ; color: control.pressed ? "#dcc" : "#ccc" }
                                        }
                                    }
                                }
                        }
                        Button {
                            text: "+50"
                            onClicked: tasksModel.addNumericTask(50)
                            Layout.preferredWidth: 40
                            Layout.alignment: Qt.AlignRight
                            style: ButtonStyle {
                                    background: Rectangle {
                                        implicitWidth: 100
                                        implicitHeight: 25
                                        border.width: 2
                                        border.color: "#ffaaaa"
                                        radius: 4
                                        gradient: Gradient {
                                            GradientStop { position: 0 ; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
                                            GradientStop { position: 1 ; color: control.pressed ? "#dcc" : "#ccc" }
                                        }
                                    }
                                }
                        }
                        Button {
                            text: "+200"
                            onClicked: tasksModel.addNumericTask(200)
                            Layout.preferredWidth: 40
                            Layout.alignment: Qt.AlignRight
                            style: ButtonStyle {
                                    background: Rectangle {
                                        implicitWidth: 100
                                        implicitHeight: 25
                                        border.width: 2
                                        border.color: "#ffaaaa"
                                        radius: 4
                                        gradient: Gradient {
                                            GradientStop { position: 0 ; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
                                            GradientStop { position: 1 ; color: control.pressed ? "#dcc" : "#ccc" }
                                        }
                                    }
                                }
                        }
                    }

                    ListView {
                        id: tasksListView
                        clip: true
                        spacing: 10
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: tasksModel
                        delegate: ColumnLayout {
                            height: itemSize
                            width: 380
                            anchors.right: parent.right
                            anchors.left: parent.left
                            RowLayout {
                                ColumnLayout{
                                    Item {
                                        Layout.fillWidth: true
                                        Layout.minimumHeight: 2
                                        Layout.alignment: Qt.AlignBottom
                                        Rectangle { anchors.fill: parent; color: "#ffaaaa" }
                                        }
                                    RowLayout{
                                        // rectangleGradient { width: parent.width}
                                        Text { text: "Задача №" + taskId; color: textColor }
                                        Item { Layout.fillWidth: true } //spacer
                                        ProgressBar {
                                            id: itemTask
                                            value: progress
                                            minimumValue: 0
                                            maximumValue: 100
                                            Behavior on value {
                                                  NumberAnimation {
                                                    duration: (progress >= itemTask.value) ? 1600 : 0
                                                    //TODO если вернул сортировку, то этот вариант лучше восстановить
                                                    //duration: (value >= itemTask.value) ? 1600 : 0
                                                  }
                                              }
                                            style: ProgressBarStyle {
                                                progress: Rectangle {
                                                    color: "#47c779"
                                                    radius: 4
                                                }
                                            }
                                        }
                                        Button {
                                            text: "X"
                                            onClicked: tasksModel.deleteTask(taskId)
                                            Layout.preferredWidth: 25
                                            Layout.alignment: Qt.AlignRight
                                            style: ButtonStyle {
                                                    background: Rectangle {
                                                        implicitWidth: 100
                                                        border.width: 2
                                                        border.color: "#ffaaaa"
                                                        radius: 4
                                                        gradient: Gradient {
                                                            GradientStop { position: 0 ; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
                                                            GradientStop { position: 1 ; color: control.pressed ? "#e44545" : "#e44545" }
                                                        }
                                                    }
                                                }
                                        }
                                    }

                                    RowLayout {
                                        Text { text: "Статус: " + status; color: textColor; }
                                        Item { Layout.fillWidth: true } //spacer
                                        Text { text: "Тип: " + type; color: textColor; }
                                        }
                                }
                            }
                        }
                    }
                }

                // Центральное пространство
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: 2
                    Rectangle { anchors.fill: parent; color: "#ffaaaa" }
                    }

                // Правый список исполнителей с кнопками
                ColumnLayout {
                    Layout.minimumWidth: 380
                    spacing: 5

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 380
                        // GradientRectangle { anchors.fill: parent}
                        Text {
                            text: "Исполнители (рек. знач: " + taskManager.recommendedCountWorkers() + ")"
                            color: textColor
                            Layout.alignment: Qt.AlignLeft
                        }
                        Item { Layout.fillWidth: true } //spacer
                        Button {
                            text: "+1"
                            onClicked: workersModel.addWorkers(1)
                            Layout.preferredWidth: 40
                            Layout.alignment: Qt.AlignRight
                            style: ButtonStyle {
                                    background: Rectangle {
                                        implicitWidth: 100
                                        implicitHeight: 25
                                        border.width: 2
                                        border.color: "#ffaaaa"
                                        radius: 4
                                        gradient: Gradient {
                                            GradientStop { position: 0 ; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
                                            GradientStop { position: 1 ; color: control.pressed ? "#dcc" : "#ccc" }
                                        }
                                    }
                                }
                        }
                        Button {
                            text: "+10"
                            onClicked: taskManager.addWorkers(10)
                            Layout.preferredWidth: 40
                            Layout.alignment: Qt.AlignRight
                            style: ButtonStyle {
                                    background: Rectangle {
                                        implicitWidth: 100
                                        implicitHeight: 25
                                        border.width: 2
                                        border.color: "#ffaaaa"
                                        radius: 4
                                        gradient: Gradient {
                                            GradientStop { position: 0 ; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
                                            GradientStop { position: 1 ; color: control.pressed ? "#dcc" : "#ccc" }
                                        }
                                    }
                                }
                        }
                        Button {
                            text: "+50"
                            onClicked: taskManager.addWorkers(50)
                            Layout.preferredWidth: 40
                            Layout.alignment: Qt.AlignRight
                            style: ButtonStyle {
                                    background: Rectangle {
                                        implicitWidth: 100
                                        implicitHeight: 25
                                        border.width: 2
                                        border.color: "#ffaaaa"
                                        radius: 4
                                        gradient: Gradient {
                                            GradientStop { position: 0 ; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
                                            GradientStop { position: 1 ; color: control.pressed ? "#dcc" : "#ccc" }
                                        }
                                    }
                                }
                        }
                        Button {
                            text: "+200"
                            onClicked: taskManager.addWorkers(200)
                            Layout.preferredWidth: 40
                            Layout.alignment: Qt.AlignRight
                            style: ButtonStyle {
                                    background: Rectangle {
                                        implicitWidth: 100
                                        implicitHeight: 25
                                        border.width: 2
                                        border.color: "#ffaaaa"
                                        radius: 4
                                        gradient: Gradient {
                                            GradientStop { position: 0 ; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
                                            GradientStop { position: 1 ; color: control.pressed ? "#dcc" : "#ccc" }
                                        }
                                    }
                                }
                        }
                    }

                    ListView {
                        id: workersListView
                        clip: true
                        spacing: 10
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: workersModel
                        delegate: ColumnLayout {
                            height: itemSize
                            width: parent.width
                            Layout.fillWidth: true
                            Item {
                                Layout.fillWidth: true
                                Layout.minimumHeight: 2
                                Layout.alignment: Qt.AlignBottom
                                Rectangle { anchors.fill: parent; color: "#ffaaaa" }
                                }
                            RowLayout {
                                Layout.fillWidth: true
                                // rectangleGradient { width: parent.width}
                                Text { text: "Исполнитель №" + workerId; color: textColor }
                                Text { text: workerStatus; color: textColor }
                            }

                        }
                    }
                }
            }

            // Нижняя плашка
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10
                Item {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 2
                    Layout.alignment: Qt.AlignBottom
                    Rectangle { anchors.fill: parent; color: "#ffaaaa" }
                }

                RowLayout {
                    // Левая колонка с информацией о работниках
                    ColumnLayout {
                        Layout.alignment: Qt.AlignTop
                        spacing: 5

                        RowLayout {
                            Text {text: "Задачи    ";color: textColor}
                        }
                        Text { text: "Всего: " + tasksModel.totalTasks; color: textColor }
                        Text { text: "Ожидают: " + tasksModel.waitingTasks; color: textColor }
                        Text { text: "В работе: " + tasksModel.inProgressTasks; color: textColor }
                    }

                    Item { Layout.fillWidth: true } // Spacer

                    // Центральная колонка с переключателем темы
                    ColumnLayout {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter

                        Text {
                            text: "Темная тема"
                            color: textColor
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                        }

                        Switch {
                            id: themeSwitch
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                            checked: isDarkTheme
                            onCheckedChanged: {
                                isDarkTheme = checked
                            }
                        }
                    }

                    Item { Layout.fillWidth: true } // Spacer

                    // Правая колонка с информацией об исполнителях
                    ColumnLayout {
                        Layout.alignment: Qt.AlignTop
                        spacing: 5

                        RowLayout {
                            Text {text: "Исполнители    "; color: textColor}
                        }
                        Text { text: "Всего: " + workersModel.totalWorkers; color: textColor }
                        Text { text: "Ожидают: " + workersModel.waitingWorkers; color: textColor }
                        Text { text: "В работе: " + workersModel.busyWorkers; color: textColor }
                    }

                }
            }
        }
    }
}
