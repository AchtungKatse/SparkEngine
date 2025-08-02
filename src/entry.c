#include "Spark/entry.h"
#include "Spark/core/application.h"
#include "Spark/core/clock.h"
#include "Spark/core/smemory.h"
#include "Spark/math/smath.h"

int main() {
    spark_clock_t startup_timer = {};
    clock_start(&startup_timer);
    game_t game_inst = { };
    if (!create_game(&game_inst)) {
        SCRITICAL("Failed to create game.");
    }

    SASSERT(game_inst.initialize, "Game is missing initialize function");
    SASSERT(game_inst.on_resize, "Game is missing on_resize function");
    SASSERT(game_inst.render, "Game is missing render function");
    SASSERT(game_inst.state, "Game is missing internal state");
    SASSERT(game_inst.update, "Game is missing update function");

    // Initialize
    if (!application_create(&game_inst)) {
        SCRITICAL("Failed to create application");
    }
    clock_update(&startup_timer);
    SINFO("Startup took %f ms", startup_timer.elapsed_time * 1000);

    // Run
    if (!application_run()) {
        SCRITICAL("Application failed to shutdown gracefully.");
    }

    shutdown_memory();

    return 0;
}
