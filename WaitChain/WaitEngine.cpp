// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.


#include "stdafx.h"

/*********************************************************************************************************************************************************************

The Godot.exe tool is based on the "Wait Chain Traversal" API https://msdn.microsoft.com/en-us/library/windows/desktop/ms681622(v=vs.85).aspx

The core functionality of the tool is in the WaitEngine.h and WaitEngine.cpp files. And, the code is mainly based on the MSDN sample

Using WCT
https://msdn.microsoft.com/en-us/library/windows/desktop/ms681418(v=vs.85).aspx

*********************************************************************************************************************************************************************/

/******************************************** GLOBAL VARIABLES	********************************************

STR_OBJECT_TYPE	:	stores the names for the WCT_OBJECT_TYPE enumeration

STR_OBJ_STATUS	:	stores the names for the WCT_OBJECT_STATUS enumeration

g_WctHandle		:	Global variable to store the WCT session handle

g_Ole32Hnd		:	Global variable to store OLE32.DLL module handle.

*************************************************************************************************************/

extern FILE	*g_FileStream;

_TCHAR* STR_OBJECT_TYPE[] =
{
	_TEXT("CriticalSection"),
	_TEXT("SendMessage"),
	_TEXT("Mutex"),
	_TEXT("Alpc"),
	_TEXT("Com"),
	_TEXT("ThreadWait"),
	_TEXT("ProcWait"),
	_TEXT("Thread"),
	_TEXT("ComActivation"),
	_TEXT("Unknown"),
	_TEXT("Max")
};

_TCHAR* STR_OBJ_STATUS[] =
{
	_TEXT("NoAccess"),
	_TEXT("Running"),
	_TEXT("Blocked"),
	_TEXT("PidOnly"),
	_TEXT("PidOnlyRpcss"),
	_TEXT("Owned"),
	_TEXT("NotOwned"),
	_TEXT("Abandoned"),
	_TEXT("Unknown"),
	_TEXT("Error")
};


HWCT g_WctHandle = NULL;
HMODULE g_Ole32Hnd = NULL;


BOOL GrantDebugPrivilege()
{
	BOOL             fSuccess = FALSE;
	HANDLE           hToken = NULL;
	TOKEN_PRIVILEGES TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{	
		_ftprintf(g_FileStream, TEXT("Could not get the process token"));
		goto Cleanup;
	}

	TokenPrivileges.PrivilegeCount = 1;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &TokenPrivileges.Privileges[0].Luid))
	{		
		_ftprintf(g_FileStream, TEXT("Couldn't lookup SeDebugPrivilege name"));
		goto Cleanup;
	}

	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TokenPrivileges), NULL, NULL))
	{	
		_ftprintf(g_FileStream, TEXT("Could not revoke the debug privilege"));
		goto Cleanup;
	}

	fSuccess = TRUE;

Cleanup:

	if (hToken)
	{
		CloseHandle(hToken);
	}

	return fSuccess;
}

BOOL InitCOMAccess()
{
	PCOGETCALLSTATE       CallStateCallback;
	PCOGETACTIVATIONSTATE ActivationStateCallback;

	// Get a handle to OLE32.DLL. You must keep this handle around
	// for the life time for any WCT session.
	g_Ole32Hnd = LoadLibrary(_TEXT("ole32.dll"));
	if (!g_Ole32Hnd)
	{		
		_ftprintf(g_FileStream, TEXT("ERROR: GetModuleHandle failed: 0x%X\n"), GetLastError());
		return FALSE;
	}

	// Retrieve the function addresses for the COM helper APIs.
	CallStateCallback = (PCOGETCALLSTATE) GetProcAddress(g_Ole32Hnd, "CoGetCallState");
	if (!CallStateCallback)
	{		
		_ftprintf(g_FileStream, TEXT("ERROR: GetProcAddress failed: 0x%X\n"), GetLastError());
		return FALSE;
	}

	ActivationStateCallback = (PCOGETACTIVATIONSTATE)GetProcAddress(g_Ole32Hnd, "CoGetActivationState");
	if (!ActivationStateCallback)
	{
		_ftprintf(g_FileStream, TEXT("ERROR: GetProcAddress failed: 0x%X\n"), GetLastError());
		return FALSE;
	}

	// Register these functions with WCT.
	RegisterWaitChainCOMCallback(CallStateCallback, ActivationStateCallback);
	return TRUE;
}

BOOL CheckThreads( DWORD ProcId, BOOL fDump )
{
	DWORD	i;
	DWORD	processes[1024];
	DWORD	numProcesses;
	int		countOfProcesses = 0;	
	HANDLE	hThread;
	HANDLE	hProcess;
	HANDLE	hSnapshot;
	std::list<DWORD> currentRelevantProcesses, newRelevantProcesses;

	// Try to enable the SE_DEBUG_NAME privilege for this process. 
	// Continue even if this fails--we just won't be able to retrieve
	// wait chains for processes not owned by the current user.
	if (!GrantDebugPrivilege())
	{		
		_ftprintf(g_FileStream, TEXT("Could not enable the debug privilege\n"));
	}

	// Get a list of all processes currently running.
	if (EnumProcesses(processes, sizeof(processes), &numProcesses) == FALSE)
	{
		_ftprintf(g_FileStream, TEXT("Could not enumerate processes\n"));
		return FALSE;
	}

	// If all processes are going to be examined then display the count of processes
	countOfProcesses = numProcesses / sizeof(DWORD);

	if (ProcId==0)
		wprintf(_TEXT("\n%d processes to examine\n"), countOfProcesses);


	for (i = 0; i < numProcesses / sizeof(DWORD); i++)
	{

		// Show progress to the console
		if (ProcId == 0)
			wprintf(_TEXT("."));

		if (processes[i] == GetCurrentProcessId())
		{
			continue;
		}

		// If the caller specified a Process ID, check if we have a match.
		if (ProcId != 0)
		{
			if (processes[i] != ProcId)
			{
				continue;
			}
		}

		// Get a handle to this process.
		hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processes[i]);
		if (hProcess)
		{
			WCHAR file[MAX_PATH];

			_ftprintf(g_FileStream, TEXT("Process (%#x:0n%u) - "), processes[i], processes[i]);

			// Retrieve the executable name and print it.
			if (GetProcessImageFileName(hProcess, file, ARRAYSIZE(file)) > 0)
			{
				PCWSTR filePart = wcsrchr(file, L'\\');
				if (filePart)
				{
					filePart++;
				}
				else
				{
					filePart = file;
				}

				_ftprintf(g_FileStream, TEXT("%s"), filePart);				
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("N/A"));
			}
						
			_ftprintf(g_FileStream, TEXT("\n----------------------------------\n"));

			// Clear the relevant process information
			currentRelevantProcesses.clear();
			newRelevantProcesses.clear();

			// Get a snapshot of this process. This enables us to
			// enumerate its threads.
			hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processes[i]);
			if (hSnapshot)
			{
				THREADENTRY32 thread;
				thread.dwSize = sizeof(thread);

				// Walk the thread list and print each wait chain
				if (Thread32First(hSnapshot, &thread))
				{
					
					do
					{
						if (thread.th32OwnerProcessID == processes[i])
						{
							// Open a handle to this specific thread
							hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, thread.th32ThreadID);
							if (hThread)
							{
								// Check whether the thread is still running
								DWORD exitCode;
								GetExitCodeThread(hThread, &exitCode);

								if (exitCode == STILL_ACTIVE)
								{
									// Print the wait chain.
									newRelevantProcesses = PrintWaitChain(thread.th32ThreadID);
								}

								for (DWORD dw : newRelevantProcesses)
								{
									if (dw != processes[i])
									{
										BOOL found = FALSE;
										for (DWORD dwC : currentRelevantProcesses)
										{
											if (dw == dwC)
											{
												found = TRUE;
												break;
											}
										}

										if (!found)
											currentRelevantProcesses.push_back(dw);
									}
								}
								
								newRelevantProcesses.clear();

								CloseHandle(hThread);
							}
							// Usually openthread with all access fails with access denied for protected processes. If the process snapshot won't be available then the report will list the process with an empty threadlist.
							// If this situation needs to be investigated when access denied is not expected then this else part can be used
							
						}
					} while (Thread32Next(hSnapshot, &thread));
				}
				// The else part can be implemented for troubleshooting of Godot.exe to investigate why Thread32First fails. But having a snapshot and not be able to enumerating it is a slim possibility. 
				
				CloseHandle(hSnapshot);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("CreateToolhelp32Snapshot failed for (%#x:0n%u) with error=%#x\n"), processes[i], processes[i], GetLastError());
			}

						
			_ftprintf(g_FileStream, TEXT("\n"));

			// Add the current process to the relevant processes so that it will be dumped later on
			if (fDump)
				currentRelevantProcesses.push_back(ProcId);

			// Display the relevant processes which showed up in the wait chain and dump them if specified
			ProcessRelevantProcesses(currentRelevantProcesses, fDump);
			
			CloseHandle(hProcess);
		}
	}
	return TRUE;
}

std::list<DWORD> PrintWaitChain(DWORD ThreadId)
{
	WAITCHAIN_NODE_INFO NodeInfoArray[WCT_MAX_NODE_COUNT];
	DWORD               Count, i;
	BOOL                IsCycle;

	std::list<DWORD> RelatedProcesses;
		
	_ftprintf(g_FileStream, TEXT("%u: "), ThreadId);

	Count = WCT_MAX_NODE_COUNT;

	// Make a synchronous WCT call to retrieve the wait chain.
	if (!GetThreadWaitChain(g_WctHandle, NULL, WCTP_GETINFO_ALL_FLAGS, ThreadId, &Count, NodeInfoArray, &IsCycle))
	{		
		_ftprintf(g_FileStream, TEXT("GetThreadWaitChain failed with error (0X%x)\n"), GetLastError());
		return RelatedProcesses;
	}

	// Check if the wait chain is too big for the array we passed in.
	if (Count > WCT_MAX_NODE_COUNT)
	{		
		_ftprintf(g_FileStream, TEXT("Found additional nodes %u\n"), Count);
		Count = WCT_MAX_NODE_COUNT;
	}

	// Loop over all the nodes returned and print useful information.
		for (i = 0; i < Count; i++)
	{
		switch (NodeInfoArray[i].ObjectType)
		{
		case WctCriticalSectionType:
			// Some objects have names...
			if (NodeInfoArray[i].LockObject.ObjectName[0] != L'\0')
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1],
					NodeInfoArray[i].LockObject.ObjectName);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s]->"),					
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1]);
			}

			break;

		case WctSendMessageType:

			// Some objects have names...
			if (NodeInfoArray[i].LockObject.ObjectName[0] != L'\0')
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1],
					NodeInfoArray[i].LockObject.ObjectName);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1]);
			}

			break;

		case WctMutexType:
			// Some objects have names...
			if (NodeInfoArray[i].LockObject.ObjectName[0] != L'\0')
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1],
					NodeInfoArray[i].LockObject.ObjectName);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1]);
			}

			break;

		case WctComType:

			if (NodeInfoArray[i].LockObject.ObjectName[0] != L'\0')
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1],
					NodeInfoArray[i].LockObject.ObjectName);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1]);
			}

			break;


		case WctThreadWaitType:

			if (NodeInfoArray[i].LockObject.ObjectName[0] != L'\0')
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1],
					NodeInfoArray[i].LockObject.ObjectName);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1]);
			}

			break;

		case WctProcessWaitType:
			if (NodeInfoArray[i].LockObject.ObjectName[0] != L'\0')
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1],
					NodeInfoArray[i].LockObject.ObjectName);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1]);
			}

			break;

		case WctThreadType:
			// A thread node contains process and thread ID.
			_ftprintf(g_FileStream, TEXT("[%x:%x:%s]->"),
				NodeInfoArray[i].ThreadObject.ProcessId,
				NodeInfoArray[i].ThreadObject.ThreadId,
				STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1]);
			
			RelatedProcesses.push_back(NodeInfoArray[i].ThreadObject.ProcessId);

			break;

		case WctComActivationType:

			if (NodeInfoArray[i].LockObject.ObjectName[0] != L'\0')
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1],
					NodeInfoArray[i].LockObject.ObjectName);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("[%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					STR_OBJ_STATUS[NodeInfoArray[i].ObjectStatus - 1]);
			}

			break;

		default:
			// A synchronization object.

			// Some objects have names...
			if (NodeInfoArray[i].LockObject.ObjectName[0] != L'\0')
			{				
				_ftprintf(g_FileStream, TEXT("[%s:%s]->"),
					STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1],
					NodeInfoArray[i].LockObject.ObjectName);
			}
			else
			{
				_ftprintf(g_FileStream, TEXT("[%s]->"), STR_OBJECT_TYPE[NodeInfoArray[i].ObjectType - 1]);

			}
			
			if (NodeInfoArray[i].ObjectStatus == WctStatusAbandoned)
			{
				_ftprintf(g_FileStream, TEXT("<abandoned>"));
			}

			break;
		}
	}
		

	_ftprintf(g_FileStream, TEXT("[End]"));

	// Did we find a deadlock?
	if (IsCycle)
	{	
		_ftprintf(g_FileStream, TEXT(" !!!Deadlock!!!"));
	}
		
	_ftprintf(g_FileStream, TEXT("\n"));

	return RelatedProcesses;
}

void GetWaitInformation(BOOL fPID, DWORD dwPID, BOOL fPName, _TCHAR* szPName, BOOL fDump)
{
	std::list<DWORD> pList;

	// Initialize the WCT interface to COM. Continue if this
	// fails--there just will not be COM information.
	if (!InitCOMAccess())
	{		
		_ftprintf(g_FileStream, TEXT("Could not enable COM access\n"));
	}
	
	if (fPName)
	{
		pList = GetProcIDsFromProcName(szPName);
	}
	else
	{
		if (fPID)
		{
			pList.push_back(dwPID);
		}
	}
	
	// Open a synchronous WCT session.
	g_WctHandle = OpenThreadWaitChainSession(0, NULL);
	if (NULL == g_WctHandle)
	{		
		_ftprintf(g_FileStream, TEXT("ERROR: OpenThreadWaitChainSession failed\n"));
		goto Cleanup;
	}

	// If neither PID nor the Process Name is specified then there is no need to generate a dump file !!!
	if (!fPID && !fPName)
	{
		CheckThreads(0, FALSE);
	}
	else
	{
		for (DWORD dwC : pList)
		{
			CheckThreads(dwC, fDump);
		}
	}
	// Close the WCT session.
	CloseThreadWaitChainSession(g_WctHandle);
		

Cleanup:

	if (NULL != g_Ole32Hnd)
	{
		FreeLibrary(g_Ole32Hnd);
	}

}

void ProcessRelevantProcesses(std::list<DWORD> relevantProcesses, BOOL fDump)
{
	
	if (relevantProcesses.size() == 0)
	{		
		return;
	}

	_ftprintf(g_FileStream, _TEXT("Relevant Processes:\t"));	
		
	// List the processes found in the waitchain nodes of this process
	for (DWORD dwC : relevantProcesses)
	{
		_ftprintf(g_FileStream, _TEXT("0x%x->"), dwC);			
	}
	_ftprintf(g_FileStream, _TEXT("[END]\n\n"));

	// Then generate dump files of the processes found in the waitchain nodes of this process
	if (fDump)
	{		
		for (DWORD dwC : relevantProcesses)
		{
			GenerateDumpFile(dwC);
		}
		_ftprintf(g_FileStream, _TEXT("\n"));
	}

}