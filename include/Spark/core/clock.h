#pragma once
#include "Spark/defines.h"

typedef struct {
    f64 start_time;
    f64 elapsed_time;
} spark_clock_t;

/**
 * @brief Updates clock state and recalculates elapsed time.
 * @param clock The clock to be updated
 */
SAPI void clock_update(spark_clock_t* clock);

/**
 * @brief Start the clock
 * @param clock The clock to be updated
 */
SAPI void clock_start(spark_clock_t* clock);

/**
 * @brief Stops the clock
 * @param clock The clock to be updated
 */
SAPI void clock_stop(spark_clock_t* clock);


