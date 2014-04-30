// LogMeOutCmd.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <iostream>
#include <math.h>

#pragma comment(lib, "winmm.lib")

#define FREQUENCY     440                 // 440hz = Musical A Note
#define BUFFERSIZE    4860                // 4k sound buffer

#define PI            3.14159265358979

// A simple way to report Sound Errors
void Message(LPCSTR message)
{
	printf(message);
}
//-------------------------------------------------------------------------------------------
void Play()
{
    HWAVEOUT     hWaveOut;          // Handle to sound card output
    WAVEFORMATEX WaveFormat;        // The sound format
    WAVEHDR      WaveHeader;        // WAVE header for our sound data
          
    char         Data[BUFFERSIZE];  // Sound data buffer
          
    HANDLE       Done;              // Event Handle that tells us the sound has finished being played.
            // This is a real efficient way to put the program to sleep
            // while the sound card is processing the sound buffer
    double x;
    int i;
          
    // ** Initialize the sound format we will request from sound card **    
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;     // Uncompressed sound format
    WaveFormat.nChannels = 1;                    // 1=Mono 2=Stereo
    WaveFormat.wBitsPerSample = 8;               // Bits per sample per channel
    WaveFormat.nSamplesPerSec = 11025;           // Sample Per Second
    WaveFormat.nBlockAlign = WaveFormat.nChannels * WaveFormat.wBitsPerSample / 8;
    WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;    
    WaveFormat.cbSize = 0;
          
    // ** Create our "Sound is Done" event **
    Done = CreateEvent (0, FALSE, FALSE, 0);
          
    // ** Open the audio device **
    if (waveOutOpen(&hWaveOut,0,&WaveFormat,(DWORD) Done,0,CALLBACK_EVENT) != MMSYSERR_NOERROR) 
	{        
		Message("Sound card cannot be opened.");
		return ;
	}
          
    // ** Make the sound buffer **    
    for (i=0; i < BUFFERSIZE; i++)
    {        
      // ** Generate the sound wave based on FREQUENCY define
      // ** x will have a range of -1 to +1
      x = sin(i*2.0*PI*(FREQUENCY)/(double)WaveFormat.nSamplesPerSec); 
          
      // ** scale x to a range of 0-255 (signed char) for 8 bit sound reproduction **
      Data[i] = (char)(127*x+128);
    }
          
          
    // ** Create the wave header for our sound buffer **
    WaveHeader.lpData=Data;
    WaveHeader.dwBufferLength=BUFFERSIZE;
    WaveHeader.dwFlags=0;
    WaveHeader.dwLoops=0;
          
    // ** Prepare the header for playback on sound card **
	if (waveOutPrepareHeader(hWaveOut,&WaveHeader,sizeof(WaveHeader)) != MMSYSERR_NOERROR)
	{
		Message("Error preparing Header!");
		return ;
	}
          
    // ** Play the sound! **
    ResetEvent(Done);  // Reset our Event so it is non-signaled, it will be signaled again with buffer finished
          
    if (waveOutWrite(hWaveOut,&WaveHeader,sizeof(WaveHeader)) != MMSYSERR_NOERROR)
    {
        Message("Error writing to sound card!");
		return;
    }
          
    // ** Wait until sound finishes playing
    if (WaitForSingleObject(Done,INFINITE) != WAIT_OBJECT_0)
    {
		Message("Error waiting for sound to finish");
		return;
    }  
          
    // ** Unprepare our wav header **
    if (waveOutUnprepareHeader(hWaveOut,&WaveHeader,sizeof(WaveHeader)) != MMSYSERR_NOERROR)
    {
		Message("Error unpreparing header!");
		return;
    }
          
    // ** close the wav device **
    if (waveOutClose(hWaveOut) != MMSYSERR_NOERROR)
    {
		Message("Sound card cannot be closed!");
		return;
    } 
          
    // ** Release our event handle **
    CloseHandle(Done);
}
//-------------------------------------------------------------------------------------------
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
		
		const int MIN = 30;
		while (now + MIN * 60 * 1000 > GetTickCount())
		{
			static int sec = 0;
			static bool bPlay = true;

			++sec;
			if (sec % 10 == 0)
				std::cout << "Counter: " << sec << std::endl;
			
			
			if ((now + (MIN - 1) * 60 * 1000 < GetTickCount()) && bPlay)
			{
				Play();
				bPlay = false;
			}
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

