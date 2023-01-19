#pragma once

void _LogFatalfLF(const char* fmt, ...);
void _LogErrorfLF(const char* fmt, ...);
void _LogWarningfLF(const char* fmt, ...);
void _LogInfofLF(const char* fmt, ...);
void _LogDebugfLF(const char* fmt, ...);

#define LOGERROR(...) 		_LogErrorfLF(__VA_ARGS__)
#define LOGWARNING(...) 	_LogWarningfLF(__VA_ARGS__)
#define LOGINFO(...) 		_LogInfofLF(__VA_ARGS__)
#define LOGDEBUG(...) 		_LogDebugfLF(__VA_ARGS__)

