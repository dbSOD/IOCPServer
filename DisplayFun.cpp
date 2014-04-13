#include "DisplayFun.h"
#include <process.h>
void ObjManager::EditDisplay ()
{
	read_only_edit edit;
	for (int i = 0; i < 5; i ++)
	{
		edit.fnClassInit (m_hwndMain, 

			WS_CHILD | WS_VISIBLE | 

			WS_BORDER | ES_RIGHT | ES_AUTOHSCROLL,

			65, 20 + i * 40, 40, 20,

			i + 1, m_hInstance);
		m_edit.push_back(edit);
	}
	edit.fnClassInit (m_hwndMain, 

		WS_CHILD | WS_VISIBLE | 

		WS_BORDER | ES_RIGHT | ES_AUTOHSCROLL,

		5, 20, 40, 20,

		6, m_hInstance);
	m_edit.push_back(edit);
	for(int i = 0; i < 5; i++)
	{
		edit.fnClassInit (m_hwndMain, 

			WS_CHILD | WS_VISIBLE | 

			WS_BORDER | ES_RIGHT | ES_AUTOHSCROLL,

			125, 20 + i * 40, 40, 20,

			i + 7, m_hInstance);
		m_edit.push_back(edit);
	}


	edit.fnClassInit (m_hwndMain, //һ�����ʹ���

		WS_CHILD | WS_VISIBLE | 

		WS_BORDER | ES_LEFT |

		ES_AUTOVSCROLL | ES_MULTILINE,

		205, 20, 260, 180,

		12, m_hInstance);

	m_edit.push_back(edit);
	edit.fnClassInit (m_hwndMain, //�����ܴ���

		WS_CHILD | WS_VISIBLE | 

		WS_BORDER | ES_LEFT | ES_READONLY |

		ES_AUTOVSCROLL | ES_MULTILINE,

		205, 20 + 200, 260, 180,

		13, m_hInstance);
	m_edit.push_back(edit);
	edit.fnClassInit (m_hwndMain, 

		WS_CHILD | WS_VISIBLE | 

		WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,

		480, 20, 400, 20,

		14, m_hInstance);
	m_edit.push_back(edit);

	edit.fnClassInit (m_hwndMain, 

		WS_CHILD | WS_VISIBLE | 

		WS_BORDER | ES_LEFT,

		480, 60, 200, 20,

		15, m_hInstance);
	
	m_edit.push_back(edit);
	m_edit[13].fnAddContent (L"C:\\mydatabase.mdb");

	m_edit[5].fnReplaceContent(L"���ݱ�");

	m_edit[0].fnReplaceContent(L"ѧ��");

	m_edit[1].fnReplaceContent(L"����");
}
void ObjManager::OnPaint ()
{
	PAINTSTRUCT	ps ;
	m_hdc = BeginPaint(m_hwndMain, &ps);

	TextOut (m_hdc, 5, 0, TEXT ("����"), lstrlen (TEXT ("����")));

	TextOut (m_hdc, 65, 0, TEXT ("��ֵ"), lstrlen (TEXT ("��ֵ")));
	EndPaint(m_hwndMain, &ps);
}
void ObjManager::OnCreate ()
{
	HWND button;
	button =  CreateWindow (TEXT("button"), TEXT("��ֵ"),

		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,

		5, 115, 40, 20, m_hwndMain, (HMENU) (ID_ASSIGN),

		m_hInstance, NULL);

	m_button.push_back(button);

	button = CreateWindow (TEXT("button"), TEXT("save"),

		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,

		5, 155, 40, 20, m_hwndMain, (HMENU) (ID_SAVE),

		m_hInstance, NULL);
	m_button.push_back(button);

	button = CreateWindow (TEXT("button"), TEXT("ɾ��"),

		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,

		5, 195, 40, 20, m_hwndMain, (HMENU) (ID_DELETE),

		m_hInstance, NULL);
	m_button.push_back(button);

	button = CreateWindow (TEXT("button"), TEXT("����"),

		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,

		205, 420, 40, 20, m_hwndMain, (HMENU) (ID_SEND),

		m_hInstance, NULL);
	m_button.push_back(button);

	button = CreateWindow (TEXT("button"), TEXT("����"),

		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,

		480, 120, 40, 20, m_hwndMain, (HMENU) (ID_RECV_FILE),

		m_hInstance, NULL);
	m_button.push_back(button);
	button = CreateWindow (TEXT("button"), TEXT("��ѯ"),

		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,

		5, 235, 40, 20, m_hwndMain, (HMENU) (ID_RECV_INQUIRE),

		m_hInstance, NULL);
	m_button.push_back(button);
	SendMessage(m_hwndMain, WM_COMMAND, ID_ASSIGN, NULL);
	_beginthread(IOCPThread, 0, NULL);
}
void ObjManager::OnCommand (WPARAM wParam, LPARAM lParam)
{
	static TCHAR szBuf[5][20];
	switch (LOWORD (wParam))
	{
	case ID_ASSIGN:
		m_edit[5].nSizeOfx = 20;

		m_edit[5].fnGetContent (m_CDBSocket.ListName, 0);

		for (int i = 0; i < 2; i ++)
		{
			m_edit[i].nSizeOfx = 20;

			m_edit[i].fnGetContent (m_CDBSocket.ValueName[i], 0);
		}

		m_edit[13].fnGetContent(m_CDBSocket.fileName, 0);
		break;
	case ID_SAVE:
		for (int i = 0; i < m_CDBSocket.NumOfValue; i ++)
		{
			m_edit[i + 6].nSizeOfx = 20;

			m_edit[i + 6].fnGetContent (*(szBuf + i), 0);
		}

		m_CDBSocket.DataBaseInit (m_hwndMain);

		m_CDBSocket.SaveToDataBase (m_hwndMain, 2, szBuf);

		m_CDBSocket.DataBaseFree ();

		break;
	case ID_DELETE:
		m_edit[6].nSizeOfx = 20;

		m_edit[6].fnGetContent (*szBuf, 0);

		m_CDBSocket.DataBaseInit (m_hwndMain);

		m_CDBSocket.DeleteDB (m_hwndMain, *m_CDBSocket.ValueName, *szBuf);

		m_CDBSocket.DataBaseFree ();
		break;
	case ID_SEND:
		break;
	case ID_RECV_FILE:
		m_CDBSocket.DataBaseInit (m_hwndMain);

		((CQuery*)lParam)->bState = m_CDBSocket.inquireDB(m_hwndMain, ((CQuery *)lParam)->valueName, ((CQuery *)lParam)->subName);

		m_CDBSocket.DataBaseFree ();
		break;
	case ID_RECV_INQUIRE:
		break;
	}
}
ObjManager* ObjManager::getInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new ObjManager();
	}
	return m_instance;
}
ObjManager::~ObjManager()
{
	delete m_instance;
}
ObjManager::ObjManager()
{
	m_hwndMain = NULL;
}
void ObjManager::ObjInit(HINSTANCE hInstance, HWND hwnd)
{
	m_hInstance = hInstance;
	m_hwndMain = hwnd;
}
void ObjManager::IOCPThread(PVOID pvoid)
{
	CIOCP iocp;

	if( !iocp.Init() )
	{
	}

	iocp.MainLoop();
}
ObjManager* ObjManager::m_instance = NULL;