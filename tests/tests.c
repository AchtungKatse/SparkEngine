#include "Spark/defines.h"
#include "Spark/core/logging.h"

void hashmap_test();
void freelist_test();

s32 main(s32 argc, char** argv) {
    freelist_test();
    hashmap_test();
}
