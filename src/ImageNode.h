#pragma once

#include "VideoNode.h"
#include "OpenGLWorker.h"
#include <QOpenGLTexture>
#include <QMutex>
#include <QTimer>
#include <vector>

class ImageNodeOpenGLWorker;

// This class extends VideoNode to provide a static image or GIF
class ImageNode
    : public VideoNode {
    Q_OBJECT
    Q_PROPERTY(QString file READ file WRITE setFile NOTIFY fileChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(double frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged);

    friend class ImageNodeOpenGLWorker;

public:
    ImageNode(Context *context);
    void init(QString file);

    QJsonObject serialize() override;

    GLuint paint(QSharedPointer<Chain> chain, QVector<GLuint> inputTextures) override;

    // These static methods are required for VideoNode creation
    // through the registry

    // A string representation of this VideoNode type
    static QString typeName();

    // Create a VideoNode from a JSON description of one
    // Returns nullptr if the description is invalid
    static VideoNodeSP *deserialize(Context *context, QJsonObject obj);

    // Return true if a VideoNode could be created from
    // the given filename
    // This check should be very quick.
    static bool canCreateFromFile(QString filename);

    // Create a VideoNode from a filename
    // Returns nullptr if a VideoNode cannot be create from the given filename
    static VideoNodeSP *fromFile(Context *context, QString filename);

    // Returns QML filenames that can be loaded
    // to instantiate custom instances of this VideoNode
    static QMap<QString, QString> customInstantiators();

public slots:
    QString file();
    QString name();
    void setFile(QString file);
    double frequency();
    void setFrequency(double frequency);
signals:
    void fileChanged(QString file);
    void nameChanged(QString name);
    void frequencyChanged(double frequency);

private:
    QString fileToName();

protected:
    QString m_file;

    // This is not actually shared,
    // I just like the deletion semantics
    QSharedPointer<ImageNodeOpenGLWorker> m_openGLWorker{};

    bool m_ready{false};

    int          m_totalDelay{};
    QVector<int> m_frameDelays{}; // milliseconds
    double       m_frequency{};
    QVector<QSharedPointer<QOpenGLTexture>> m_frameTextures{};
};

typedef QmlSharedPointer<ImageNode, VideoNodeSP> ImageNodeSP;
Q_DECLARE_METATYPE(ImageNodeSP*)

///////////////////////////////////////////////////////////////////////////////

// This class extends OpenGLWorker
// to enable image loading
// and texture generation
// in a background context
class ImageNodeOpenGLWorker : public OpenGLWorker {
    Q_OBJECT

public:
    ImageNodeOpenGLWorker(QSharedPointer<ImageNode> p);

public slots:
    void initialize(QString filename);

signals:
    void message(QString str);
    void warning(QString str);
    void error(QString str);
protected:
    QWeakPointer<ImageNode> m_p;
};
