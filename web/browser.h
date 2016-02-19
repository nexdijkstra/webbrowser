#pragma once
#include<Windows.h>
#include<iostream>
#include<stdio.h>
#include<string.h>
#include<string>
#include<vector>
#include<stack>
#include<fstream>
#include<GdiPlus.h>
#include<atlstr.h>

#define MAXLEN 8192

#pragma comment(lib, "gdiplus")
#pragma warning(disable:4996)


using namespace Gdiplus;
using namespace std;

struct hyperLink
{
	int x;
	int y;
	int width;
	int height;
	WCHAR uri[MAXLEN];
};



class browser {
private:
	WCHAR buf[1024];
	WCHAR rbuf[MAXLEN];
	WCHAR ip[MAXLEN];
	WCHAR port[MAXLEN];
	WCHAR uri[MAXLEN];
	
	int x, y;
	int hlinkFlag;
	bool flag;

	SOCKET sockfd;
	HWND hWnd;
	HOSTENT* host;
	SOCKADDR_IN addr;
	HDC hdc;
	PAINTSTRUCT ps;

	
	vector<HWND> asset;
	stack<wstring> tagStack;

public:
	browser();

	vector<hyperLink> linker;

	WCHAR* connection(WCHAR* URL);
	void parsingURL(WCHAR* message);

	WCHAR* headerParsing(WCHAR* message);
	void tagParsing(WCHAR* message, HDC hdc, HWND mainhWnd, int yPos);
	void fileProcess(wstring fileName, HDC hdc);
	void jpgShow(wstring fileName, HDC hdc);
	void bmpShow(wstring fileName, HDC hdc);
	void styleParsing(wstring message, HWND mainhWnd);
	void initialize();
	char* convertUnicodeToMultibyte(WCHAR* strUnicode);
	CString convertMultibyteToUnicode(char* pMultibyte);
};