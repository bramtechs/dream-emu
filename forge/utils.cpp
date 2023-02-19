
#define EXISTS(X) std::filesystem::exists(X)

bool SUBFILES_EXISTS(std::string& path) {
    int count = 0;
    for (const auto& file : std::filesystem::directory_iterator(path)){
        if (!file.is_directory()){
            count++;
        }
    }
    return count > 0;
}

std::string concat_sep(char sep, std::initializer_list<std::string> list) {
    std::string result = "";
    for (const auto elem : list) {
        result += elem;
        result += sep;
    }
    return result;
}

void MKDIRS(std::initializer_list<std::string> list){
    std::string path = concat_sep(PATH_SEP,list);
    std::filesystem::create_directories(path);
}

// TODO: don't crash on linux when cmd fails
bool CMD(std::initializer_list<std::string> list){
#if __linux__
    size_t argc = list.count;
    const char **argv = malloc(sizeof(const char*) * (argc - 1));

    argc = 0;
    printf(">> ");
    va_start(args,silent);
    for (const auto elem : list){
        argv[argc++] = arg;
        printf("%s ",arg);
    }
    printf("\n");

    assert(argc >= 1);
    int cpid = fork();
    if (cpid == -1){
        std::cerr << "Could not fork a child process! " << strerror(errno)
                                                        << std::endl;
        exit(1);
    }

    if (cpid == 0){
        if (execvp(argv[0], (char * const*) argv) < 0){
            std::cerr << "Sub-process failed: " << strerror(errno) << std::endl;
            exit(1);
        }
    }
    else if (cpid > 0) {
        wait(NULL); // wait for child to end
    }
    else {
        std::cerr << ("Fork failed: " << strerror(errno) << std::endl;
        exit(1);
    }

#elif defined(_WIN32) || defined(WIN32)

    if (!system(NULL)){
        std::cerr << "No command processor found!" << std::endl;
        exit(1);
    }

    std::string cmd = concat_sep(' ',list);
    std::cout << ">> " << cmd << std::endl;

    int code = system(cmd.c_str());
    return code == 0;
#endif
}
