#pragma once
#include "util/common.h"

#include <portmidi.h>

#include "midi/config.h"
#define SDL_MIDI_COMMAND_EVENT (SDL_USEREVENT)

enum midi_status {
    MIDI_STATUS_NOTEOFF = 0x80,
    MIDI_STATUS_NOTEON = 0x90,
    MIDI_STATUS_CC = 0xB0,
    MIDI_STATUS_AFTERTOUCH = 0xD0,
};
struct midi_controller {
    const char * name;
    const char * short_name;
    PmDeviceID device_id;
    PortMidiStream * stream;

    int enabled;
    midi_config::_controller_midi_config * config;
};
enum midi_event_type {
    MIDI_EVENT_SLIDER,
    MIDI_EVENT_KEY,
};

struct midi_event {
    midi_event_type type;
    int snap;
    struct midi_slider {
        int index;
        float value;
    };
    struct midi_key {
        char *keycode;
    };
    union {
        midi_slider slider;
        midi_key    key;
    };
};

//extern struct midi_controller * midi_controllers;
//extern int n_midi_controllers;

extern Uint32 midi_command_event;

void midi_start();
void midi_stop();
void midi_refresh();

PmError pm_errmsg(PmError err);
