#include<WinSock2.h>
#include<Windows.h>
#include<CommCtrl.h>
#include<stdio.h>
#include"browser.h"
#include"resource.h"
#include<stack>
#include<list>
#include<vector>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma warning(disable:4996)

#define SOUND_FILE_NAME "test.wav"
#define PORT_NUM 8777
#define MAXLEN 8192


HINSTANCE g_hIsnt;
HWND hEdit; //에디트박스(URL)
HWND mainhWnd; //메인 윈도우 핸들러
HWND toolhWnd; //toolbar 윈도우 핸들러

WNDPROC OldEditProc1;
WNDPROC BtnOk;

int yPos, xPos;
int yMax, xMax;
int index = -1;
int enterFlag;
int reFlag;
int tempCmd;
WCHAR* msg;
WCHAR buf[MAXLEN] = { '\0', };

stack<wstring> backHistroy;
stack<wstring> frontHistory;
list<wstring> fav;
vector<wstring> history;

////////////////////////리스트뷰//////////////////
HWND listhWnd;
//////////////////////////////////////////////////


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK listSubProc1(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EditSubProc1(HWND, UINT, WPARAM, LPARAM);

void getCode(wstring tempMsg);
void createToolBar();
void initializeVar();
void initializeWindow(HINSTANCE hInstance, WNDCLASS& WndClass, LPCTSTR& lpszClass);

LPCTSTR lpszClass = TEXT("Tiny Web browser");
LPCTSTR lpszClass1 = TEXT("List View");

class browser bw;

class CGdiPlusStarter
{
private:
	ULONG_PTR m_gpToken;

public:
	bool m_bSuccess;
	CGdiPlusStarter() {
		GdiplusStartupInput gpsi;
		m_bSuccess = (GdiplusStartup(&m_gpToken, &gpsi, NULL) == Ok);
	}
	~CGdiPlusStarter() {
		GdiplusShutdown(m_gpToken);
	}
};

CGdiPlusStarter g_gps;


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	/*윈도우 관리 변수*/
	MSG Message = { 0, };
	WNDCLASS WndClass = { 0, };

	tempCmd = nCmdShow;
	/*소켓 관리 변수*/
	WSADATA wsaData = { 0, };

	if (g_gps.m_bSuccess == FALSE) {
		printf("error\n");
		return 0;
	}

	initializeVar();
	initializeWindow(hInstance, WndClass, lpszClass);

	g_hIsnt = hInstance;

	mainhWnd = CreateWindow(lpszClass, lpszClass, WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW | WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);

	createToolBar();

	ShowWindow(mainhWnd, nCmdShow);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return 0;
	}
	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	WSACleanup();
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int xInc = 0, yInc = 0;
	int mX = 0;
	int mY = 0;
	HDC hdc = 0; 
	PAINTSTRUCT ps = { 0, };
	wstring tempMsg = TEXT(""); 

	vector<struct hyperLink>::iterator it;

	switch (iMessage)
	{
	case WM_CREATE:
		xPos = 0, yPos = 0;
		xMax = 1024, yMax = 768;
		SetScrollRange(mainhWnd, SB_VERT, 0, yMax, TRUE);
		SetScrollPos(mainhWnd, SB_VERT, 0, TRUE);
		SetScrollRange(mainhWnd, SB_HORZ, 0, xMax, TRUE);
		SetScrollPos(mainhWnd, SB_HORZ, 0, TRUE);

		hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 70, 50, 500, 25, hWnd, (HMENU)100, g_hIsnt, NULL);
		OldEditProc1 = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)EditSubProc1);
		SetFocus(hEdit);

		return 0;

	case WM_LBUTTONDOWN:
		mX = LOWORD(lParam);
		mY = HIWORD(lParam);
		for (it = bw.linker.begin(); it != bw.linker.end(); it++)
		{
			//주소 값 안에 오면
			if (mX >= it->x && mX <= (it->x + it->width) && mY >= it->y && mY <= (it->y + it->height))
			{
				
				while (index < history.size() - 1)
				{
					history.pop_back();
				}

				index++;

				if (msg != NULL) free(msg);

				history.push_back(it->uri);

				tempMsg = bw.connection(it->uri);

				msg = (WCHAR*)malloc(sizeof(WCHAR) * tempMsg.length());

				memset(msg, '\0', lstrlenW(msg));

				getCode(tempMsg);

				RECT rect;

				GetClientRect(mainhWnd, &rect);

				InvalidateRect(mainhWnd, &rect, TRUE);

				break;
			}
		}
		return 0;
	case WM_VSCROLL:
		yInc = 0;
		switch (LOWORD(wParam))
		{
		case SB_LINEUP:
			yInc = -10;
			break;
		case SB_LINEDOWN:
			yInc = 10;
			break;
		case SB_PAGEUP:
			yInc = -20;
			break;
		case SB_PAGEDOWN:
			yInc = 20;
		case SB_THUMBTRACK:
			yInc = HIWORD(wParam) - yPos;
			break;
		}
		if (yPos + yInc < 0) yInc = -yPos;

		if (yPos + yInc > yMax) yInc = yMax - yPos;

		yPos = yPos + yInc;

		ScrollWindow(mainhWnd, 0, -yInc, NULL, NULL);
		SetScrollPos(mainhWnd, SB_VERT, yPos, TRUE);

		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (enterFlag == 1)
		{
 			bw.tagParsing(msg, hdc, hWnd, yPos); //메시지 넘겨줘서 태그 파싱 
			if (reFlag == 0) //앞으로 가기, 뒤로 가기로 접속하면 다시 그려주기
			{
				InvalidateRect(hWnd, NULL, TRUE);
				reFlag = 1;
			}
		}
		EndPaint(hWnd, &ps);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_BACK: //뒤로가기

			if (index > 0)
			{
				index--; //인덱스 감소

				WCHAR url[MAXLEN] = { '\0', };

				lstrcpyW(url, history.at(index).c_str()); //인덱스에 있는 uri 추출
			
				
				memset(msg, '\0', lstrlenW(msg));

				tempMsg = bw.connection(url);


				msg = (WCHAR*)malloc(sizeof(WCHAR) * tempMsg.length());

				getCode(tempMsg);

				enterFlag = 1;

				RECT rect;

				GetClientRect(mainhWnd, &rect);

				InvalidateRect(mainhWnd, &rect, TRUE);
			}

			break;
		case IDM_FRONT: //앞으로가기

			if (index < history.size() - 1)
			{
				WCHAR url[MAXLEN] = { '\0', };

				index++; //인덱스 증가
				
				lstrcpyW(url, history.at(index).c_str()); //인덱스에 있는 uri 추출

				memset(msg, '\0', lstrlenW(msg));

				tempMsg = bw.connection(url);

				msg = (WCHAR*)malloc(sizeof(WCHAR) * tempMsg.length());

				getCode(tempMsg);

				enterFlag = 1;

				RECT rect;

				GetClientRect(mainhWnd, &rect);

				InvalidateRect(mainhWnd, &rect, TRUE);
			}
			break;
		case IDM_FAV:
			if (buf != NULL) fav.push_back(buf);
			DialogBox(g_hIsnt, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, reinterpret_cast<DLGPROC>(listSubProc1));
			break;
		case IDM_OPT:
			break;
		}
		return 0;
	}
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK EditSubProc1(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	wstring tempMsg = TEXT("");

	switch (iMessage)
	{
	case WM_KEYDOWN:

		if (wParam == VK_RETURN)
		{

			while (index < history.size() - 1)
			{
				history.pop_back();
			}

			index++;

			HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
			SetClassLongPtr(mainhWnd, GCLP_HBRBACKGROUND, (LONG)brush);


			GetWindowText(hEdit, buf, sizeof(buf));

			if (msg != NULL) free(msg);

			history.push_back(buf);

			tempMsg = bw.connection(buf);

 			msg = (WCHAR*)malloc(sizeof(WCHAR) * tempMsg.length());

			getCode(tempMsg);

			enterFlag = 1;

			reFlag = 0;

			RECT rect;

			GetClientRect(mainhWnd, &rect);

			InvalidateRect(mainhWnd, &rect, TRUE);
		}
		return 0;
	}
	return CallWindowProc(OldEditProc1, hWnd, iMessage, wParam, lParam);

}

LRESULT CALLBACK listSubProc1(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	int iPos = 0;
	int itemNumber = 0;
	int colWidth = 450;
	wstring tempMsg = TEXT("");
	list<wstring>::iterator it;
	RECT rect = { 0, };

	LVCOLUMN cItem = { 0, };
	LVITEM rItem = { 0, };
	switch (iMessage)
	{
	case WM_INITDIALOG:

		listhWnd = GetDlgItem(hWnd, IDC_LIST1);

		cItem.mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
		cItem.fmt = LVCFMT_LEFT;
		cItem.cx = 300;
		cItem.pszText = TEXT(" ");
		cItem.iSubItem = 0;
		SendMessage(listhWnd, LVM_INSERTCOLUMN, (WPARAM)0, (LPARAM)&cItem);

		fav.reverse();
		for (it = fav.begin(); it != fav.end(); it++)
		{
			rItem.mask = LVIF_TEXT;
			rItem.iItem = 0;
			rItem.iSubItem = 0;
			rItem.pszText = (LPWSTR)(it)->c_str();//(LPSTR)(it)->c_str();
			SendMessage(listhWnd, LVM_INSERTITEM, (WPARAM)itemNumber, (LPARAM)&rItem);
			itemNumber++;
		}
		fav.reverse();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:

			iPos = ListView_GetNextItem(listhWnd, -1, LVNI_SELECTED);
			
			while (iPos != -1)
			{
				for (it = fav.begin(); it != fav.end(); it++)
				{
					if (i == iPos)
					{
						WCHAR buf[MAXLEN] = { '\0', };
						lstrcpyW(buf, it->c_str());

						if (msg != NULL) free(msg);

						history.push_back(buf);

						tempMsg = bw.connection(buf);

						msg = (WCHAR*)malloc(sizeof(WCHAR) * tempMsg.length());

						getCode(tempMsg);

						enterFlag = 1;

						break;

					}
					i++;
				}
				break;
			}
			GetClientRect(mainhWnd, &rect);
			InvalidateRect(mainhWnd, &rect, TRUE);
			EndDialog(hWnd, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hWnd, TRUE);
			break;
		}
	}
	return CallWindowProc(OldEditProc1, hWnd, iMessage, wParam, lParam);

}

void getCode(wstring tempMsg)
{
	WCHAR* p = NULL;
	WCHAR* buffer = NULL;

	buffer = (WCHAR*)malloc(sizeof(WCHAR) * tempMsg.length());

	lstrcpyW(buffer, tempMsg.c_str());
	
	p = wcstok(buffer, TEXT("\r\n\r\n"));




	if (p != NULL)
	{
		WCHAR code[MAXLEN] = { '\0', };
		lstrcpyW(code, bw.headerParsing(p));
		
		if (lstrcmpW(code, TEXT("200")) == 0)
		{
			lstrcpyW(buffer, tempMsg.c_str());
	
			p = wcsstr(buffer, TEXT("\r\n\r\n"));
			if (p != NULL)
			{
				lstrcpyW(msg, p);
			}
			
		}
		else if (lstrcmpW(code, TEXT("404")) == 0)
		{
			lstrcpyW(msg, TEXT("HTTP/1.1 404 Not Found"));
		}
		else if (lstrcmpW(code, TEXT("400")) == 0)
		{
			lstrcpyW(msg, TEXT("HTTP/1.1 400 Bad Request"));
		}
		else
		{
			lstrcpyW(msg, TEXT("HTTP/1.1 400 Bad Request"));
		}
	}
	else
	{
		lstrcpyW(msg, TEXT("HTTP/1.1 400 Bad Request"));
	}
}

void createToolBar()
{
	TBBUTTON toolBtn[] = {
		{ 0, IDM_BACK, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0 },
		{ 1, IDM_FRONT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 1 },
		{ 4,0,0,TBSTYLE_SEP,0,0 },
		{ 2, IDM_FAV, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 2 },
		{ 3, IDM_OPT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 3 },
		{ 4,0,0,TBSTYLE_SEP,0,0 },
		{ 5, IDM_PLUS, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 4 },
	};

	TCHAR *szToolText = TEXT("뒤로\0앞으로\0즐겨찾기\0옵션\0추가\0");

	toolhWnd = CreateToolbarEx(mainhWnd, WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT,
		99, 4, g_hIsnt, IDR_TOOLBAR1, toolBtn, 7, 18, 15, 18, 15, sizeof(TBBUTTON));

	SendMessage(toolhWnd, TB_ADDSTRING, NULL, (LPARAM)szToolText);
	SendMessage(toolhWnd, TB_AUTOSIZE, (WPARAM)4, (LPARAM)toolBtn);
}

void initializeWindow(HINSTANCE hInstance, WNDCLASS& WndClass, LPCTSTR& lpszClass)
{
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);
}

void initializeVar()
{
	HINSTANCE g_hIsnt = NULL;
	hEdit = NULL; //에디트박스(URL)
	mainhWnd = NULL; //메인 윈도우
	toolhWnd = NULL;
	OldEditProc1 = NULL;
	enterFlag = 0;
	reFlag = 0;
}