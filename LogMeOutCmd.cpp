// LogMeOutCmd.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <iostream>

int _tmain(int argc, _TCHAR* argv[])
{
	try 
	{
		// step 1: 
		HANDLE				hToken;     /* process token */
		TOKEN_PRIVILEGES	tp;			/* token provileges */
		TOKEN_PRIVILEGES	oldtp;		/* old token privileges */
		DWORD				dwSize = sizeof (TOKEN_PRIVILEGES);          
		LUID				luid;          

		/* now, set the SE_SYSTEMTIME_NAME privilege to our current
		*  process, so we can call SetSystemTime()
		*/
		if (!OpenProcessToken (GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		   &hToken))
		{
		   printf ("OpenProcessToken() failed with code %d\n", GetLastError());
		   return 1;
		}
		if (!LookupPrivilegeValue (NULL, SE_SYSTEMTIME_NAME, &luid))
		{
		   printf ("LookupPrivilege() failed with code %d\n", GetLastError());
		   CloseHandle (hToken);
		   return 1;
		}

		ZeroMemory (&tp, sizeof (tp));
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		/* Adjust Token privileges */
		if (!AdjustTokenPrivileges (hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), 
				 &oldtp, &dwSize))
		{
		   printf ("AdjustTokenPrivileges() failed with code %d\n", GetLastError());
		   CloseHandle (hToken);
		   return 1;
		}

		HWND hWnd = GetForegroundWindow();
		// disable the [x] button.
		if(hWnd != NULL)
		{
			HMENU hMenu = GetSystemMenu(hWnd, 0);
			if(hMenu != NULL)
			{
				DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
				DrawMenuBar(hWnd);
			}
		}

		/*disable CTRL+C*/
		if (!SetConsoleCtrlHandler(NULL, TRUE)) {
			printf("\nERROR: Could not set control handler");
			return 1;
		}

		/* wait  30 min. */
		DWORD now = GetTickCount();

		std::cout << "START!\n";

		while (now + 30 * 60 * 1000 > GetTickCount())
		{
			static int sec = 0;
			++sec;
			if (sec % 10 == 0)
				std::cout << "Counter: " << sec << std::endl;

			Sleep(1000);
		}

		std::cout << "STOP!\n";

		/* LogOut */
		if (ExitWindowsEx(EWX_LOGOFF | EWX_FORCEIFHUNG, 0) == FALSE)
		{
		   printf ("LookupPrivilege() failed with code %d\n", GetLastError());
		   CloseHandle (hToken);
		   return 1;
		}

		/* disable SE_SYSTEMTIME_NAME again */
		AdjustTokenPrivileges (hToken, FALSE, &oldtp, dwSize, NULL, NULL);
		if (GetLastError() != ERROR_SUCCESS)
		{
		   printf ("AdjustTokenPrivileges() failed with code %d\n", GetLastError());
		   CloseHandle (hToken);
		   return 1;
		}
		   
		CloseHandle (hToken);	
		return 0;
	}
	catch(const std::exception &e)
	{
		printf ("std::exception error: %s\n", e.what());
		return -1;
	}
}

