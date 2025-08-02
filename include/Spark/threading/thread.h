#pragma once

typedef enum : u8 {
    THREAD_STATE_INVALID,
    THREAD_STATE_RUNNING,
    THREAD_STATE_COMPLETE,
} thread_state_t;

typedef struct {
    u32 thread_id;
} thread_t;

b8 thread_start(void (*function(void* args)), void* args, thread_t* out_thread);
void thread_join(thread_t thread);
void thread_abort(thread_t thread);
