import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

ButtonStyle {
    background: Rectangle {
        property int borderWidth: 2

        implicitWidth: 100
        implicitHeight: 25
        border.width: borderWidth
        border.color: "#ffaaaa"
        radius: 4
        gradient: Gradient {
            GradientStop { position: 0; color: control.pressed ? "#e8ccfb" : "#f8ccfb" }
            GradientStop { position: 1; color: control.pressed ? "#dcc" : "#ccc" }
        }
    }
}
