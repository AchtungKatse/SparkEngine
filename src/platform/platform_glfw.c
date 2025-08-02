#if SPKPLATFORM_GLFW
#include "Spark/core/input.h"
#include "Spark/platform/platform.h"
#include "Spark/defines.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <bits/time.h>
#include <string.h>
#include <time.h>

#if _POSIX_C_SOURCE >= 199309L
#elif
#include <unistd.h>
#endif


typedef enum : u8 {
    STATE_UNINITIALIZED = 0,
    STATE_INITIALIZED   = 1,
} glfw_state_t;

typedef struct internal_state {
    GLFWwindow* mWindow;
    u32 width;
    u32 height;
    glfw_state_t state;
} internal_state_t;

b8 
platform_init(
        platform_state_t* plat_state,
        const char* application_name,
        u32 x,
        u32 y,
        u32 width,
        u32 height) {
    plat_state->internal_state = malloc(sizeof(internal_state_t));
    internal_state_t* state = plat_state->internal_state;
    state->width = width;
    state->height = height;

    if (!(state->state & STATE_INITIALIZED)) {
        STRACE("Initializing glfw");
        state->state |= glfwInit();
        SASSERT(state->state & STATE_INITIALIZED, "GLFW failed to initialize.");
    }

    const u32 VersionMajor = 3;
    const u32 VersionMinor = 3;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, VersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, VersionMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, application_name, NULL, NULL);
    state->mWindow = window;

    STRACE("Created main window");
    SASSERT(window, "Failed to create window: %p", window);

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        SCRITICAL("Failed to load glad GL: {0}");
    }
    STRACE("Initialized OpenGL");
    return TRUE;
}

void 
platform_shutdown(platform_state_t* plat_state) {
    internal_state_t* state = plat_state->internal_state;
    glfwDestroyWindow(state->mWindow);
}
b8 
platform_pump_messages(platform_state_t* plat_state) {
    internal_state_t* state = plat_state->internal_state;
    glfwPollEvents();

    for (s32 i = 0; i < 0xff; i++) {
        s32 key_state = glfwGetKey(state->mWindow, i);
        b8 is_pressed = key_state != GLFW_RELEASE;
        input_process_key(i, is_pressed);
    }
    return TRUE;
}

void*   
platform_allocate(u64 size, b8 aligned) {
    return malloc(size);
}
void    
platform_free(void* block, b8 aligned) {
    free(block);
}
void*   
platform_zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}
void*   
platform_copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}
void*   
platform_set_memory(void* dest, s32 value, u64 size) {
    return memset(dest, value, size);
}

void 
platfrom_console_write(const char* message, u8 color) {
#ifdef SPARK_DEBUG
    const char* color_strings[] = {"0:42", "1:31", "1:33", "1:32", "1:34", "1:38"};
    SASSERT(color >= 0 && color <= sizeof(color_strings) / sizeof(const char*), "Trying to use invalid console color");
    SINFO("\033[%sm%s\033[0m", color_strings[color], message);
#endif
}
void 
platfrom_console_write_error(const char* message, u8 color) {
#ifdef SPARK_DEBUG
    const char* color_strings[] = {"0:42", "1:31", "1:33", "1:32", "1:34", "1:38"};
    SASSERT(color >= 0 && color <= sizeof(color_strings) / sizeof(const char*), "Trying to use invalid console color");
    SINFO("\033[%sm%s\033[0m", color_strings[color], message);
#endif
}

void 
platform_get_required_extension_names(void* extension_name_darray) {

}

f64 
platform_get_absolute_time() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}
void 
platform_sleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, 0);
#elif
    sleep(ms / 1000);
    usleep((ms % 1000) * 1000);
#endif
}

#endif
