#include "Spark/core/clock.h"
#include "Spark/platform/platform.h"

void 
clock_update(spark_clock_t* clock) {
    if (clock->start_time != 0) {
        clock->elapsed_time = platform_get_absolute_time() - clock->start_time;
    }
}

void 
clock_start(spark_clock_t* clock) {
    clock->start_time = platform_get_absolute_time();
    clock->elapsed_time = 0;
}

/**
 * @brief Stops the clock
 * @param clock The clock to be updated
 */
void clock_stop(spark_clock_t* clock) {
    clock->start_time = 0;
}
