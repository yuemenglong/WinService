#pragma once

#include <windows.h>
#include <stdio.h>
#include "Tool.h"

class Logger
{
public:
	static void log(const char* fmt, ...){
		FILE* fp = fopen("D:/hooklog.txt", "a+");
		if(!fp){
			return;
		}
		va_list arg_ptr;
		va_start(arg_ptr, fmt);
		vfprintf(fp, fmt, arg_ptr);
		va_end(arg_ptr);

		fprintf(fp, "\n");

		fclose(fp);
	}

	static void debug(const char* fmt, ...){
#ifdef _DEBUG
		char output_fmt[1024];
		sprintf(output_fmt, "[Logger]=>%s\n", fmt);
		char str[1024];
		va_list arg_ptr;
		va_start(arg_ptr, fmt);
		vsprintf(str, output_fmt, arg_ptr);
		va_end(arg_ptr);
		OutputDebugStringA(str);
#else
		return;
#endif
	}

	static void write(const char* fmt, ...){
		static char last_window_name[128];
		char path[MAX_PATH] = {0};
		Tool::get_save_path(path);
		FILE* fp = fopen(path, "a+");
		if(!fp){
			return;
		}
		HWND hwnd = GetForegroundWindow();
		char window_name[128];
		GetWindowText(hwnd, window_name, sizeof(window_name));
		if(strcmp(window_name, last_window_name) != 0){
			char time_str[64] = {0};
			time_t now = time(NULL);
			strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now)); 
			fprintf(fp, "\n%s\t%s\n", time_str, window_name);
			strncpy(last_window_name, window_name, sizeof(window_name));
		}
		va_list arg_ptr;
		va_start(arg_ptr, fmt);
		vfprintf(fp, fmt, arg_ptr);
		va_end(arg_ptr);

		fclose(fp);
	}
};