#include "Spark/defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "s3d.h"

const char* extension_blacklist[] = {
    ".psd",
    ".xcf",
    ".blend",
    ".blend1",
};
const u32 extension_blacklist_count = sizeof(extension_blacklist) / sizeof(const char*);

b8 compile_shader(const char* file, const char* output, const char* stage) {
    char compile_command[1024];
    snprintf(compile_command, 1024, "glslc -fshader-stage=%s '%s' -o '%s'", stage, file, output);
    return system(compile_command) == 0;
}

int main(int argc, char** argv) {
    // Arg 0: in file
    // Arg 1: base path
    // Arg 2: out dir

    for (u32 i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    if (argc != 5) {
        printf("Cannot process resource file. Invalid number of arguments (%d). Expected 4 args.\n", argc);
        return -1;
    }

    const char* base_file      = argv[1];
    const char* out_dir        = argv[2];
    const char* file_name      = argv[3];
    const char* file_extension = argv[4];
    char output_path[1024] = {};

    // Dont copy blacklisted files
    for (u32 i = 0; i < extension_blacklist_count; i++) {
        if (strcmp(extension_blacklist[i], file_extension) == 0) {
            return 0;
        }
    }

    strcat(output_path, out_dir);
    strcat(output_path, "/");
    strcat(output_path, file_name);

    if (strcmp(file_extension, ".glb") == 0) {
        strcat(output_path, ".s3d");
        printf("\tConverting glb to s3d\n");
        if (s3d_convert(base_file, output_path) == -1) {
            return -1;
        }
    } else if (strcmp(file_extension, ".frag.glsl") == 0) {
        strcat(output_path, ".frag.spv");
        if (!compile_shader(base_file, output_path, "frag")) {
            return -1;
        }
    } else if (strcmp(file_extension, ".vert.glsl") == 0) {
        strcat(output_path, ".vert.spv");
        if (!compile_shader(base_file, output_path, "vert")) {
            return -1;
        }
    } else if (strcmp(file_extension, ".trs") == 0) {
        // TODO: Convert text resources to binary resources
        strcat(output_path, ".trs");
        char command_buffer[2048];
        snprintf(command_buffer, sizeof(command_buffer), "cp '%s' '%s'", base_file, output_path);
        system(command_buffer);
    } else {
        strcat(output_path, file_extension);
        char command_buffer[2048];
        snprintf(command_buffer, sizeof(command_buffer), "cp '%s' '%s'", base_file, output_path);
        system(command_buffer);
    }

    return 0;
}
