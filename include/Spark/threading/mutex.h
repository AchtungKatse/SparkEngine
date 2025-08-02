#pragma once

typedef struct {
    void* internal_data;
} spark_mutex_t;

void mutex_create(spark_mutex_t* out_mutex);
void mutex_destroy(spark_mutex_t* out_mutex);
void mutex_lock(spark_mutex_t mutex);
void mutex_unlock(spark_mutex_t mutex);
