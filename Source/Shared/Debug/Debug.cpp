#include "Debug.h"
#include "ImGuiLog.h"

#include <stdio.h>
#include <stdarg.h>
#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _WIN32

#define PLATFORM_FORMAT_LOG_MESSAGE(fn, level)  \
	char buf[16*1024]; \
    va_list ap; \
    va_start(ap, fmt); \
	vsprintf_s(buf,16*1024, fmt, ap); \
	va_end(ap); \
	fn(buf); 

#define PLATFORM_FORMAT_LOG_MESSAGE_LF(fn, level)  \
	char buf[16*1024]; \
    va_list ap; \
    va_start(ap, fmt); \
	vsprintf_s(buf,16*1024, fmt, ap); \
	va_end(ap); \
	strcat_s(buf,16*1024, "\n"); \
	fn(buf); 
#else
#define PLATFORM_FORMAT_LOG_MESSAGE(fn, level)  \
	char buf[16*1024]; \
    va_list ap; \
    va_start(ap, fmt); \
	vsprintf(buf, fmt, ap); \
	va_end(ap); \
	fn(buf); 

#define PLATFORM_FORMAT_LOG_MESSAGE_LF(fn, level)  \
	char buf[16*1024]; \
    va_list ap; \
    va_start(ap, fmt); \
	vsprintf(buf, fmt, ap); \
	va_end(ap); \
	strcat(buf, "\n"); \
	fn(buf); 
#endif

void LogFatal(const char* str)
{
#ifdef WIN32
	OutputDebugStringA(str);
#endif
	g_ImGuiLog.AddLog("[Fatal]%s", str);
}

void LogError(const char* str)
{
#ifdef WIN32
	OutputDebugStringA(str);
#endif
	g_ImGuiLog.AddLog("[Error]%s", str);
}

void LogWarning(const char* str)
{
#ifdef WIN32
	OutputDebugStringA(str);
#endif
	g_ImGuiLog.AddLog("[Warning]%s", str);
}

void LogInfo(const char* str)
{
#ifdef WIN32
	OutputDebugStringA(str);
#endif
	g_ImGuiLog.AddLog("[Info]%s", str);
}

void LogDebug(const char* str)
{
#ifdef WIN32
	OutputDebugStringA(str);
#endif
	g_ImGuiLog.AddLog("[Debug]%s", str);
}

void _LogFatalfLF(const char* fmt, ...)
{
	PLATFORM_FORMAT_LOG_MESSAGE_LF(LogFatal, 0);//dvLogLevel_Fatal);
}

void _LogErrorfLF(const char* fmt, ...)
{
	PLATFORM_FORMAT_LOG_MESSAGE_LF(LogError, 0);//dvLogLevel_Error);
}

void _LogWarningfLF(const char* fmt, ...)
{
	PLATFORM_FORMAT_LOG_MESSAGE_LF(LogWarning, 0);//dvLogLevel_Warning);
}

void _LogInfofLF(const char* fmt, ...)
{
	PLATFORM_FORMAT_LOG_MESSAGE_LF(LogInfo, 0);//dvLogLevel_Info);
}

void _LogDebugfLF(const char* fmt, ...)
{
	PLATFORM_FORMAT_LOG_MESSAGE_LF(LogDebug, 0);//dvLogLevel_Debug);
}