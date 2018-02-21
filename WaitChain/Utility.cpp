// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.



#include "stdafx.h"



/******************************************** GLOBAL VARIABLES	********************************************
 

 g_OutputFileName	:	The full path name of the report file that the WaitChain.exe tool generates
 g_FileStream		:	The global filestream pointer
 g_ProcessName		:	Process Name for which the process instances will be searched for
 g_PID				:	ProcessID of the WaitChain.exe process
 g_szProcSuffix		:	Process Name Suffix. i.e. .exe
 g_szProcName		:	Process Name for which the process instances will be searched for
 g_fProcName		:	Will WaitChain.exe work on process names ?
 g_fProcID			:	Will WaitChain.exe work on a specific Process ID ?
 g_fDump			:	Should WaitChain.exe generate memory dump files of the processes taking part in WaitChains
 g_dwProcID			:	The process ID for which WaitChain.exe will report the wait chain
 szStartString		:	The format string for report start time logging
 szFinishString		:	The format string for report end time logging
*************************************************************************************************************/

TCHAR	g_OutputFileName[MAX_PATH];
FILE*	g_FileStream;
TCHAR	g_ProcessName[MAX_PATH];
DWORD	g_PID;
_TCHAR* g_szProcSuffix = _TEXT(".exe");
_TCHAR* g_szProcName = NULL;
BOOL	g_fProcName = FALSE;
BOOL	g_fProcID = FALSE;
BOOL	g_fDump = FALSE;
DWORD	g_dwProcID = 0;
TCHAR*	szStartString = _TEXT("Started on   : %02d/%02d/%d  %02d:%02d:%02d\n\n");
TCHAR*	szFinishString = _TEXT("Finished on  : %02d/%02d/%d  %02d:%02d:%02d\n");

void GetProcessInfo(void)
{
	_tcscpy_s(g_ProcessName, MAX_PATH, GetCommandLine());
	g_PID = GetCurrentProcessId();

	_ftprintf(g_FileStream, _TEXT("Command line : \"%s\"\n"), g_ProcessName);
	_ftprintf(g_FileStream, _TEXT("PID          : (0x%X:0n%u)\n"), g_PID, g_PID);

	return;
}

void PutTime(BOOL fStart)
{
	SYSTEMTIME stLocal;

	GetLocalTime(&stLocal);

	if (fStart)
	{
		_ftprintf(g_FileStream, szStartString,
			stLocal.wMonth, stLocal.wDay, stLocal.wYear,
			stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
	}
	else
	{
		_ftprintf(g_FileStream, szFinishString,
			stLocal.wMonth, stLocal.wDay, stLocal.wYear,
			stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
	}
}

void OutputDoHeader(_TCHAR* szHeaderName)
{
	int i;
	
	_ftprintf(g_FileStream, _TEXT("\n%s\n"), szHeaderName);

	for ( i = 0; i != HEADER_WIDTH; i++ )
		_ftprintf(g_FileStream, _T("="));

	_ftprintf(g_FileStream, _T("\n"));
	
}

BOOL OutputSetFile(void)
{

	TCHAR	szCompName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD	cName = MAX_COMPUTERNAME_LENGTH;
	_TCHAR	szErrorMessage[MAX_PATH];
	errno_t err;
	
	// Build the file name
	_tcscpy_s( g_OutputFileName, MAX_PATH, OUTPUT_FILE_PREFIX );

	if ( 0 != GetComputerName( szCompName, &cName ) )
	{
		_tcscat_s(g_OutputFileName, MAX_PATH, szCompName);
		_tcscat_s(g_OutputFileName, MAX_PATH, OUTPUT_FILE_SUFFIX);
	}
	else
		// could not get the computer name therefore failing back to the default name
		_tcscat_s(g_OutputFileName, MAX_PATH, OUTPUT_FILE_SUFFIX);

	// Destroy any existing file and close it	
	err = _tfopen_s(&g_FileStream, g_OutputFileName, _TEXT("w+"));

	if (err != 0 || !g_FileStream)
	{		
		_tcserror_s(szErrorMessage, sizeof(_TCHAR)*MAX_PATH, err);		
		_tprintf(_TEXT("\nError opening output file.\n"));
		_tprintf(_TEXT("Error No:%d\t Error Message:%s\n"), err, szErrorMessage);
		return FALSE;
	}
	else
	{
		fflush(g_FileStream);
		fclose(g_FileStream);
	}

	// Finally open the file
	err = _tfopen_s(&g_FileStream, g_OutputFileName, _TEXT("a"));
	if (err != 0)
	{
		_tcserror_s(szErrorMessage, sizeof(_TCHAR)*MAX_PATH, err);
		_tprintf(_TEXT("Error opening output file.\n"));
		_tprintf(_TEXT("Error No:%d\t Error Message:%s\n"), err, szErrorMessage);
		return FALSE;
	}
	else
		return TRUE;

}

void OutputCloseFile( void )
{
	fclose(g_FileStream);
}

void DisplayDescription()
{
	_tprintf(_TEXT("\nWaitChain v1.0 Wait Chain traversal utility\n"));
	_tprintf(_TEXT("Reports the Wait Chain of a process.\n"));
	_tprintf(_TEXT("Optionally, generates memory dump files of the processes listed in the Wait Chain.\n"));
}

void DisplaySyntax()
{
	_tprintf(_TEXT("\nUsage:"));
	_tprintf(_TEXT("\nWaitChain.exe [ProcID | ProcName, [/d]] | [/?]\n"));
}

void DisplayUsage()
{	
	DisplaySyntax();

	_tprintf(_TEXT("\nOptions:"));
	_tprintf(_TEXT("\nProcID\t\tProcessID for which Wait Chain analysis will be made.\n"));
	_tprintf(_TEXT("\nProcName\tAll process instances with ProcName will be analysed."));
	_tprintf(_TEXT("\n\t\tIf neither ProcID nor ProcName is specified then Wait Chain Analysis report"));
	_tprintf(_TEXT("\n\t\tfor each running process will be generated and the /d option is discarded.\n"));
	_tprintf(_TEXT("\n/d\t\tThe memory dump file of each process that is in the Wait Chain will be generated.\n"));
	_tprintf(_TEXT("\n/?\t\tDisplays this help\n"));
	_tprintf(_TEXT("\nWaitChain.exe generates a text file report in the same folder where it runs."));
	_tprintf(_TEXT("\nThe file name format is WAITCHAIN_computername.LOG\n"));
}

BOOL ProcessArguments(int argc, _TCHAR **argv)
{
	_TCHAR* str; // = argv[1];
	BOOL fDigit = TRUE;
	
	// Get the wait chain information for all running processes
	if (argc == 1)
	{
		DisplayDescription();		
		// The defaults of the globals are enough to describe this case		
		return TRUE;
	}

	if (argc == 2)
	{
		// Is this help display request
		if (_tcscmp(argv[1], _TEXT("/?")) == 0)
		{
			DisplayDescription();
			DisplayUsage();
			return FALSE;
		}

		// Is the argument a number hence a proc id
		str = argv[1];
		for (size_t i = 0; str[i] != _T('\0'); ++i)
		{
			if (!(_istdigit(str[i])))
			{
				fDigit = FALSE;
				break;
			}
		}
		
		// WaitChain will process ProcID
		if (fDigit)
		{	
			g_fProcID = TRUE;
			g_dwProcID = _wtoi(argv[1]);
			DisplayDescription();
			return TRUE;
		}

		// Is the argument a process name ?
		if (NULL != _tcsstr(argv[1], g_szProcSuffix))
		{
			g_szProcName = (_TCHAR*)malloc(sizeof(_TCHAR) * (_tcslen(argv[1]) + 1));
			
			if (g_szProcName)
			{
				_tcscpy_s(g_szProcName, _tcslen(argv[1]) + 1, argv[1]);
				g_fProcName = TRUE;
				DisplayDescription();
				return TRUE;
			}
			return FALSE;
		}
	}

	// Processing two arguments
	if (argc == 3)
	{
		// If the second argument is not "/d" then exit
		if (_tcscmp(argv[2], _TEXT("/d")) != 0)
		{
			_tprintf(_TEXT("Invalid input!\n"));
			DisplaySyntax();
			return FALSE;
		}
		
		// Second argument is /d hence dumps should be generated
		g_fDump = TRUE;

		// Is the argument a number hence a proc id ?
		str = argv[1];
		for (size_t i = 0; str[i] != _T('\0'); ++i)
		{
			if (!(_istdigit(str[i])))
			{
				fDigit = FALSE;
				break;
			}
		}

		if (fDigit)
		{
			g_fProcID = TRUE;
			g_dwProcID = _wtoi(argv[1]);			
			DisplayDescription();
			return TRUE;
		}

		// Is the argument a process name ?
		if (NULL != _tcsstr(argv[1], g_szProcSuffix))
		{
			g_szProcName = (_TCHAR*)malloc(sizeof(_TCHAR) * (_tcslen(argv[1]) + 1));
			if (g_szProcName)
			{
				_tcscpy_s(g_szProcName, _tcslen(argv[1]) + 1, argv[1]);
				g_fProcName = TRUE;
				DisplayDescription();
				return TRUE;
			}
			return FALSE;
		}
	}

	// Anything else is improper input !!!
	_tprintf(_TEXT("Invalid input!\n"));
	DisplaySyntax();
	return FALSE;

}
