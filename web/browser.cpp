#include"browser.h"

browser::browser()
{
	memset(buf, '\0', MAXLEN);
	memset(rbuf, '\0', MAXLEN);
	memset(ip, '\0', MAXLEN);
	memset(port, '\0', MAXLEN);
	memset(uri, '\0', MAXLEN);
	x = 0;
	y = 100;
	hlinkFlag = 0;
	flag = false;
	host = NULL;
}

WCHAR* browser::connection(WCHAR* URL)
{
	initialize();
	wstring msg = TEXT("");
	char* mBuf = NULL;
	char mrBuf[MAXLEN] = { '\0', };

	parsingURL(URL); //URL�� �Ľ��Ѵ�.
	


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		return TEXT("-1");
	}


	/*�߸��� ������ �� �ٽ� ���� �Է¹ޱ�*/
	DWORD sock_timeout = 1 * 1000;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)sock_timeout, sizeof(sock_timeout));

	if (host != NULL) //DNS�϶� (host�� DNS�� �ּҸ� �Է¹��� ����)
	{
		if (lstrlenW(uri) != 0)
		{
			wsprintfW(buf, TEXT("GET /%s HTTP/1.1\r\n\r\nAccept:text/html, application/xhtml+xml, */*\r\nAccept-Language: ko-KR\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko\r\nAccept-Encoding: gzip, deflate\r\nHost: %s:80\r\nDNT:1\r\nConnection: Keep-Alive\r\n"), uri, inet_ntoa(*((struct in_addr *)host->h_addr_list[0])));
		}
		else
		{
			wsprintfW(buf, TEXT("GET / HTTP/1.1\r\n\r\nAccept:text/html, application/xhtml+xml, */*\r\nAccept-Language: ko-KR\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko\r\nAccept-Encoding: gzip, deflate\r\nHost: %s:80\r\nDNT:1\r\nConnection: Keep-Alive\r\n"), inet_ntoa(*((struct in_addr *)host->h_addr_list[0])));
		}
		memset((void*)&addr, 0x00, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(*((struct in_addr *)host->h_addr_list[0])));

		char* mPort = NULL;
		mPort = (char*)malloc(sizeof(char) * lstrlenW(port) + 1);
		memset(mPort, '\0', lstrlenW(port) + 1);
		mPort = convertUnicodeToMultibyte(port);

		addr.sin_port = htons(atoi(mPort));
		

	}
	else //DNS�� �ƴ� ��
	{
		if (ip != NULL && port != NULL)
		{
			memset((void*)&addr, 0x00, sizeof(addr));
			wsprintfW(buf, TEXT("GET /%s HTTP/1.1\r\nAccept:text/html, application/xhtml+xml, */*\r\nAccept-Language: ko-KR\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko\r\nAccept-Encoding: gzip, deflate\r\nHost: 52.192.132.151:8777\r\nDNT:1\r\nConnection: Keep-Alive\r\n"), uri);
			addr.sin_family = AF_INET;

			char* mIp = NULL;
			mIp = (char*)malloc(sizeof(char) * lstrlenW(ip) + 1);
			memset(mIp, '\0', lstrlenW(ip) + 1);
			mIp = convertUnicodeToMultibyte(ip);

			addr.sin_addr.S_un.S_addr = inet_addr(mIp);

			char* mPort = NULL;
			mPort = (char*)malloc(sizeof(char) * lstrlenW(port) + 1);
			memset(mIp, '\0', lstrlenW(port) + 1);
			mPort = convertUnicodeToMultibyte(port);

			addr.sin_port = htons(atoi(mPort));
		}
	}

	if (connect(sockfd, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		return TEXT("-1");
	}

	

	mBuf = (char*)malloc(sizeof(char) * lstrlenW(buf));
	mBuf = convertUnicodeToMultibyte(buf);

	send(sockfd, mBuf, strlen(mBuf), 0);

	memset(mrBuf, '\0', MAXLEN);

	int ret = 0;
	int i = 0;

	printf("here is conneciton\n");

	while (recv(sockfd, mrBuf, MAXLEN, 0) > 0)
	{
		printf("%s\n", mrBuf);
		msg += convertMultibyteToUnicode(mrBuf);//rbuf;
		
		if (strstr(mrBuf, "</html>"))
		{
			break;
		}
		memset(rbuf, '\0', MAXLEN);
		memset(mrBuf, '\0', MAXLEN);
	}
	printf("here is end of connection\n");

	WCHAR* returnMsg = (WCHAR*)malloc(sizeof(WCHAR) * msg.length());

	memset(returnMsg, '\0', sizeof(WCHAR) * msg.length());
	
	wcscpy(returnMsg, msg.c_str());

	closesocket(sockfd);


	return returnMsg;
}

void browser::parsingURL(WCHAR* message)
{
	initialize();
	WCHAR* p = NULL;
	char* mtempURL = NULL;

	lstrcpyW(buf, message);
	
	//http �Ľ�
	p = wcsstr(buf, TEXT("http://"));

	if ((p = wcsstr(buf, TEXT("http://"))) != NULL || (p = wcsstr(buf, TEXT("https://"))) != NULL) //http�� https�� �����ϸ�
	{
		lstrcpyW(buf, buf + lstrlenW(TEXT("http://")));

		p = NULL;

		//�������̸�
		p = wcsstr(buf, TEXT("www"));
		if (p != NULL)
		{
			WCHAR tempURL[MAXLEN] = { '\0', };

			lstrcpyW(tempURL, p);


			p = wcstok(p, TEXT(":"));

			lstrcpyW(tempURL, p);

			
			mtempURL = (char*)malloc(sizeof(char) * lstrlenW(tempURL) + 1);
			memset(mtempURL, '\0', lstrlenW(tempURL) + 1);
			mtempURL = convertUnicodeToMultibyte(tempURL);

			p = wcstok(NULL, TEXT("\r\n"));
			if (p == NULL) // ��Ʈ�� ���� ��
			{
				unsigned long hostaddr = inet_addr(mtempURL);

				if (hostaddr == INADDR_NONE)
				{
					host = gethostbyname(mtempURL);
				}
				lstrcpyW(port, TEXT("80")); //well known port(DNS)
			}
			else //��Ʈ�� ���� ��
			{
				unsigned long hostaddr = inet_addr(mtempURL);

				if (hostaddr == INADDR_NONE)
				{
					host = gethostbyname(mtempURL);
				}


				if (p != NULL)
				{
					lstrcpyW(port, p);
				}

			}
		}
		else //�������� �ƴϸ�
		{
			
			p = wcstok(buf, TEXT(":"));
			//ip
			lstrcpyW(ip, p);
			p = wcstok(NULL, TEXT("/"));
			//port
			lstrcpyW(port, p);
			//uri
			p = wcstok(NULL, TEXT(" "));

			lstrcpyW(uri, p);
		}
	}
	else//http�� ��������������
	{
		//www.~~~�̸�
		p = wcsstr(buf, TEXT("www"));
		if (p != NULL)
		{
			WCHAR tempURL[MAXLEN] = { '\0', };

			lstrcpyW(tempURL, p);

			p = wcstok(p, TEXT(":"));

			lstrcpyW(tempURL, p);

			mtempURL = (char*)malloc(sizeof(char) * lstrlenW(tempURL) + 1);
			memset(mtempURL, '\0', lstrlenW(tempURL) + 1);
			mtempURL = convertUnicodeToMultibyte(tempURL);



			p = wcstok(NULL, TEXT("\r\n"));
			if (p == NULL) // ��Ʈ�� ���� ��
			{

				unsigned long hostaddr = inet_addr(mtempURL);

				if (hostaddr == INADDR_NONE)
				{
					host = gethostbyname(mtempURL);
				}
				lstrcpyW(port, TEXT("80"));

			}
			else //��Ʈ�� ���� ��
			{

				unsigned long hostaddr = inet_addr(mtempURL);

				if (hostaddr == INADDR_NONE)
				{
					host = gethostbyname(mtempURL);
				}

				if (p != NULL)
				{
					lstrcpyW(port, p);
				}
			}
		}
		else
		{
			p = wcstok(buf, TEXT(":"));
			//ip
			if (p != NULL)
				lstrcpyW(ip, p);
			p = wcstok(NULL, TEXT("/"));
			//port
			if (p != NULL)
				lstrcpyW(port, p);
			//uri
			p = wcstok(NULL, TEXT(" "));
			if (p != NULL)
				lstrcpyW(uri, p);
		}
	}
}

WCHAR* browser::headerParsing(WCHAR* message)
{
	/*��� �߿� code�� �ĺ��Ͽ� � �������� ����ڿ��� ����ϱ� ���� parsing*/
	WCHAR buf[MAXLEN] = { '\0', };
	WCHAR* p = NULL;

	lstrcpyW(buf, message);

	p = wcstok(buf, TEXT(" "));
	if (p != NULL)
	{
		p = wcstok(NULL, TEXT(" "));

		if (p != NULL)
		{

			if (lstrcmpW(p, TEXT("200")) == 0) //200 ok
			{
				return TEXT("200");
			}
			else if (lstrcmpW(p, TEXT("404")) == 0) //404 not found
			{
				return TEXT("404");
			}
			else if (lstrcmpW(p, TEXT("400")) == 0) //400 bad request
			{
				return TEXT("400");
			}
			else
			{
				return TEXT("-1");
			}

		}
		return TEXT("-1");
	}

	return TEXT("-1");
}

void browser::tagParsing(WCHAR* message, HDC hdc, HWND mainhWnd, int yPos)
{
	WCHAR tempIp[MAXLEN] = { '\0', };
	WCHAR tempPort[MAXLEN] = { '\0', };

	if (ip != NULL && port != NULL)
	{
		lstrcpyW(tempIp, ip);
		lstrcpyW(tempPort, port);
	}
	initialize();
	if (ip != NULL && port != NULL)
	{
		lstrcpyW(ip, tempIp);
		lstrcpyW(port, tempPort);
	}

	unsigned int i = 0;
	int tagFlag = 0; //tag������ �˸�
	int outTagFlag = 0; //�±��� ���Ḧ �˸��� ����
	int headerFlag = 0;
	int tempBodyFlag = 0; //body�ȿ��� tag�� ������ �˸�
	int bodyFlag = 0; //body�� ������ �˸�
	int fontHeight = 0;
	int styleFlag = 0;
	
	int htempX = 0;
	int htempY = 0;

	WCHAR htempUri[MAXLEN] = { '\0', };
	WCHAR* tempMessage = NULL;
	wstring fileName = TEXT("");
	wstring tempTag = TEXT(""); //tag���� ����
	wstring body = TEXT(""); //tag������ ���� ���
	wstring hTag[] = { TEXT("<h1>"), TEXT("<h2>"), TEXT("<h3>"), TEXT("<h4>"), TEXT("<h5>"), TEXT("<h6>") };
	RECT rect = {0, };
	HFONT hFont = 0, oldFont = 0;

	GetClientRect(mainhWnd, &rect);

	x = rect.left;
	y = rect.top;

	y += 100 - yPos;



	//�޽�����ŭ �迭 ����
	tempMessage = (WCHAR*)malloc(sizeof(WCHAR) * (lstrlenW(message) + 1));

	memset(tempMessage, '\0', (lstrlenW(message) + 1));

	lstrcpyW(tempMessage, message);

	//������ ��� 
	if (lstrcmpW(tempMessage, TEXT("HTTP/1.1 404 Not Found")) == 0)
	{
		TextOut(hdc, x, y, tempMessage, lstrlenW(tempMessage));
		return;
	}
	else if (lstrcmpW(tempMessage, TEXT("HTTP/1.1 400 Bad Request")) == 0)
	{
		TextOut(hdc, x, y, tempMessage, lstrlenW(tempMessage));
		return;
	}
	
	while (1)
	{
		//body�� ���������� ���� ����
		if (tempMessage[i] == '<' && bodyFlag == 1)
		{
			tempTag += tempMessage[i];
			i++;
			tagFlag = 1;
			continue;
		}

		if (tempMessage[i] == '>' && bodyFlag == 1)
		{
			tempTag += tempMessage[i];
			if (outTagFlag == 0) // </~>�� �ƴҶ�
			{
				/////////////////////////////////////////////////////////////////////
				const WCHAR* p = NULL;
				p = wcsstr(tempTag.c_str(), TEXT("img src"));
				/* �̹��� ���� �̸� ã��*/
				if (p != NULL)
				{
					//������ ������ �� �� �� �ٵ� ���� ���
					
					TextOut(hdc, x, y, body.c_str(), body.length());

					x += (body.length() * 10);

					int pivot = 0;
					for (int j = 0; tempTag[j] != NULL; j++)
					{
						if (tempTag[j] != '\"' && pivot == 1)
						{
							fileName += tempTag[j];
						}
						if (tempTag[j] == '\"' && pivot == 0)
						{
							j++;
							fileName += tempTag[j];
							pivot = 1;
						}
						else if (tempTag[j] == '\"' && pivot == 1)
						{
							break;
						}
					}
					//���� ó�� �Լ�
					fileProcess(fileName, hdc);

					//���� �ʱ�ȭ
					fileName = TEXT("");
					body = TEXT("");
					tempTag = TEXT("");
					tagFlag = 0;
					i++;
					continue;
				}
				/////////////////////////////////////////////////////////////////
				

				////////////////////////////////////////////////////////////////
				p = wcsstr(tempTag.c_str(), TEXT("form action"));
				if (p != NULL) //form action �� ��
				{
					WCHAR formTag[MAXLEN] = { '\0', };
					lstrcpyW(formTag, tempTag.c_str());
					p = wcstok(formTag, TEXT("\""));
					p = wcstok(NULL, TEXT("\""));
					printf("%s\n", p); //����
					p = wcstok(NULL, TEXT("\""));
					p = wcstok(NULL, TEXT("\""));
					printf("%s\n", p); //�޽���
				}
				////////////////////////////////////////////////////////////////

				////////////////////////////////////////////////////////////////

				p = wcsstr(tempTag.c_str(), TEXT("input type"));
				if (p != NULL)
				{
					
					TextOut(hdc, x, y, body.c_str(), body.length());

					WCHAR inputTag[MAXLEN] = { '\0', };
					lstrcpyW(inputTag, tempTag.c_str());
					p = wcstok(inputTag, TEXT("="));
					p = wcstok(NULL, TEXT(" "));//edit
					p = wcstok(NULL, TEXT("\""));
					p = wcstok(NULL, TEXT("\""));//name
				}


				//////////////////////////////////////////////////////


				p = wcsstr(tempTag.c_str(), TEXT("a href"));
				if (p != NULL)
				{
					TextOut(hdc, x, y, body.c_str(), body.length());

					x += (body.length() * 10);

					WCHAR inputTag[MAXLEN] = { '\0', };
					lstrcpyW(inputTag, tempTag.c_str());
					p = wcstok(inputTag, TEXT("\""));
					p = wcstok(NULL, TEXT("\""));

					htempX = x;
					htempY = y;					
					wcscpy(htempUri, p);

					tagStack.push(TEXT("<a>"));
				}

				//////////////////////////////////////////////
				//h~�±��϶� h Ȯ��

				
				for (int j = 0; j < sizeof(hTag) / sizeof(hTag[0]); j++)
				{
					if (lstrcmpW(tempTag.c_str(), hTag[j].c_str()) == 0) //h�±׶� �Ǵ�
					{
						switch (j)
						{
						case 0:
							fontHeight = 40;
							break;
						case 1:
							fontHeight = 35;
							break;
						case 2:
							fontHeight = 30;
							break;
						case 3:
							fontHeight = 25;
							break;
						case 4:
							fontHeight = 20;
							break;
						case 5:
							fontHeight = 15;
							break;
						}
						hFont = CreateFont(fontHeight, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0,
							VARIABLE_PITCH | FF_ROMAN, TEXT("Impect"));
						oldFont = (HFONT)SelectObject(hdc, hFont);

						tagStack.push(tempTag);
					}
				}
				
				//<br>, <h~>, ...��� �� �� ����
				if (lstrcmpW(tempTag.c_str(), TEXT("<br>")) == 0)
				{
					TextOut(hdc, x, y, body.c_str(), body.length());
					x = rect.left;
					y += 30;
				}
				
				body = TEXT("");
				tempTag = TEXT("");
				outTagFlag = 0;
				tagFlag = 0;
				i++;
				continue;

			}
			else//���γ��� �±װ� end tag�� ��
			{
				//</br>, </body> �� �� ����
				if (lstrcmpW(tempTag.c_str(), TEXT("<br>")) == 0)
				{
					
					TextOut(hdc, x, y, body.c_str(), body.length());
					y += 30;
					tempTag = TEXT("");
					body = TEXT("");
					outTagFlag = 0;
					i++;
					continue;
				}
				///////////////////////////////////////////////////////

				if (lstrcmpW(tempTag.c_str(), TEXT("<p>")) == 0)
				{
					TextOut(hdc, x, y, body.c_str(), body.length());
					y += 30;
					tempTag = TEXT("");
					body = TEXT("");
					outTagFlag = 0;
					i++;
					continue;
				}


				if (!tagStack.empty())
				{
					if (lstrcmpW(tagStack.top().c_str(), tempTag.c_str()) == 0)
					{
						//body¦�� ��
						if (lstrcmpW(tempTag.c_str(), TEXT("<body>")) == 0)
						{
							TextOut(hdc, x, y, body.c_str(), body.length());
							bodyFlag = 0;
							tagStack.pop();
						}
						//h~ ¦�� ��
						for (int j = 0; j < sizeof(hTag) / sizeof(hTag[0]); j++)
						{
							if (lstrcmpW(tempTag.c_str(), hTag[j].c_str()) == 0) //h�±׶� �Ǵ�
							{
								TextOut(hdc, x, y, body.c_str(), body.length());
								y += fontHeight;
								fontHeight = 0;
								SelectObject(hdc, oldFont);
								DeleteObject(hFont);
								tagStack.pop();
							}
						}

						if (lstrcmpW(tempTag.c_str(), TEXT("<a>")) == 0)
						{
							SetTextColor(hdc, RGB(0, 0, 255));
							TextOut(hdc, x, y, body.c_str(), body.length());

							struct hyperLink tempLink = { 0, };
							tempLink.x = htempX;
							tempLink.y = htempY;
							tempLink.width = body.length()*20;
							tempLink.height = 20;
							
							wsprintfW(tempLink.uri,TEXT("%s:%s/%s"),ip, port, htempUri);

							linker.push_back(tempLink);

							htempX = 0;
							htempY = 0;
							memset(htempUri, '\0', MAXLEN);

							x += (body.length() * 15);
							SetTextColor(hdc, RGB(0, 0, 0));
							tagStack.pop();
						}
					}
					else// ¦�� �� �¾Ƽ� ���� 
					{
						break;
					}
				}
			}

			tempTag = TEXT("");
			tagFlag = 0;
			body = TEXT("");
			outTagFlag = 0;
			i++;
			continue;
		}
		
		/////////////////////////////////////////////////

		if (tempMessage[i] == '<' && bodyFlag == 0)
		{
			tagFlag = 1;
			tempTag += tempMessage[i];
			i++;
			continue;
		}
		if (tempMessage[i] == '>' && bodyFlag == 0)
		{
			tempTag += tempMessage[i];

			if (outTagFlag == 0)
			{

				if (lstrcmpW(tempTag.c_str(), TEXT("<body>")) == 0) //body���� �Ǻ�
				{
					bodyFlag = 1;
					tagStack.push(tempTag);
				}
				else if (lstrcmpW(tempTag.c_str(), TEXT("<center>")) == 0) //center���� �Ǻ�
				{
					x = (rect.top + rect.right) / 2;
					tagStack.push(tempTag);
				}
				else if (lstrcmpW(tempTag.c_str(), TEXT("<title>")) == 0)
				{
					//no action
					tagStack.push(tempTag);
				}
				else if (lstrcmpW(tempTag.c_str(), TEXT("<style>")) == 0)
				{
					//no action
					tagStack.push(tempTag);
				}
				else if (lstrcmpW(tempTag.c_str(), TEXT("<span>")) == 0)
				{
					//no action
					tagStack.push(tempTag);
				}
				else if (lstrcmpW(tempTag.c_str(), TEXT("<br>")) == 0)
				{
					y += 30;
					x = rect.left;
				}
				else if (lstrcmpW(tempTag.c_str(), TEXT("<li>")) == 0)
				{
					tagStack.push(tempTag);
				}
				else 
				{
					tagStack.push(tempTag);
				}
				

			}
			else //������ Ȯ�� 
			{
				if (!tagStack.empty())
				{
					if (lstrcmpW(tagStack.top().c_str(), tempTag.c_str()) == 0)
					{

						if (lstrcmpW(tagStack.top().c_str(), TEXT("<title>")) == 0) //title ¦
						{
							SetWindowTextW(mainhWnd, body.c_str()); //Ÿ��Ʋ�� �������� window Ÿ��Ʋ ����
							tagStack.pop();
						}
						else if (lstrcmpW(tagStack.top().c_str(), TEXT("<center>")) == 0) //center ¦
						{
							x = rect.top;
							//SetTextAlign(hdc, TA_LEFT);
							tagStack.pop();
						}
						else if (lstrcmpW(tagStack.top().c_str(), TEXT("<style>")) == 0)
						{
							styleParsing(body, mainhWnd);
							tagStack.pop();
						}
						else if (lstrcmpW(tagStack.top().c_str(), TEXT("<span>")) == 0)
						{
							TextOut(hdc, x, y, body.c_str(), body.length());
							x += (body.length() * 16);
							tagStack.pop();

						}
						else if (lstrcmpW(tagStack.top().c_str(), TEXT("<li>")) == 0)
						{
							TextOut(hdc, x, y, body.c_str(), body.length());
							x = rect.left;
							y += 30;
							tagStack.pop();
						}else if (lstrcmpW(tagStack.top().c_str(), TEXT("<html>")) == 0)
						{
							break;
						}
					}
				}
			}

			tempTag = TEXT("");
			body = TEXT("");
			tagFlag = 0;
			outTagFlag = 0;
			i++;
			continue;
		}
		if (tagFlag == 0) //tag�� �ƴϸ� body ������ ��� ���� ����
		{
			body += tempMessage[i];
			i++;
			continue;
		}
		if (tagFlag == 1) //�±� ���� ����
		{
			if (tempMessage[i] == '/')
			{
				tagFlag = 1;
				outTagFlag = 1; //�� �±�
				i++;
				continue;
			}
			if (tempMessage[i] >= 'A' && tempMessage[i] <= 'Z') //�ҹ��ڷ� �����
			{
				tempTag += tempMessage[i] + 32;
			}
			else
			{
				tempTag += tempMessage[i];
			}
			i++;
			continue;
		}
		////////////////////////////////////////////////////
		i++;
	}
}

void browser::styleParsing(wstring message, HWND mainhWnd)
{

	//style�� ������ �� �Ľ��ϴ� �Լ�
	WCHAR* tempMessage = NULL;
	wstring msgBody = TEXT("");
	int i = 0;
	int bodyFlag = 0;
	tempMessage = (WCHAR*)malloc(sizeof(WCHAR) * message.length());


	lstrcpyW(tempMessage, message.c_str());

	while (i <= message.length())
	{


		if (bodyFlag == 1 && tempMessage[i] != '}') //body�ٹ̱� �϶�
		{
			msgBody += tempMessage[i];
			i++;
			continue;
		}



		if (tempMessage[i] == '{') //���� ����
		{
			const WCHAR *p = NULL;
			if ((p = wcsstr(msgBody.c_str(), TEXT("body"))) != NULL)
			{
				msgBody = TEXT("");
				bodyFlag = 1;
				i++;
				continue;
			}
		}
		else if (tempMessage[i] == '}') //���� ���� ���� ������ ��
		{
			
			const WCHAR *p = NULL;
			if ((p = wcsstr(msgBody.c_str(), TEXT("background-color"))) != NULL) //���� ��׶��带 ������
			{
				
				p = wcsstr(msgBody.c_str(), TEXT("#")); //#���� �����ϴ� ���ȭ��
				if (p != NULL)
				{
					HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));
					SetClassLongPtr(mainhWnd, GCLP_HBRBACKGROUND, (LONG)brush);	
				}
			}


			i++;
			bodyFlag = 0;
			continue;
		}

		msgBody += tempMessage[i];
		i++;
	}

}

void browser::fileProcess(wstring fileName, HDC hdc)
{
	FILE *checkFile = NULL;
	string mFileName = "";
	char* mFileBuf = NULL;
	char mrBuf[MAXLEN] = { '\0', };
	mFileName.assign(fileName.begin(), fileName.end());

	printf("here is fileProcess\n");

	if ((checkFile = fopen(mFileName.c_str(), "r")) == NULL) //������ ������ ���� ����
	{

		WCHAR filebuf[MAXLEN] = { '\0', };

		//���� �̸��� �����Ͽ� ������Ʈ ��û
		wsprintfW(filebuf, TEXT("GET /%s HTTP/1.1\r\nAccept:text/html, application/xhtml+xml, */*\r\nAccept-Language: ko-KR\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko\r\nAccept-Encoding: gzip, deflate\r\nHost: 52.192.132.151:8777:80\r\nDNT:1\r\nConnection: Keep-Alive\r\n"), fileName.c_str());

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		{
			return;
		}

		if (connect(sockfd, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
		{
			return;
		}

		mFileBuf = (char*)malloc(sizeof(char) * lstrlenW(filebuf) + 1);
		memset(mFileBuf, '\0', lstrlenW(filebuf) + 1);
		mFileBuf = convertUnicodeToMultibyte(filebuf);

		send(sockfd, mFileBuf, lstrlenW(filebuf), 0);

		FILE* save = fopen(mFileName.c_str(), "wb");

		int ret = 0;
		int filelength = 0;
		int headerFlag = 0;
		int mimeFlag = 0;
		int i = 0;

		while (1)
		{
			ret = recv(sockfd, mrBuf, 1024, 0);
			if (ret > 0)
			{
				i++;
				if (i == 2) //������� ����
				{
					const char* p = NULL;

					p = strstr(mrBuf, "\r\n\r\n");

					fwrite(p + strlen("\r\n\r\n"), ret - (p - mrBuf) - strlen("\r\n\r\n"), 1, save); //\r\n\r\n �� �ķ� ���� ����
					memset(mrBuf, '\0', 1024);

					continue;
				}

				if (i >= 3)
				{
					fwrite(mrBuf, ret, 1, save);
				}
				memset(mrBuf, '\0', 1024);
			}
			else
			{
				break;
			}
		}





		/*
		while (1)
		{
			ret = recv(sockfd, mrBuf, 1024, 0);
			if (ret > 0)
			{
				if (mimeFlag == 0) //������� ����
				{
					const char* p = NULL;

					p = strstr(mrBuf, "\r\n\r\n");

					printf("%s\n", p);

					fwrite(p + strlen("\r\n\r\n"), ret - (p - mrBuf) - strlen("\r\n\r\n"), 1, save); //\r\n\r\n �� �ķ� ���� ����
					
					mimeFlag = 1;
					memset(mrBuf, '\0', 1024);

					continue;
				}

				if (mimeFlag == 1)
				{
					fwrite(mrBuf, ret, 1, save);
				}
				memset(mrBuf, '\0', 1024);
			}
			else
			{
				break;
			}
		}
		*/
		fclose(save);
		closesocket(sockfd);
	}
	
	wstring fileExt = fileName.substr(fileName.find('.') + 1, fileName.length());

	if (fileExt == TEXT("jpg")) //���� Ȯ���ڰ� jpg�̸�
	{
		if(checkFile != NULL)
			fclose(checkFile);
		jpgShow(fileName, hdc);
	}
	else if (fileExt == TEXT("bmp")) //bmp�̸�
	{
		if (checkFile != NULL)
			fclose(checkFile);
		bmpShow(fileName, hdc);
	}

}

void browser::jpgShow(wstring fileName, HDC hdc)
{
	/*jpg�� ���*/
	WCHAR FN[MAXLEN] = { '\0', };

	for (int j = 0; j < fileName.length(); j++)
	{
		FN[j] = fileName[j];
	}

	Graphics graphics(hdc);

	Image image(FN);

	graphics.DrawImage(&image, x, y, image.GetWidth(), image.GetHeight());
	
	x += image.GetWidth();

	graphics.ReleaseHDC(hdc);
}

void browser::bmpShow(wstring fileName, HDC hdc)
{
	/*��Ʈ���� ���*/
	HANDLE hBmp = LoadImage(NULL, fileName.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	if (hBmp == NULL)
		return;

	HDC dcmem = CreateCompatibleDC(NULL);

	if (SelectObject(dcmem, hBmp) == NULL)
	{
		DeleteDC(dcmem);
		return;
	}

	BITMAP bm = { 0, };
	GetObject(hBmp, sizeof(bm), &bm);
	
	if (BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, dcmem,
		0, 0, SRCCOPY) == 0)
	{	// failed the blit
		DeleteDC(dcmem);
		x += bm.bmWidth;
		return;
	}
	x += bm.bmWidth;
}

void browser::initialize()
{
	/*class ������� �ʱ�ȭ*/
	memset(buf, '\0', MAXLEN);
	memset(rbuf, '\0', MAXLEN);
	memset(ip, '\0', MAXLEN);
	memset(port, '\0', MAXLEN);
	memset(uri, '\0', MAXLEN);

	x = 0;
	y = 100;
	hlinkFlag = 0;

	flag = false;
	host = NULL;

	asset.clear();

	while (!tagStack.empty())
	{
		tagStack.pop();
	}
}

char* browser::convertUnicodeToMultibyte(WCHAR* strUnicode)
{
	int nLen = WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, NULL, 0, NULL, NULL);

	char * pMultibyte = new char[nLen];

	memset(pMultibyte, 0x00, nLen*sizeof(char));

	WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, pMultibyte, nLen, NULL, NULL);

	return pMultibyte;
}

CString browser::convertMultibyteToUnicode(char* pMultibyte)
{
	int nLen = strlen(pMultibyte);

	WCHAR *pWideChar = new WCHAR[nLen];
	
	for (int i = 0; i < nLen; i++)
	{
		pWideChar[i] = '\0';
	}
	
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pMultibyte, -1, pWideChar, nLen);

	CString strUnicde = "";
	strUnicde.Format(_T("%s"), pWideChar);

	delete[] pWideChar;

	return strUnicde;
}
	