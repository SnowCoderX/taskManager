import QtQuick 2.9

Rectangle {
    property color startColor: "#9d32ea"
    property color endColor: "#ffffff"
    property int borderRadius: 25
    property int borderWidth: 2
    radius: 2

    gradient: Gradient {
        GradientStop { position: 0.0; color: startColor }
        GradientStop { position: 0.35; color: endColor }
    }

    border.color: "black"
    border.width: borderWidth
}
