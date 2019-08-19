import QtQuick 2.7
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import radiance 1.0

VideoNodeTile {
    id: tile;

    onVideoNodeChanged: {
        videoNode.frequency = Qt.binding(function() { return frequencyCombo.value });
    }
    Connections {
        target: videoNode
        onFrequencyChanged: {
            frequencyCombo.value = frequency;
        }
    }
    ColumnLayout {
        anchors.fill: parent;
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.bottomMargin: 5
        anchors.topMargin: 5

        RadianceTileTitle {
            Layout.fillWidth: true;
            text: videoNode ? videoNode.name : "";
        }

        CheckerboardPreview {
            videoNode: tile.videoNode
        }
        RowLayout {
            FrequencyDropDown {
                id: frequencyCombo;
            }
            Label {
                text: attachedParameter >= 0 ? "#" + attachedParameter : "";
                color: RadianceStyle.tileTextColor;
            }
        }
    }

    Keys.onPressed: {

    }
}
