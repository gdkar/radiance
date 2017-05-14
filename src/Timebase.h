#pragma once

#include <QElapsedTimer>
#include <QMutex>
#include <QtGlobal>
#include <QMetaType>
#include <QMetaEnum>
#include <QMetaObject>

class Timebase {

    Q_GADGET
public:
    static constexpr double S_PER_MINUTE = 60;
    static constexpr double MS_PER_MINUTE =  S_PER_MINUTE * 1000;
    static constexpr double US_PER_MINUTE = MS_PER_MINUTE * 1000;
    static constexpr double NS_PER_MINUTE = US_PER_MINUTE * 1000;

    enum TimeSource {
        TimeSourceAudio,
        TimeSourceUser,
        TimeSourceClock,
        //TimeSourceNetwork,
    };
    Q_ENUM(TimeSource);
    enum TimeSourceEvent {
        TimeSourceEventNone,
        TimeSourceEventBeat,
        TimeSourceEventBar,
        TimeSourceEventBPM,
    };
    Q_ENUM(TimeSourceEvent);

    Timebase();
    void update(enum TimeSource source, enum TimeSourceEvent, double eventArg);
    double beat() const;
    double wallTime() const;
    int beatIndex() const;

protected:
    long m_wall_ns;
    double m_beatFrac;
    int m_beatIndex;
    double m_bpm;
    QElapsedTimer m_timer;
    mutable QMutex m_timeLock;
};
