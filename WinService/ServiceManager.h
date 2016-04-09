#pragma once
#include "stdafx.h"
#include "tool.h"

class ServiceManager
{
public:
	ServiceManager(void){}
	~ServiceManager(void){}

	void WriteLog(char* pMsg)
	{
		Logger::debug(pMsg);
	}

	void install(char* pPath, char* pName)
	{  
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE); 
		if (schSCManager==0) 
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "OpenSCManager failed, error code = %d\n", nError);
			WriteLog(pTemp);
		}
		else
		{
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
			if (schService==0) 
			{
				long nError =  GetLastError();
				char pTemp[121];
				sprintf(pTemp, "Failed to create service %s, error code = %d\n", pName, nError);
				WriteLog(pTemp);
			}
			else
			{
				char pTemp[121];
				sprintf(pTemp, "Service %s installed\n", pName);
				WriteLog(pTemp);
				CloseServiceHandle(schService); 
			}
			CloseServiceHandle(schSCManager);
		}	
	}

	void uninstall(char* pName)
	{
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
		if (schSCManager==0) 
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "OpenSCManager failed, error code = %d\n", nError);
			WriteLog(pTemp);
		}
		else
		{
			SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
			if (schService==0) 
			{
				long nError = GetLastError();
				char pTemp[121];
				sprintf(pTemp, "OpenService failed, error code = %d\n", nError);
				WriteLog(pTemp);
			}
			else
			{
				if(!DeleteService(schService)) 
				{
					char pTemp[121];
					sprintf(pTemp, "Failed to delete service %s\n", pName);
					WriteLog(pTemp);
				}
				else 
				{
					char pTemp[121];
					sprintf(pTemp, "Service %s removed\n",pName);
					WriteLog(pTemp);
				}
				CloseServiceHandle(schService); 
			}
			CloseServiceHandle(schSCManager);	
		}
	}

	bool kill_service(char* pName) 
	{ 
		// kill service with given name
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
		if (schSCManager==0) 
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "OpenSCManager failed, error code = %d\n", nError);
			WriteLog(pTemp);
		}
		else
		{
			// open the service
			SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
			if (schService==0) 
			{
				long nError = GetLastError();
				char pTemp[121];
				sprintf(pTemp, "OpenService failed, error code = %d\n", nError);
				WriteLog(pTemp);
			}
			else
			{
				// call ControlService to kill the given service
				SERVICE_STATUS status;
				if(ControlService(schService,SERVICE_CONTROL_STOP,&status))
				{
					CloseServiceHandle(schService); 
					CloseServiceHandle(schSCManager); 
					return true;
				}
				else
				{
					long nError = GetLastError();
					char pTemp[121];
					sprintf(pTemp, "ControlService failed, error code = %d\n", nError);
					WriteLog(pTemp);
				}
				CloseServiceHandle(schService); 
			}
			CloseServiceHandle(schSCManager); 
		}
		return false;
	}

	bool run_service(char* pName) 
	{ 
		// run service with given name
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
		if (schSCManager==0) 
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "OpenSCManager failed, error code = %d\n", nError);
			WriteLog(pTemp);
		}
		else
		{
			// open the service
			SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
			if (schService==0) 
			{
				long nError = GetLastError();
				char pTemp[121];
				sprintf(pTemp, "OpenService failed, error code = %d\n", nError);
				WriteLog(pTemp);
			}
			else
			{
				// call StartService to run the service
				if(StartService(schService, 0, (const char**)NULL))
				{
					CloseServiceHandle(schService); 
					CloseServiceHandle(schSCManager); 
					return true;
				}
				else
				{
					long nError = GetLastError();
					char pTemp[121];
					sprintf(pTemp, "StartService failed, error code = %d\n", nError);
					WriteLog(pTemp);
				}
				CloseServiceHandle(schService); 
			}
			CloseServiceHandle(schSCManager); 
		}
		return false;
	}
};

