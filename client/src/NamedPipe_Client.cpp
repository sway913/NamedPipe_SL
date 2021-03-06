// NamedPipe_Client.cpp : Defines the entry point for the console application.
//

#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include <atlbase.h>
#include <atlconv.h>

#define BUFSIZE 512

int _tmain(int argc, TCHAR *argv[])
 {
	HANDLE hPipe;
	LPTSTR lpvMessage = LPTSTR(TEXT("Default message from client."));
	TCHAR  chBuf[BUFSIZE];
	BOOL   fSuccess = FALSE;
	DWORD  cbRead, cbToWrite, cbWritten, dwMode;
	LPTSTR lpszPipename = LPTSTR(TEXT("\\\\.\\pipe\\mynamedpipe"));

	if (argc > 1)
		lpvMessage = argv[1];

	// Try to open a named pipe; wait for it, if necessary. 

	while (1)
	{
		hPipe = CreateFile(
			lpszPipename,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

							// Break if the pipe handle is valid. 

		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			return -1;
		}

		// All pipe instances are busy, so wait for 20 seconds. 

		if (!WaitNamedPipe(lpszPipename, 20000))
		{
			printf("Could not open pipe: 20 second wait timed out.");
			return -1;
		}
	}

	// The pipe connected; change to message-read mode. 
	_tprintf(TEXT("Connection Established\n\n"));

	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess)
	{
		_tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
		return -1;
	}

	
	
	// Send a message to the pipe server. 
	while (1)
	{
		TCHAR messageToSend[256] = { 0, };
		_tprintf(TEXT("\n=====================================================\n"));
		_tprintf(TEXT("SaveData at the server   EX) save Title Contents\n"));
		_tprintf(TEXT("LoadData from the server EX) load Title\n"));
		_tprintf(TEXT("Send String              EX) anything\n"));
		_tprintf(TEXT("=====================================================\n"));
		lpvMessage = 0;
		scanf_s("%[^\n]s", messageToSend, sizeof(messageToSend));
		getchar();
		lpvMessage = (LPTSTR)(messageToSend);
		cbToWrite = (lstrlen(lpvMessage) + 1) * sizeof(TCHAR);
		_tprintf(TEXT("Sending %d byte message: \"%s\"\n"), cbToWrite, lpvMessage);

		fSuccess = WriteFile(
			hPipe,                  // pipe handle 
			lpvMessage,             // message 
			cbToWrite,              // message length 
			&cbWritten,             // bytes written 
			NULL);                  // not overlapped 

		if (!fSuccess)
		{
			_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
			return -1;
		}

		printf("\nfrom server:\n");

		do
		{
			// Read from the pipe. 

			fSuccess = ReadFile(
				hPipe,    // pipe handle 
				chBuf,    // buffer to receive reply 
				BUFSIZE * sizeof(TCHAR),  // size of buffer 
				&cbRead,  // number of bytes read 
				NULL);    // not overlapped 

			if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
				break;

			_tprintf(TEXT("\%s \n"), chBuf);
		} while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

		if (!fSuccess)
		{
			_tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
			return -1;
		}
	}

	printf("\n<End of message, press ENTER to terminate connection and exit>");
	_getch();

	CloseHandle(hPipe);

	return 0;
}