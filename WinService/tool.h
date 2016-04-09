#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <time.h>

#define SHARED_MEMORY_NAME "hook_area"

#define WM_MESSAGE_HOOK		WM_USER+200 //
#define WM_KEYBOARD_HOOK	WM_USER+201 //
#define WM_DEBUG_HOOK		WM_USER+202 //

class SharedMemory
{
	char _name[256];
	HANDLE _handle;
	void* _area;
public:
	SharedMemory(const char* name);
	bool create(int size);
	bool open();
	bool destroy();
	void write_int(int val, int pos = 0);
	int read_int(int pos = 0);
	void write_mem(const void* mem, int from, int length);
	void read_mem(void* mem, int from, int length);
};

class Logger
{
public:
	static void log(const char* fmt, ...);
	static void debug(const char* fmt, ...);
	static void write(const char* fmt, ...);
};

bool get_process_name(int pid, char* name);
bool is_vista_or_later();
bool get_save_path(char* path);
