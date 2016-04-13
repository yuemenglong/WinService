#pragma once
#include "stdafx.h"
#include "tool.h"
#include "Logger.h"
#include "tlhelp32.h" 

class ServiceManager
{
public:
	ServiceManager(void){}
	~ServiceManager(void){}
	static bool install(char* pPath, char* pName){  
		bool ret = false;
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE); 
		if (schSCManager==0) {
			Logger::debug("OpenSCManager failed, error code = %d\n", GetLastError());
		} else {
			SC_HANDLE schService = CreateService( 
				schSCManager,	/* SCManager database      */ 
				pName,			/* name of service         */ 
				pName,			/* service name to display */ 
				SERVICE_ALL_ACCESS,        /* desired access          */ 
				SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS , /* service type            */ 
				SERVICE_AUTO_START,      /* start type              */ 
				SERVICE_ERROR_NORMAL,      /* error control type      */ 
				pPath,			/* service's binary        */ 
				NULL,                      /* no load ordering group  */ 
				NULL,                      /* no tag identifier       */ 
				NULL,                      /* no dependencies         */ 
				NULL,                      /* LocalSystem account     */ 
				NULL
				);                    
			if (schService==0) {
				Logger::debug("Failed to create service %s, error code = %d\n", pName, GetLastError());
			}
			else {
				Logger::debug("Service %s installed\n", pName);
				ret = true;
			}
			CloseServiceHandle(schSCManager);
		}	
		return ret;
	}
	static void uninstall(char* pName){
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
		if (schSCManager==0) {
			Logger::debug("OpenSCManager failed, error code = %d\n", GetLastError());
		} else {
			SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
			if (schService==0) {
				Logger::debug("OpenService failed, error code = %d\n", GetLastError());
			} else {
				if(!DeleteService(schService)) {
					Logger::debug("Failed to delete service %s\n", pName);
				} else {
					Logger::debug("Service %s removed\n",pName);
				}
				CloseServiceHandle(schService); 
			}
			CloseServiceHandle(schSCManager);	
		}
	}
	static bool kill(char* pName) { 
		// kill service with given name
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
		if (schSCManager==0) {
			Logger::debug("OpenSCManager failed, error code = %d\n", GetLastError());
		}else{
			// open the service
			SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
			if (schService==0) {
				Logger::debug("OpenService failed, error code = %d\n", GetLastError());
			} else {
				// call ControlService to kill the given service
				SERVICE_STATUS status;
				if(ControlService(schService,SERVICE_CONTROL_STOP,&status)){
					CloseServiceHandle(schService); 
					CloseServiceHandle(schSCManager); 
					return true;
				} else {
					Logger::debug("ControlService failed, error code = %d\n", GetLastError());
				}
				CloseServiceHandle(schService); 
			}
			CloseServiceHandle(schSCManager); 
		}
		return false;
	}
	static bool run(char* pName){ 
		// run service with given name
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
		if (schSCManager==0) {
			Logger::debug("OpenSCManager failed, error code = %d\n", GetLastError());
		} else {
			// open the service
			SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
			if (schService==0) {
				Logger::debug("OpenService failed, error code = %d\n", GetLastError());
			}else{
				// call StartService to run the service
				if(StartService(schService, 0, (const char**)NULL)){
					CloseServiceHandle(schService); 
					CloseServiceHandle(schSCManager); 
					return true;
				}else{
					Logger::debug("StartService failed, error code = %d\n", GetLastError());
				}
				CloseServiceHandle(schService); 
			}
			CloseServiceHandle(schSCManager); 
		}
		return false;
	}
	static bool is_service_env(){
		DWORD dwSessionId;
		ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionId);
		return dwSessionId == 0;
		//HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		//return handle == 0;
	}
	static bool create_process_as_user(char* cmd, PROCESS_INFORMATION* info){
		if(!cmd){
			return false;
		}
		HANDLE hToken;
		if(!get_process_token("EXPLORER.EXE", hToken)){
			return false;
		}
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb= sizeof(STARTUPINFO);
		si.lpDesktop = TEXT("winsta0\\default");

		BOOL bResult = CreateProcessAsUser(hToken, NULL, cmd, NULL, NULL,
			false, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, info);
		CloseHandle(hToken);
		if(bResult){
			Logger::debug("CreateProcessAsUser ok!\r\n");
		}else{
			Logger::debug("CreateProcessAsUser false!\r\n");
		}
		return bResult;
	}
	static bool get_process_token(char* proc, HANDLE& token){
		if(!proc){
			return false;
		}
		HANDLE hProcessSnap = NULL;  
		BOOL bRet = false;  
		PROCESSENTRY32 pe32 = {0};  

		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  
		if (hProcessSnap == INVALID_HANDLE_VALUE)  
			return (false);  

		pe32.dwSize = sizeof(PROCESSENTRY32);  

		if (Process32First(hProcessSnap, &pe32)){   
			do{
				if(!strcmp(_strupr(pe32.szExeFile),_strupr(proc))){
					HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
						false,pe32.th32ProcessID);
					bRet = OpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&token);
					CloseHandle (hProcessSnap);  
					return (bRet);
				}
			}  
			while (Process32Next(hProcessSnap, &pe32));  
			bRet = true;  
		}else  
			bRet = false;

		CloseHandle (hProcessSnap);  
		return (bRet);
	}
};

