#pragma once
#include "Spark/defines.h"
#include "stdlib.h"

typedef enum log_level {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_CRITICAL,
} log_level_t;


b8 initialize_logging(u64* memory_requirement, void* state);
void shutdown_logging(void* state);
void pvt_log(log_level_t level, const char* message, ...);

#ifdef SPARK_DEBUG

#define SDEBUG(...)         pvt_log(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define STRACE(...)         pvt_log(LOG_LEVEL_TRACE, __VA_ARGS__)
#define SINFO(...)          pvt_log(LOG_LEVEL_INFO, __VA_ARGS__)
#define SWARN(...)          pvt_log(LOG_LEVEL_WARN, __VA_ARGS__)
#define SERROR(...)         pvt_log(LOG_LEVEL_ERROR, __VA_ARGS__)
#define SCRITICAL(...)      pvt_log(LOG_LEVEL_CRITICAL, __VA_ARGS__); abort()

#define SASSERT(cond, ...) if (!(cond)) { SERROR("Assertion '" #cond "' failed: " __VA_ARGS__); SERROR("\t\t" __FILE__ ":%i", __LINE__); abort(); }
#define SRETURN_ASSERT(cond, fail_return, ...) if (!(cond)) { SERROR("Assertion '" #cond "' failed: " __VA_ARGS__); SERROR("\t\t" __FILE__ ":%i", __LINE__); exit(-1); return fail_return; }

#else

#define SDEBUG(...)
#define STRACE(...)
#define SINFO(...)
#define SWARN(...)

#ifndef SPARK_NO_ERROR_CHECKING

#define SASSERT(cond, ...) if (!(cond)) { SERROR("Assertion failed " __FILE__ ":%i", __LINE__); }
#define SRETURN_ASSERT(cond, fail_return, ...) if (!(cond)) { SERROR("Assertion failed " __FILE__ ":%i", __LINE__); exit(-1); return fail_return; }
#define SERROR(...)         pvt_log(LOG_LEVEL_ERROR, __VA_ARGS__)
#define SCRITICAL(...)      pvt_log(LOG_LEVEL_CRITICAL, __VA_ARGS__)

#else

#define SASSERT(cond, ...)
#define SRETURN_ASSERT(cond, fail_return, ...)
#define SERROR(...)
#define SCRITICAL(...)

#endif

#endif

