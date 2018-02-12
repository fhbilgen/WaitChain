// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.


#include "stdafx.h"

/******************************************** GLOBAL VARIABLES	********************************************

g_DumpProcCmdLine	:	The command line string that is passed to the CreatProcess API

*************************************************************************************************************/

_TCHAR g_DumpProcCmdLine[MAX_PATH] = _TEXT("procdump.exe  -accepteula -ma ");

extern FILE	*g_FileStream;


BOOL GenerateDumpFile(DWORD procid)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	DWORD dwWaitResult;
	_TCHAR DumpCmdLine[MAX_PATH];
	_TCHAR wszProcID[10];
	DWORD	lastError;


	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	_ltot_s(procid, wszProcID,10, 10);
	_tcscpy_s(DumpCmdLine, MAX_PATH, g_DumpProcCmdLine);
	_tcscat_s(DumpCmdLine, MAX_PATH, wszProcID);

	if (CreateProcess(NULL, DumpCmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		dwWaitResult = WaitForSingleObject(pi.hProcess, INFINITE);		
		_ftprintf(g_FileStream, _TEXT("Dump generated for (0x%x:0n%u)\n"), procid, procid);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{		
		lastError = GetLastError();
		_tprintf(_TEXT("procdump.exe could not be executed. Error=0x%x:\n"), lastError);
		_ftprintf(g_FileStream, _TEXT("procdump.exe could not be executed. Error=0x%x:\n"), lastError);
				
		if (lastError == ERROR_FILE_NOT_FOUND)
		{
			_tprintf(_TEXT("Please, make sure that procdump.exe is accessible\n"));
			_tprintf(_TEXT("ProcDump is available at https ://download.sysinternals.com/files/Procdump.zip\n"));
			_ftprintf(g_FileStream, _TEXT("Please, make sure that procdump.exe is accessible\n"));
			_ftprintf(g_FileStream, _TEXT("ProcDump is available at https ://download.sysinternals.com/files/Procdump.zip\n"));
		}

		return FALSE;
	}

	return TRUE;
}
