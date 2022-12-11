#include "logger.h"

void logger_log(const char *msg)
{
    printf("--> %s\n",msg);
}

void logger_warn(const char *msg)
{
    printf("--> WARN: %s\n",msg);
}

void logger_error(const char *msg)
{
    printf("--> ERROR: %s\n",msg);
}
