#include "Spark/core/event.h"
#include "Spark/core/logging.h"
#include "Spark/core/smemory.h"
#include "Spark/defines.h"

#include "Spark/containers/darray.h"
#include <stdlib.h>

// ============================
// Local types
// ============================
#define MAX_MESSAGE_CODES 1024

typedef struct registered_event {
    void* listener;
    PFN_on_event callback;
} registered_event_t;

darray_type(registered_event_t, registered_event);
darray_impl(registered_event_t, registered_event);

typedef struct {
    darray_registered_event_t registered_events[MAX_MESSAGE_CODES];
} event_system_state_t;


// ============================
// Static Valirables
// ============================
static b8 is_initialized = false;
static event_system_state_t state; 

// ============================
// Functions 
// ============================
b8 
event_initialize() {
    SASSERT(is_initialized == false, "Event system already initialized.");
    STRACE("Initializing event system");

    szero_memory(&state, sizeof(event_system_state_t));
    is_initialized = true;
    STRACE("Finished initializing event system");

    return true;
}

void 
event_shutdown() {
    STRACE("Shutting down event system");
    for (u32 i = 0; i < MAX_MESSAGE_CODES; i++) {
        if (state.registered_events[i].data != NULL) {
            darray_registered_event_destroy(&state.registered_events[i]);
        }
    }
}

event_registered_t 
event_register(system_event_code_t code, void* listener, PFN_on_event on_event) {
    SASSERT(is_initialized, "Cannot register event: event system not initialized.");

    // Initialize events if necessary
    if (state.registered_events[code].data == NULL) {
         darray_registered_event_create(1, &state.registered_events[code]);
    }

    // Duplicate check
    u64 registered_count = state.registered_events[code].count;
    for (u64 i = 0; i < registered_count; i++) {
        if (state.registered_events[code].data->listener == listener) {
            SWARN("Trying to register two identical listeners to event code %d.", code); 
            return false;
        }
    }

    // No duplicates, add event
    registered_event_t event = {
        .listener = listener,
        .callback = on_event,
    };

    darray_registered_event_push(&state.registered_events[code], event);
    STRACE("Registered event %d", code);

    return true;
}
event_unregistered_t 
event_unregister(system_event_code_t code, void* listener, PFN_on_event on_event) {
    STRACE("Unregistering event %d", code);
    SASSERT(is_initialized, "Cannot unregister event: event system not initialized.");
    SASSERT(state.registered_events[code].data != NULL, "Cannot unregister event: Event system does not have code %d initialized.", code);

    u64 registered_count = state.registered_events[code].count;
    for (u64 i = 0; i < registered_count; i++) {
        registered_event_t event = state.registered_events[code].data[i];
        if (event.listener == listener && event.callback == on_event) {
            STRACE("Registered_count: %d", registered_count);
            darray_registered_event_pop(&state.registered_events[code], i);
            STRACE("Unregistered event: Code: %d, index: %lu", code, i);
            return true;
        }
    }

    SERROR("Failed to unregister event. Code %d, listener: %p, event: %p", code, listener, on_event);
    return false;
}

event_handled_t 
event_fire(system_event_code_t code, void* sender, event_context_t context) {
    SASSERT(is_initialized, "Cannot unregister event: event system not initialized.");
    if (state.registered_events[code].data == NULL) {
        return false;
    }

    u64 registered_count = state.registered_events[code].count;
    for (u64 i = 0; i < registered_count; i++) {
        registered_event_t event = state.registered_events[code].data[i];
        if (event.callback(code, sender, event.listener, context)) {
            return true;
        }
    }

    return false;
}
