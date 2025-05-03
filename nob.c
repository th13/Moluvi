#include <stdio.h>
#include <string.h>
#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_DIR "build/"
#define SRC_DIR "src/"

int build_test(Nob_Cmd *const cmd) {
    nob_cmd_append(cmd, "clang", "-Wall", "-Wextra", "-o", BUILD_DIR "test",
                   SRC_DIR "test.c");
    return nob_cmd_run_sync_and_reset(cmd);
}

int build_example(Nob_Cmd *const cmd) {
    nob_cmd_append(cmd, "clang", "-Wall", "-Wextra", "-o", BUILD_DIR "example",
                   SRC_DIR "example.c");
    nob_cmd_append(cmd, "-I./vendor/raylib-5.5_macos/include/",
                   "-L./vendor/raylib-5.5_macos/lib/", "-lraylib", "-rpath",
                   "./vendor/raylib-5.5_macos/lib/");
    return nob_cmd_run_sync_and_reset(cmd);
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return 1;
    Nob_Cmd cmd = {0};

    nob_shift(argv, argc);
    const char *target = nob_shift(argv, argc);
    printf("Building target %s\n", target);

    if (strcmp(target, "test") == 0) {
        return !build_test(&cmd);
    } else if (strcmp(target, "example") == 0) {
        return !build_example(&cmd);
    }
}
