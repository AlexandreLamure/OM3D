#include <utils.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <chrono>

#ifdef OS_WIN
#include <windows.h>
#endif

namespace OM3D {

#ifdef OS_WIN
bool running_in_debugger() {
    return IsDebuggerPresent();
}
#endif

void break_in_debugger() {
#ifdef OS_WIN
    if(running_in_debugger()) {
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

// Fatal error: print message and terminate
void fatal(const char* msg, const char* file, int line) {
    std::cerr << msg;

    if(file) {
        std::cerr << " in file \""<< file << "\"";
    }
    if(line) {
        std::cerr << " at line " << line;
    }

    std::cerr << std::endl;

    break_in_debugger();
    std::terminate();
}


static const auto start_time = std::chrono::high_resolution_clock::now();

double program_time() {
    using Seconds = std::chrono::duration<double>;
    return std::chrono::duration_cast<Seconds>(std::chrono::high_resolution_clock::now() - start_time).count();
}

Result<std::string> read_text_file(const std::string& file_name) {
    if(FILE* file = std::fopen(file_name.data(), "r")) {
        DEFER(std::fclose(file));

        std::fpos_t pos = {};
        std::fgetpos(file, &pos);
        std::fseek(file, 0, SEEK_END);
        const size_t size = ftell(file);
        std::fsetpos(file, &pos);

        std::string content(size, '\0');
        const size_t read = std::fread(content.data(), 1, size, file);

        ALWAYS_ASSERT(read <= size, "Unable to read file");
        if(read != size) {
            content.resize(read);
        }

        return {true, std::move(content)};
    }

    return {false, {}};
}


bool ends_with(std::string_view str, std::string_view suffix) {
    if(str.size() < suffix.size()) {
        return false;
    }
    return str.substr(str.size() - suffix.size()) == suffix;
}

}
