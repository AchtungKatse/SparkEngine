#include "Spark/core/logging.h"
#include "Spark/core/smemory.h"
#include "Spark/core/sstring.h"
#include "Spark/platform/filesystem.h"
#include "Spark/platform/platform.h"
#include <stdarg.h>

typedef struct logger_system_state {
    file_handle_t log_file_handle;
} logger_system_state;

static logger_system_state* state_ptr;
void append_to_log_file(const char* message);

#define message_buffer_size  32000
char message_buffer[message_buffer_size];
char message_buffer2[message_buffer_size];

b8 initialize_logging(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(logger_system_state);
    if (state == 0) {
        return true;
    }

    state_ptr = state;
       // Create new/wipe existing log file, then open it.
    // if (!filesystem_open("console.log", FILE_MODE_WRITE, false, &state_ptr->log_file_handle)) {
    //     platform_console_write_error("ERROR: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
    //     return false;
    // }

    // TODO: create log file.
    return true;
}

void shutdown_logging(void* state) {
    // TODO: cleanup logging/write queued entries.
    // SDEBUG("Shutting down logging.");
    // filesystem_close(&state_ptr->log_file_handle);
    state_ptr = 0;
}


void pvt_log(log_level_t level, const char* message, ...) {
  const char* level_strings[6] = {
      "\x1B[32m[DEBUG]: ",
      "\x1B[36m[TRACE]: ",
      "\x1B[37m[INFO]:  ",
      "\x1B[33m[WARN]:  ",
      "\x1B[91m[ERROR]: ",
      "\x1B[31m[FATAL]: ",
  };
    szero_memory(message_buffer, sizeof(message_buffer));
    b8 is_error = level > LOG_LEVEL_WARN;

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!
    szero_memory(message_buffer, sizeof(message_buffer));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    string_format_v(message_buffer, message, arg_ptr);
    va_end(arg_ptr);

    string_format(message_buffer2, "%s%s\x1B[0m\n", level_strings[level], message_buffer);

    if (is_error) {
        platform_console_write_error(message_buffer2, level);
    } else {
        platform_console_write(message_buffer2, level);
    }

    append_to_log_file(message_buffer2);
}

void append_to_log_file(const char* message) {
    if (state_ptr && state_ptr->log_file_handle.is_valid) {
        // Since the message already contains a '\n', just write the bytes directly.
        // u64 length = string_length(message);
        // u64 written = 0;
        // if (!filesystem_write(&state_ptr->log_file_handle, length, message, &written)) {
        //     platform_console_write_error("ERROR writing to console.log.", LOG_LEVEL_ERROR);
        // }
    }
}
