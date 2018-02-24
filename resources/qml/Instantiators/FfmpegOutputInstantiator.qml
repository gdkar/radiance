import QtQuick 2.3

QtObject {
    Component.onCompleted: {
        var vn = registry.deserialize(context, JSON.stringify({
            type: "FfmpegOutputNode"
        }));
        if (vn) {
            graph.insertVideoNode(vn);
        } else {
            console.log("Could not instantiate FfmpegOutputNode");
        }
    }
}
