#pragma once

SINLINE void loader_read_line(const char* text, u32* const text_offset, char* arg_buffer, u32 arg_buffer_count, u32 arg_buffer_size, u32* out_arg_count) {
    arg_buffer[0] = 0;
    for (u32 i = 0, arg_index = 0;; i++) {
        u32 offset = *text_offset;
        *text_offset += 1;
        char c = text[offset];

        if (i >= arg_buffer_size) {
            SERROR("Resource loader trying to read more than max number of arguments.");
            break;
        }

        // Check for delimeters
        if (c == ',' || c == ':') {
            arg_buffer[arg_index * arg_buffer_size + i] = 0;
            *out_arg_count += 1;
            arg_index++;
            i = -1; // Needs to be -1 since the loop will increment to 0
            continue;
        }

        // Check end of line
        if (c == 0 || c == '\n') {
            arg_buffer[arg_index * arg_buffer_size +i] = 0;
            *out_arg_count += 1;
            break;
        }

        // Check for comments
        if (c == ';' || c == '#') {
            while (c != 0 && c != '\n') {
                u32 offset = *text_offset;
                *text_offset += 1;
                c = text[offset];
            }
            break;
        }

        // Add character
        arg_buffer[arg_index * arg_buffer_size + i] = c;
    }
}
