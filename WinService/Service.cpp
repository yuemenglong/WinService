//  Service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "KeyLogManager.h"
#include "ServiceManager.h"

#define		MAX_NUM_OF_PROCESS		4
/** Window Service **/
VOID ServiceMainProc();
VOID Install(char* pPath, char* pName);
VOID UnInstall(char* pName);
VOID WriteLog(char* pMsg);
BOOL KillService(char* pName);
BOOL RunService(char* pName);
VOID ExecuteSubProcess();
VOID ProcMonitorThread(VOID *);
BOOL StartProcess(int ProcessIndex);
VOID EndProcess(int ProcessIndex);
VOID AttachProcessNames();

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ServiceHandler(DWORD fdwControl);

void start_working_thread();
void stop_working_thread();

/** Window Service **/
const int nBufferSize = 500;
CHAR pServiceName[nBufferSize+1];
CHAR pExeFile[nBufferSize+1];
CHAR lpCmdLineData[nBufferSize+1];
CHAR pLogFile[nBufferSize+1];
BOOL ProcessStarted = TRUE;

SERVICE_TABLE_ENTRY lpServiceStartTable[] = 
{
	{pServiceName, ServiceMain},
	{NULL, NULL}
};

SERVICE_STATUS_HANDLE   hServiceStatusHandle; 
SERVICE_STATUS          ServiceStatus; 

int _tmain(int argc, _TCHAR* argv[])
{
	if(argc >= 2)
		strcpy(lpCmdLineData, argv[1]);
	ServiceMainProc();
	return 0;
}

VOID ServiceMainProc()
{
	// initialize variables for .exe and .log file names
	char pModuleFile[nBufferSize+1];
	DWORD dwSize = GetModuleFileName(NULL, pModuleFile, nBufferSize);
	pModuleFile[dwSize] = 0;
	if(dwSize>4 && pModuleFile[dwSize-4] == '.')
	{
		sprintf(pExeFile,"%s",pModuleFile);
		pModuleFile[dwSize-4] = 0;
	}
	strcpy(pServiceName,"Sundar_Service");

	//start serivce
	if(!StartServiceCtrlDispatcher(lpServiceStartTable))
	{
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "StartServiceCtrlDispatcher failed, error code = %d\n", nError);
		WriteLog(pTemp);
	}
}

VOID WriteLog(char* pMsg)
{
	Logger::debug(pMsg);
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
    if (hServiceStatusHandle==0) 
    {
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "RegisterServiceCtrlHandler failed, error code = %d\n", nError);
		WriteLog(pTemp);
        return; 
    } 
 
    // Initialization complete - report running status 
    ServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0;  
    if(!SetServiceStatus(hServiceStatusHandle, &ServiceStatus)) 
    { 
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "SetServiceStatus failed, error code = %d\n", nError);
		WriteLog(pTemp);
    } 

	// AttachProcessNames();
	// for(int iLoop = 0; iLoop < MAX_NUM_OF_PROCESS; iLoop++)
	// {
	// 	pProcInfo[iLoop].hProcess = 0;
	// 	StartProcess(iLoop);
	// }
	start_working_thread();
}

void start_working_thread(){

}
void stop_working_thread(){

}

VOID WINAPI ServiceHandler(DWORD fdwControl)
{
	switch(fdwControl) 
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			ProcessStarted = FALSE;
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
			char pTemp[121];
			sprintf(pTemp,  "Unrecognized opcode %d\n", fdwControl);
			WriteLog(pTemp);
	};
    if (!SetServiceStatus(hServiceStatusHandle,  &ServiceStatus)) 
	{ 
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "SetServiceStatus failed, error code = %d\n", nError);
		WriteLog(pTemp);
    } 
}
