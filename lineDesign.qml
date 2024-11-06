import QtQuick 2.9

Rectangle {
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
