#ifdef __unix___

#define INFO(X...) logger_log(TextFormat(X))
#define WARN(X...) logger_warn(TextFormat(X))
#define ERROR(X...) logger_error(TextFormat(X))

#elif defined(_WIN32) || defined(WIN32)

#define INFO(X, ...) logger_log(TextFormat(X,__VA_ARGS__))
#define WARN(X, ...) logger_warn(TextFormat(X,__VA_ARGS__))
#define ERROR(X, ...) logger_error(TextFormat(X,__VA_ARGS__))

#endif

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
