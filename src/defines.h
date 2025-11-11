#ifndef DEFINES_H
#define DEFINES_H

#define CREATE_NAME_LINE_HELPER(prefix, LINE) _generated_ ## prefix ## _at_ ## LINE
#define CREATE_NAME_HELPER(prefix, LINE) CREATE_NAME_LINE_HELPER(prefix, LINE)
#define CREATE_UNIQUE_NAME_WITH_PREFIX(prefix) CREATE_NAME_HELPER(prefix, __LINE__)
#define CREATE_UNIQUE_NAME CREATE_UNIQUE_NAME_WITH_PREFIX()


/****************** OS DEFINES BELOW ******************/

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__) || defined(_WINDOWS)
#define OS_WIN
#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#if defined(__linux__) || defined(__gnu_linux__)
#define OS_LINUX
#endif

#endif // DEFINES_H
