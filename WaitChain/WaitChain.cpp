// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.


// WaitChain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

extern	_TCHAR* g_szProcName;
extern	BOOL	g_fProcName;
extern	BOOL	g_fProcID;
extern	BOOL	g_fDump;
extern	DWORD	g_dwProcID;
extern	TCHAR	g_OutputFileName[MAX_PATH];

int _tmain(int argc, _TCHAR **argv)
{
	// Check the arguments if they are errornous then exit else do the necessary processing
	if (!ProcessArguments(argc, argv))
		return 0;

	// Prepare the report file and add the initial lines into it
	// If the WAITCHAIN Log file cannot be created then inform the user and exit
	if (!OutputSetFile())
	{
		_tprintf(_TEXT("Cannot create the WAITCHAIN log file!\n"));
		_tprintf(_TEXT("Please execute the application in a folder where the user has file read & write permission.\n"));
		_tprintf(_TEXT("Exiting WaitChain.exe\n"));
		return 5;
	}

	OutputDoHeader(_T("PROGRESS INFORMATION"));
	GetProcessInfo();
	PutTime(TRUE);

	// Execute the main logic
	GetWaitInformation(g_fProcID, g_dwProcID, g_fProcName, g_szProcName, g_fDump);

	// If either process name or process ID is specified then WaitChain will limit it's analysis to those processes
	// However, the user might want to have a look at the list of the running processes at that time in the system
	// Hence, provide a list of running processes
	if (g_fProcID || g_fProcName)
		DisplayRunningProcessInformation();

	// Finish the report and close it
	OutputDoHeader(_T("PROGRESS INFORMATION"));
	PutTime(FALSE);
	OutputCloseFile();

	// inform the user that the WaitChain is done and notify the name of the output file
	_tprintf(_TEXT("\nOutput is available in %s\n"), g_OutputFileName);

	return 0;
}
