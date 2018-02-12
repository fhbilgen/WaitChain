// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.



#include "stdafx.h"

extern FILE	*g_FileStream;

std::list<DWORD> GetProcIDsFromProcName(_TCHAR* szProcName)
{
	
	DWORD i;
	DWORD processes[1024];
	DWORD numProcesses;
	errno_t err;
	_TCHAR* szLowerProcName;
	_TCHAR* szProcNameCopy;	
	//WCHAR file[MAX_PATH];
	_TCHAR szFileName[MAX_PATH];
	PCWSTR filePart;
	HANDLE hProc;
	std::list<DWORD> procIDs;
	

	// Get a list of all processes currently running.
	if (EnumProcesses(processes, sizeof(processes), &numProcesses) == FALSE)
	{	
		_ftprintf(g_FileStream, TEXT("Could not enumerate processes\n"));
		return procIDs;
	}
	
	szLowerProcName = _tcsdup(szProcName);
	err = _tcslwr_s(szLowerProcName, _tcslen(szLowerProcName)+1 );	

	for (i = 0; i < numProcesses / sizeof(DWORD); i++)
	{
		// Get a handle to this process.
		hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processes[i]);
		if (hProc)
		{
			// Retrieve the executable name and print it.
			if (GetProcessImageFileName(hProc, szFileName, ARRAYSIZE(szFileName)) > 0)
			{
				filePart = wcsrchr(szFileName, _T('\\'));
				if (filePart)
					filePart++;				
				else
					filePart = szFileName;
				
				szProcNameCopy = _tcsdup(filePart);
				err = _tcslwr_s(szProcNameCopy, _tcslen(szProcNameCopy)+1 );

				if ( _tcscmp(szProcNameCopy, szLowerProcName) == 0)
					procIDs.push_back(GetProcessId(hProc));

				free(szProcNameCopy);
			}	

			CloseHandle(hProc);
		}
	}

	free(szLowerProcName);
	return procIDs;
}


BOOL DisplayRunningProcessInformation()
{
	DWORD	processes[1024];
	DWORD	numProcesses;
	DWORD	i;
	HANDLE	hProcess;
	_TCHAR	szFileName[MAX_PATH];

	// Get a list of all processes currently running.
	if (EnumProcesses(processes, sizeof(processes), &numProcesses) == FALSE)
	{		
		_ftprintf(g_FileStream, TEXT("Could not enumerate processes\n"));
		return FALSE;
	}
	else
	{
		_ftprintf(g_FileStream, TEXT("RUNNING PROCESSES\n"));
		_ftprintf(g_FileStream, TEXT("=================\n\n"));
		_ftprintf(g_FileStream, TEXT("%+8s%8s\n\n"), _TEXT("PID(hex:dec)"), _TEXT("NAME") );
	}

	for (i = 0; i < numProcesses / sizeof(DWORD); i++)
	{
		// Get a handle to this process.
		hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processes[i]);
		if (hProcess)
		{				
			_ftprintf(g_FileStream, TEXT("%#x:0n%u\t"), processes[i], processes[i]);

			// Retrieve the executable name and print it.
			if (GetProcessImageFileName(hProcess, szFileName, ARRAYSIZE(szFileName)) > 0)
			{
				PCWSTR filePart = wcsrchr(szFileName, _T('\\'));
				if (filePart)
					filePart++;
				else
					filePart = szFileName;				

				_ftprintf(g_FileStream, TEXT("%s"), filePart);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("N/A"));
			}

			CloseHandle(hProcess);
			_ftprintf(g_FileStream, TEXT("\n"));
		}
	}

	return TRUE;

}