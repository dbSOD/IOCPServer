#include "DisplayFun.h"
void ButtonDisplay (HWND *CButton, HWND hwnd, HINSTANCE hInstance)
{
//	*CButton = CreateWindow(TEXT("button"),TEXT("�˳�"),WS_CHILD|WS_VISIBLE| BS_PUSHBUTTON,

//				685,115,70,25,hwnd,(HMENU)(ID_EXIT),((LPCREATESTRUCT)lParam)->hInstance,NULL);
	
}
void EditDisplay (read_only_edit *CEdit, HWND hwndParent, HINSTANCE hInstance)
{
	for (int i = 0; i < 5; i ++)
	{
		(CEdit + i)->fnClassInit (hwndParent, 

			WS_CHILD | WS_VISIBLE | 
        
            WS_BORDER | ES_RIGHT,

			65, 20 + i * 40, 40, 20,

			i + 1, hInstance);
		
		(CEdit + 6 + i)->fnClassInit (hwndParent, 

			WS_CHILD | WS_VISIBLE | 
        
            WS_BORDER | ES_RIGHT,

			125, 20 + i * 40, 40, 20,

			i + 7, hInstance);
	}
	(CEdit + 5)->fnClassInit (hwndParent, 

		WS_CHILD | WS_VISIBLE | 
        
        WS_BORDER | ES_RIGHT,

		5, 20, 40, 20,

		6, hInstance);

	for (int i = 0; i < 2; i ++)
	{
		(CEdit + 11 + i)->fnClassInit (hwndParent, 

			WS_CHILD | WS_VISIBLE | 
        
            WS_BORDER | ES_LEFT |
			
			ES_AUTOVSCROLL | ES_MULTILINE,

			205, 20 + 200 * i, 260, 180,

			i + 12, hInstance);
	}
	(CEdit + 13)->fnClassInit (hwndParent, 

		WS_CHILD | WS_VISIBLE | 
        
        WS_BORDER | ES_LEFT,

		480, 20, 400, 20,

		14, hInstance);
	(CEdit + 13)->fnAddContent (L"D:\\");

	(CEdit + 14)->fnClassInit (hwndParent, 

		WS_CHILD | WS_VISIBLE | 
        
        WS_BORDER | ES_LEFT,

		480, 60, 200, 20,

		15, hInstance);
}
void OnPaint (HDC hdc)
{
	TextOut (hdc, 5, 0, TEXT ("����"), lstrlen (TEXT ("����")));

	TextOut (hdc, 65, 0, TEXT ("��ֵ"), lstrlen (TEXT ("��ֵ")));
}
void OnCreate (HWND hwnd, WPARAM wParam, LPARAM lParam, HWND *CButton)
{
	*CButton =  CreateWindow (TEXT("button"), TEXT("��ֵ"),
		
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		
		5, 115, 40, 20, hwnd, (HMENU) (ID_ASSIGN),
		
		((LPCREATESTRUCT) lParam)->hInstance, NULL);

	*(CButton + 1) = CreateWindow (TEXT("button"), TEXT("save"),
		
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		
		5, 155, 40, 20, hwnd, (HMENU) (ID_SAVE),
		
		((LPCREATESTRUCT) lParam)->hInstance, NULL);

	*(CButton + 2) = CreateWindow (TEXT("button"), TEXT("ɾ��"),
		
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		
		5, 195, 40, 20, hwnd, (HMENU) (ID_DELETE),
		
		((LPCREATESTRUCT) lParam)->hInstance, NULL);

	*(CButton + 3) = CreateWindow (TEXT("button"), TEXT("����"),
		
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		
		205, 420, 40, 20, hwnd, (HMENU) (ID_SEND),
		
		((LPCREATESTRUCT) lParam)->hInstance, NULL);

	*(CButton + 4) = CreateWindow (TEXT("button"), TEXT("����"),
		
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		
		480, 120, 40, 20, hwnd, (HMENU) (ID_RECV_FILE),
		
		((LPCREATESTRUCT) lParam)->hInstance, NULL);
}

void OnCommand (HWND hwnd, WPARAM wParam, LPARAM lParam, CDataBase *CDBsocket, read_only_edit *ValueName, bool *bSend)
{
	TCHAR **szBuf;
	switch (LOWORD (wParam))
	{
	case ID_ASSIGN:
		ValueName[5].nSizeOfx = 20;

		ValueName[5].fnGetContent (CDBsocket->ListName, 0);

		for (int i = 0; i < 2; i ++)
		{
			ValueName[i].nSizeOfx = 20;

			ValueName[i].fnGetContent (CDBsocket->ValueName[i], 0);
		}
		break;
	case ID_SAVE:
		szBuf = (TCHAR **) malloc (sizeof (TCHAR *) * CDBsocket->NumOfValue);

		for (int i = 0; i < CDBsocket->NumOfValue; i ++)
		{
			*(szBuf + i) = (TCHAR *) malloc (sizeof (TCHAR) * 20);
			
			ValueName[i + 6].nSizeOfx = 20;

			ValueName[i + 6].fnGetContent (*(szBuf + i), 0);
		}

		CDBsocket->DataBaseInit (hwnd);

		CDBsocket->SaveToDataBase (hwnd, 2, szBuf);

		CDBsocket->DataBaseFree ();

		free (szBuf);
		break;
	case ID_DELETE:
		szBuf = (TCHAR **) malloc (sizeof (TCHAR *) * 1);

		*szBuf = (TCHAR *) malloc (sizeof (TCHAR) * 20);

		ValueName[6].nSizeOfx = 20;

		ValueName[6].fnGetContent (*szBuf, 0);

		CDBsocket->DataBaseInit (hwnd);

		CDBsocket->DeleteDB (hwnd, *CDBsocket->ValueName, *szBuf);

		CDBsocket->DataBaseFree ();

		free (*szBuf);

		free(szBuf);

		break;
	case ID_SEND:
		*bSend = true;
		break;
	case ID_RECV_FILE:
		
		break;
	}
}