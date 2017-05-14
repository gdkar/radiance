import QtQuick 2.8
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import radiance 1.0

FocusScope {
    id: tile;
    property alias crossfader: crossfader;
    width: 400;
    Layout.fillHeight: true;

    RadianceTile {
        anchors.fill: parent;
        focus: true;
        slider: slider;
    }

    MouseArea {
        anchors.fill: parent;
        onClicked: {
            tile.forceActiveFocus();
        }
    }

    ColumnLayout {
        anchors.fill: parent;
        anchors.margins: 15;

        Item {
            Layout.preferredHeight: width;
            Layout.fillWidth: true;
            layer.enabled: true;

            CheckerboardBackground {
                anchors.fill: parent;
            }

            CrossFader {
                id: crossfader;
                anchors.fill: parent;
                parameter: slider.value;
            }
        }

        Slider {
            id: slider;
            Layout.fillWidth: true;
            minimumValue: 0;
            maximumValue: 1;
        }
    }
}
