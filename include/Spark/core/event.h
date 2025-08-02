#pragma once

#include "Spark/defines.h"

typedef struct {
    union {
        u64 u64[2];
        s64 s64[2];
        f64 f64[2];

        u32 u32[4];
        s32 s32[4];
        f32 f32[4];

        u16 u16[8];
        s16 s16[8];

        s8 s8[16];
        u8 u8[16];

        char c[128];
    } data;
} event_context_t;

typedef enum : u16 {
    EVENT_CODE_APPLICATION_QUIT = 1,
    EVENT_CODE_KEY_PRESSED = 2,
    EVENT_CODE_KEY_RELEASED = 3,
    EVENT_CODE_BUTTON_PRESSED = 4,
    EVENT_CODE_BUTTON_RELEASED = 5,
    EVENT_CODE_MOUSE_MOVED = 6,
    EVENT_CODE_MOUSE_WHEEL = 7,
    EVENT_CODE_RESIZED = 8,

    EVENT_CODE_DEBUG0 = 0x9,
    EVENT_CODE_DEBUG1 = 0xA,
    EVENT_CODE_DEBUG2 = 0xB,
    EVENT_CODE_DEBUG3 = 0xC,
    EVENT_CODE_DEBUG4 = 0xD,


    EVENT_CODE_MAX,
} system_event_code_t;

typedef b8 event_handled_t;
typedef b8 event_registered_t;
typedef b8 event_unregistered_t;
typedef event_handled_t (*PFN_on_event)(system_event_code_t code, void* sender, void* listener_inst, event_context_t context);

b8 event_initialize();
void event_shutdown();

SAPI event_registered_t     event_register(system_event_code_t code, void* listener, PFN_on_event on_event);
SAPI event_unregistered_t   event_unregister(system_event_code_t code, void* listener, PFN_on_event on_event);

SAPI event_handled_t event_fire(system_event_code_t code, void* sender, event_context_t context);

