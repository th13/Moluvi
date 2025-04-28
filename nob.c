#define NOB_IMPLEMENTATION

#include "nob.h"

#define BUILD_DIR "build/"
#define SRC_DIR "src/"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return 1;
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "clang", "-Wall", "-Wextra", "-o", BUILD_DIR "test",
                   SRC_DIR "test.c");
    nob_cmd_run_sync_and_reset(&cmd);
}
