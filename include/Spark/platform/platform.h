#pragma once
#include "Spark/defines.h"

typedef struct platform_state {
    void* internal_state;
} platform_state_t;

b8 platform_init(
        platform_state_t* state,
        const char* application_name,
        u32 x,
        u32 y,
        u32 width,
        u32 height);

void platform_shutdown(platform_state_t* state);
b8 platform_pump_messages(platform_state_t* state);

void*   platform_allocate(u64 size, b8 aligned);
void    platform_free(const void* block, b8 aligned);
void*   platform_zero_memory(void* block, u64 size);
void*   platform_copy_memory(void* dest, const void* source, u64 size);
void*   platform_set_memory(void* dest, s32 value, u64 size);

void platform_console_write(const char* message, u8 color);
void platform_console_write_error(const char* message, u8 color);

f64 platform_get_absolute_time();
void platform_sleep(u64 ms);
