import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

ProgressBarStyle {
  background: Rectangle {
    radius: 2
    color: "lightgray"
    border.color: "gray"
    border.width: 1
    implicitWidth: 200
    implicitHeight: 24
  }
  progress: Rectangle {
    color: "lightsteelblue"
    border.color: "steelblue"
  }
}

