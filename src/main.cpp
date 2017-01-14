#include <QGuiApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QThread>
#include "EffectUI.h"
#include "CrossFaderUI.h"
#include "RenderContext.h"
#include "RenderThread.h"
#include "Output.h"
#include "Lux.h"
#include "Audio.h"
#include "main.h"

RenderContext *renderContext = 0;
QSettings *settings = 0;
UISettings *uiSettings = 0;
Audio *audio = 0;

QObject *uiSettingsProvider(QQmlEngine *engine, QJSEngine *scriptEngine) {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return uiSettings;
}

QObject *audioProvider(QQmlEngine *engine, QJSEngine *scriptEngine) {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return audio;
}

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("Radiance");
    QCoreApplication::setOrganizationDomain("radiance.lighting");
    QCoreApplication::setApplicationName("Radiance");
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QGuiApplication app(argc, argv);
    //qRegisterMetaType<Effect*>("Effect*");

    settings = new QSettings();
    uiSettings = new UISettings();
    audio = new Audio();

    qmlRegisterUncreatableType<VideoNodeUI>("radiance", 1, 0, "VideoNode", "VideoNode is abstract and cannot be instantiated");
    qmlRegisterType<EffectUI>("radiance", 1, 0, "Effect");
    qmlRegisterType<CrossFaderUI>("radiance", 1, 0, "CrossFader");

    qmlRegisterType<LuxBus>("radiance", 1, 0, "LuxBus");
    qmlRegisterType<OutputManager>("radiance", 1, 0, "OutputManager");

    qmlRegisterSingletonType<UISettings>("radiance", 1, 0, "UISettings", uiSettingsProvider);
    qmlRegisterSingletonType<Audio>("radiance", 1, 0, "Audio", audioProvider);

    // Render thread
    RenderThread renderThread{};
    renderThread.start();


    QQmlApplicationEngine engine(QUrl("../resources/qml/application.qml"));
    if(engine.rootObjects().isEmpty()) {
        qFatal("Failed to load main QML application");
        return 1;
    }

    QObject *window = engine.rootObjects().first();
    QObject::connect(window, SIGNAL(frameSwapped()), &renderThread , SIGNAL(render()));

    return app.exec();
}
