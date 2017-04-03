/*
#=====================================================================================================
#
#						Author : - R.RAVI CHANDRA REDDY
#=====================================================================================================
*/
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <direct.h>
#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <io.h>
#include <tchar.h>
#include <strsafe.h>
#include <WinBase.h>
#include <WinSock2.h>
#include <winsock.h>
#include <ws2tcpip.h> 
#include <strsafe.h>
#include <iphlpapi.h>

using namespace std;
// USER CHANGEABLE
//#define NETINTERFACE "Local Area Connection"
#define NETINTERFACE "wifi"
#define SOCKETTOBIND (5555)


//================debug compile switch
#define DEBUG_MODE

//================socket libraries
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

//================MACROS
#define BUFSIZE (4096)
#define ARGSIZE (64)
//=================pipe handles
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;
HANDLE g_hInputFile = NULL;

//===============protoype functions
void	CreateChildProcess(void); 
void	WriteToPipe(char *PtrRcvdBuf);
int		ReadFromPipe(char *PtrTxBuf);
void	ErrorExit(PTSTR); 


//============variables
int		numbytes = 0;
char	gblIpAddress[BUFSIZ];
char	recvbuf[BUFSIZE];
char	txbuf[BUFSIZE];
int ivar = 0;
char lengths[16] = { 26, 78, 11, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
char variables[16][120] = {
	"11\n",
	"22\n",
	"33\n",
	"aa\n",
	"bb\n",
	"cc\n",
	"dd\n",
	"ee\n",
	"ff\n",
	"gg\n",
	"hh\n",
	"ii\n" };

/*******************
*
*
*
*
********************/
int print_adapter(PIP_ADAPTER_ADDRESSES aa)
{
	int retVal = 0;
	char buf[BUFSIZ];

	memset(buf, 0, BUFSIZ);
	WideCharToMultiByte(CP_ACP, 0, aa->FriendlyName, wcslen(aa->FriendlyName), buf, BUFSIZ, NULL, NULL);
	printf("adapter_name:%s\n", buf);
	//if (!strcmp(&buf[0], "Local Area Connection"))
	if (!strcmp(&buf[0], NETINTERFACE))
	{
		retVal = 1;
	}
	return retVal;
}

/*******************
*
*
*
*
********************/
void print_addr(PIP_ADAPTER_UNICAST_ADDRESS ua)
{
	char buf[BUFSIZ];

	int family = ua->Address.lpSockaddr->sa_family;
	printf("\t%s ", family == AF_INET ? "IPv4" : "IPv6");

	memset(buf, 0, BUFSIZ);
	getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
	printf("%s\n", buf);
	
	if (family == AF_INET)
	{
		strncpy_s((char *)&gblIpAddress[0], 512, buf, 16);
	}
}

/*******************
*
*
*
*
********************/
void  parse(char *line, char *argc, char **argv)
{
	if ((*line == '\0'))
	{
		*argv++ = line;				/* save the argument position     */
	}
	while ((*line != '\0'))
	{
		/* if not the end of line ....... */
		while ( (*line == ' ') || (*line == '\t') || (*line == '\n'))
		{
			*line++ = '\0';			/* replace white spaces with 0    */
		}
		if (*line == '\0')
		{
			break;
		}
		*argv++ = line;				/* save the argument position     */
		(*argc)++;
		/* if this is a valid character */
		while (*line != '\0' && /**line != ' ' &&*/ *line != '\t' && *line != '\n')
		{
			line++;					/* skip the argument until ...    */
		}
	}
	*argv = '\0';                 /* mark the end of argument list  */
}

/*******************
*
*
*
*
********************/
bool print_ipaddress()
{
	DWORD rv, size;
	PIP_ADAPTER_ADDRESSES adapter_addresses, aa;
	PIP_ADAPTER_UNICAST_ADDRESS ua;
	int retVal = 0;

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size);
	if (rv != ERROR_BUFFER_OVERFLOW) 
	{
		fprintf(stderr, "GetAdaptersAddresses() failed...");
		return false;
	}
	adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(size);

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &size);
	if (rv != ERROR_SUCCESS) 
	{
		fprintf(stderr, "GetAdaptersAddresses() failed...");
		free(adapter_addresses);
		return false;
	}

	for (aa = adapter_addresses; aa != NULL; aa = aa->Next) 
	{
		if (retVal = (print_adapter(aa)) )
		{
			for (ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next)
			{
				print_addr(ua);
			}
			break;
		}
	}

	free(adapter_addresses);
	return true;
}

/*******************
*
*
*
*
********************/
void afterwrite(void)
{
	CHAR chBuf[BUFSIZE];
	DWORD dwRead = 0, dwWritten;
	BOOL bSuccess = FALSE;

	//===========SLEEP ==================
	Sleep(1000);
	//================ STD OUT FLUSH=========
#if 0
	ZeroMemory(&chBuf, sizeof(chBuf));
	dwRead = 0;

	sprintf_s(chBuf, "sys.stdout.flush()\n\r");
	dwRead = 20;

	bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
	if (!bSuccess)
	{
		printf("2 \r\n");
	}
#endif
	//================ STD IN FLUSH==========
#if 0
	ZeroMemory(&chBuf, sizeof(chBuf));
	dwRead = 0;

	sprintf_s(chBuf, "sys.stdin.flush()\n\r");
	dwRead = 19;

	bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
	if (!bSuccess)
	{
		printf("3 \r\n");
	}
#endif
}

void setUnbufferedMode(void)
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZ];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD bytesAvail = 0;

	ZeroMemory(&chBuf, sizeof(chBuf));
	dwRead = 0;

	sprintf_s(chBuf, 51, "do(\"C:/Python27/Python_stdin_stdout_unbuffer.py\")\n");
	dwRead = 51;

	//dwRead = strlen(largv[0]);

	// write the path for execution on IPython window
	bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
	if (!bSuccess)
	{
		printf("write failed !! \r\n");
	}
	//FlushFileBuffers(g_hChildStd_IN_Wr);

	Sleep(1000);

	// mandatory write of std in and out flush
	afterwrite();
}
/*******************
*
*
*
*
********************/
int _tmain(int argc, TCHAR *argv[]) 
{ 
	////=================Socket creation and network stuff================//
	SOCKET ListenSocket;
	struct sockaddr_in saServer,si_other;
	//WSAData wsaData;
	WSAData d;
	int slen = sizeof(si_other);
	int recv_len = 0;
	struct timeval tv;

	ZeroMemory(&gblIpAddress, sizeof(gblIpAddress));
	ZeroMemory(&txbuf, sizeof(txbuf));
	ZeroMemory(&recvbuf, sizeof(recvbuf));
	ZeroMemory(&saServer, sizeof(saServer));
	ZeroMemory(&si_other, sizeof(si_other));

	printf("\r\n Initialising Winsock...\r\n");

	if (WSAStartup(MAKEWORD(2, 2), &d) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised \r\n");
	
	//printing Local Area Connection IPV4 and IPV6 address. This step is to establish 
	//connection only on Local Area Connection or rather LAN
	print_ipaddress();
	
	// Create a socket
	ListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created \r\n");

	// Set up the sockaddr structure
	saServer.sin_family = AF_INET;
	inet_pton(AF_INET, (PCSTR)&gblIpAddress[0], &(saServer.sin_addr));
	saServer.sin_port = htons(SOCKETTOBIND);

	// Bind the listening socket using the
	// information in the sockaddr structure
	if (bind(ListenSocket, (SOCKADDR*)&saServer, sizeof(saServer)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Application binded on UDP port %d on %s interface \r\n", SOCKETTOBIND, NETINTERFACE);

	//=================Process creation and Pipes ===================//
	SECURITY_ATTRIBUTES saAttr;
	printf("\n->Start of parent process \n");

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, BUFSIZE))
	{
		ErrorExit(TEXT("StdoutRd CreatePipe"));
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
	{
		ErrorExit(TEXT("Stdout SetHandleInformation"));
	}

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
	{
		ErrorExit(TEXT("Stdin CreatePipe"));
	}

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
	{
		ErrorExit(TEXT("Stdin SetHandleInformation"));
	}


	// Create the child process. 
	CreateChildProcess();

	//setUnbufferedMode();

	if ((si_other.sin_family == 0) && (si_other.sin_port == 0))
	{
		//clear the buffer by filling null, it might have previously received data
		ZeroMemory(&recvbuf, sizeof(recvbuf));

		printf("Waiting for data from Client Server... \r\n");
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(ListenSocket, recvbuf, 1500, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			//exit(EXIT_FAILURE);
		}
		printf("received data %s",recvbuf);
	}
	// Write to the pipe that is the standard input for a child process. 
	// Data is written to the pipe's buffers, so it is not necessary to wait
	// until the child process is running before writing data.
	numbytes = ReadFromPipe(&txbuf[0]);
	if (numbytes > 0)
	{
		if (sendto(ListenSocket, txbuf, numbytes, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}

	tv.tv_sec = 1000;  /* 3 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	setsockopt(ListenSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

	//keep listening for data
	while (1)
	{

		//Sleep(1000);
		ZeroMemory(&txbuf, sizeof(txbuf));
		numbytes = ReadFromPipe(&txbuf[0]);
		if (numbytes > 0)
		{
			if (sendto(ListenSocket, txbuf, numbytes, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
		}

		ZeroMemory(&recvbuf, sizeof(recvbuf));
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(ListenSocket, recvbuf, 1500, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			//printf("recvfrom() failed with error code : %d \r\n", WSAGetLastError());
			//exit(EXIT_FAILURE);
		}
		else
		{
			printf("rcvd %s \r\n", recvbuf);
			//Sleep(1000);
			// Read from pipe that is the standard output for child process. 
			WriteToPipe(&recvbuf[0]);
		}

		//print details of the client/peer and the data received
		//printf("Data: %s\n", recvbuf);

		//
		//Sleep(1000);
		ZeroMemory(&txbuf, sizeof(txbuf));
		numbytes = ReadFromPipe(&txbuf[0]);
		if (numbytes > 0)
		{
			if (sendto(ListenSocket, txbuf, numbytes, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
		}
	}


	// The remaining open handles are cleaned up when this process terminates. 
	// To avoid resource leaks in a larger application, close handles explicitly. 
	//cleaning up earlier operation
	WSACleanup();
	return 0; 
} 

/*******************
*
*
*
*
********************/
void CreateChildProcess()
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	LPSTR szCmdLine = "C:\\Windows\\System32\\cmd.exe";

	bSuccess = CreateProcess(NULL,
	szCmdLine,     // command line 
	NULL,          // process security attributes 
	NULL,          // primary thread security attributes 
	TRUE,          // handles are inherited 
	NULL,             // creation flags 
	NULL,          // use parent's environment 
	"C:\\",          // use parent's current directory 
	&siStartInfo,  // STARTUPINFO pointer 
	&piProcInfo);  // receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if (!bSuccess)
	{
		ErrorExit(TEXT("CreateProcess"));
	}
	else
	{
		printf("child process created successfully \r\n");
	}
}

/*******************
*
*
*
*
********************/
void init_argc_argv(char *argc, char **argv)
{
	int indx = 0;
	for (indx = 0; indx < (*argc); indx++)
	{
		argv[indx] = NULL;
	}
	*argc = 0;
}


/*******************
*
*
*
*
********************/
void WriteToPipe(char *PtrRcvdBuf)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{
	DWORD dwRead = 0, dwWritten;
	DWORD dwRead1 = 0;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	char	largc = 0;
	char	*largv[ARGSIZE];
	int		loopIndx = 0;
	int		bandClass = 0;
	int		bandWidth = 0;
	int		chapter = 0;
	
	ZeroMemory(&chBuf, sizeof(chBuf));
	
	//===========F FLUSH=============
	//fflush(stdin);
	//fflush(stdout);
	//=======PIPE WRITE================

	init_argc_argv(&largc, largv);
	//======parsing the command, arguments are verfied for correctness
	parse(PtrRcvdBuf, &largc, largv);

	if (largv[0] == NULL)
	{
		return;
	}

	if (strcmp(largv[0], "echo") == 0)
	{
		printf("echo received \r\n");
	}
	else if (strcmp(largv[0], "inittest") == 0)
	{


	}
	else if ( (strcmp(largv[0], "exitserver") == 0) || (strcmp(largv[0], "quitserver") == 0) )
	{
		ErrorExit(TEXT("EXIT Command issued"));
		exit(EXIT_SUCCESS);
	}
	else 
	{
		// zeroing the buffers 
		ZeroMemory(&chBuf, sizeof(chBuf));

		/* Assuming that the Command issued is relevant, we run the relevant issued
		command as it is */
		sprintf_s(chBuf, largv[0]);
		dwRead = strlen(largv[0]);
		chBuf[dwRead++] = '\n';
	}
	//fflush(stdin);
	//fflush(stdout);
	bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
	if (!bSuccess)
	{
		printf("write failed !!! \r\n");
	}
	//Sleep(1000);

	afterwrite();

	//FlushFileBuffers(g_hChildStd_IN_Wr);
	//fflush(stdin);
	//fflush(stdout);
}


/*******************
*
*
*
*
********************/
// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
int ReadFromPipe(char *PtrTxBuf)
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZ];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD bytesAvail = 0;
	
	//=======F-FLUSH stdin stdout ==========
	//fflush(stdin);
	//fflush(stdout);
	//================ STD OUT FLUSH=========
#if 0
	ZeroMemory(&chBuf, sizeof(chBuf));
	dwRead = 0;

	sprintf_s(chBuf,"sys.stdout.flush()\n\r");
	dwRead = 20;

	bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
	if (!bSuccess)
	{
		printf("ReadFrompipe error, std_out_flush \r\n");
	}

#endif
	//============ SLEEP 250 ================
	Sleep(250);
	//==========PIPE WRITE================
	ZeroMemory(PtrTxBuf, sizeof(txbuf));
	dwRead = 0;

	PeekNamedPipe(g_hChildStd_OUT_Rd, NULL, 0, NULL, &bytesAvail, NULL);
	if (bytesAvail > 0)
	{
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, PtrTxBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0)
		{
			printf("read from child out pipe unsuccessfull \r\n");
		}
		//printf("\r\n bytesAvail %d \r\n", bytesAvail);

		#ifdef DEBUG_MODE
		bSuccess = WriteFile(hParentStdOut, PtrTxBuf, dwRead, &dwWritten, NULL);
			if (!bSuccess)
			{
				printf("cmd write error \r\n");
			}
		#endif
	}
	//=================== STD IN FLUSH=========
#if 0
	ZeroMemory(&chBuf, sizeof(chBuf));
	dwRead = 0;

	sprintf_s(chBuf, "sys.stdin.flush()\n\r");
	dwRead = 19;

	bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
	if (!bSuccess)
	{
		printf("ReadFrompipe error, std_in_flush \r\n");
	}
#endif
	return ((int)bytesAvail);
}

/*******************
*
*
*
*
********************/
// Format a readable error message, display a message box, 
// and exit from the application.
void ErrorExit(PTSTR lpszFunction)
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)*sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}




