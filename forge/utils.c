// C macros :)
#define FOREACH_VARGS(arg,args,body)                        \
    do {                                                    \
        for (const char* arg = va_arg(args,const char*);    \
                arg != NULL;                                \
                arg = va_arg(args,const char*))             \
        {                                                   \
            body;                                           \
        }                                                   \
    } while(0)

const char* form_path(int dummy, ...) {
    size_t length = 0;
    ssize_t seps = -1;

    // calculate length of vargs
    va_list args;
    va_start(args,dummy);
    FOREACH_VARGS(arg,args,{
        length += strlen(arg);
        seps++;
    });
    va_end(args);

    assert(length > 0);

    // allocate string with null terminator
    char* result = malloc(length + seps * PATH_SEP_LEN + 1);

    length = 0;
    va_start(args,dummy);
    FOREACH_VARGS(arg,args,{
        size_t n = strlen(arg);
        memcpy(result + length, arg, n);
        length += n;
        if (seps > 0){
            memcpy(result + length, PATH_SEP, PATH_SEP_LEN);
            length += PATH_SEP_LEN;
            seps--;
        }
    });
    va_end(args);

    return result;
}

void make_dirs(int dummy, ...){
    size_t length = 0;
    ssize_t seps = -1;

    // calculate length of vargs
    va_list args;
    va_start(args,dummy);
    FOREACH_VARGS(arg,args,{
        length += strlen(arg);
        seps++;
    });
    va_end(args);

    assert(length > 0);

    // allocate string with null terminator
    char* result = malloc(length + seps * PATH_SEP_LEN + 1);

    length = 0;
    va_start(args,dummy);
    FOREACH_VARGS(arg,args,{
        size_t n = strlen(arg);
        memcpy(result + length, arg, n);
        length += n;
        if (seps > 0){
            memcpy(result + length, PATH_SEP, PATH_SEP_LEN);
            length += PATH_SEP_LEN;
            seps--;
        }

        result[length] = '\0';
        if (mkdir(result,0755) < 0){
            if (errno == EEXIST){
                PRINT(".. %s",result);
            }
            else {
                ERROR("Could not create folder %s",result,strerror(errno));
                exit(1);
            }
        } else {
            PRINT(".. %s (new)",result);
        }
    });
    va_end(args);
}

bool run_cmd(int dummy, ...){
    size_t argc = 0;

    va_list args;
    va_start(args,dummy);
    FOREACH_VARGS(arg, args, {
        argc++;
    });
    va_end(args);

    const char **argv = malloc(sizeof(const char*) * (argc - 1));
    argc = 0;

    printf(">> ");
    va_start(args,dummy);
    FOREACH_VARGS(arg, args, {
        argv[argc++] = arg;
        printf("%s ",arg);
    });
    va_end(args);
    printf("\n");

    assert(argc >= 1);

    pid_t cpid = fork();
    if (cpid == -1){
        ERROR("Could not fork a child process! %s", strerror(errno));
        exit(1);
    }

    if (cpid == 0){
        if (execvp(argv[0], (char * const*) argv) < 0){
            ERROR("Sub-process failed: %s",strerror(errno));
            exit(1);
        }
    }
    else if (cpid > 0) {
        wait(NULL); // wait for child to end
    }
    else {
        ERROR("Fork failed: %s",strerror(errno));
        exit(1);
    }
}
