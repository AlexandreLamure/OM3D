#include <utils.h>

#include <csignal>
#include <cstdio>

#ifdef OS_WIN
#include <windows.h>
#endif



void break_in_debugger() {
#ifdef OS_WIN
    if(IsDebuggerPresent()) {
        DebugBreak();
    }
#endif
#ifdef OS_LINUX
    static bool handler_setup = false;
    if(!handler_setup) {
        std::signal(SIGTRAP, [](int) {});
        handler_setup = true;
    }
    std::raise(SIGTRAP);
#endif
}


Result<std::string> read_file(const std::string& file_name) {
    if(FILE* file = std::fopen(file_name.data(), "r")) {
        DEFER(std::fclose(file));

        std::fpos_t pos = {};
        std::fgetpos(file, &pos);
        std::fseek(file, 0, SEEK_END);
        const size_t size = ftell(file);
        std::fsetpos(file, &pos);

        std::string content(size, '\0');
        const size_t read = std::fread(content.data(), 1, size, file);

        return {read == size, std::move(content)};
    }

    return {false, {}};
}
