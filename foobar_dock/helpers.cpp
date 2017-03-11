#include "foo_dock.h"


void ShowLastError(LPTSTR lpszFunction)
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
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 

    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

GUID GetFooDockGUID(int seed)
{
	GUID conf_guid;

	unsigned char Data4[8] = "FooDock";
	conf_guid.Data1 = 0x12141396;
	conf_guid.Data2 = 0x9631;
	conf_guid.Data3 = seed;
	memcpy(&conf_guid.Data4, Data4, 8);

	return conf_guid;
}

GUID GetFooQueueStopGUID(int seed)
{
	GUID conf_guid;

	unsigned char Data4[8] = "AKQueue";
	conf_guid.Data1 = 0x12141526;
	conf_guid.Data2 = 0x9319;
	conf_guid.Data3 = seed;
	memcpy(&conf_guid.Data4, Data4, 8);

	return conf_guid;
}