// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.


#pragma once

/*********************************************************************************************************************************************************************

The Godot.exe tool is based on the "Wait Chain Traversal" API https://msdn.microsoft.com/en-us/library/windows/desktop/ms681622(v=vs.85).aspx

The core functionality of the tool is in the WaitEngine.h and WaitEngine.cpp files. And, the code is mainly based on the MSDN sample 

Using WCT
https://msdn.microsoft.com/en-us/library/windows/desktop/ms681418(v=vs.85).aspx

*********************************************************************************************************************************************************************/



typedef struct _STR_ARRAY
{
	CHAR Desc[32];
} STR_ARRAY;

/*************************************************

FUNCTION PROTOTYPES

*************************************************/

/*++

Routine Description:

Enables the debug privilege (SE_DEBUG_NAME) for this process.
This is necessary if we want to retrieve wait chains for processes
not owned by the current user.

Arguments:		None.
Return Value:	TRUE if this privilege could be enabled; FALSE otherwise.

--*/
BOOL GrantDebugPrivilege();

/*++

Routine Description:

Enumerates all threads (or optionally only threads for one
process) in the system.  It the calls the WCT API on each of them.

Arguments:

ProcId--Specifies the process ID to analyze.  If '0' all processes
in the system will be checked.

Return Value:

TRUE if processes could be checked; FALSE if a general failure
occurred.

--*/
BOOL CheckThreads( DWORD ProcId, BOOL fDump);

/*++

Routine Description:

Register COM interfaces with WCT. This enables WCT to provide wait
information if a thread is blocked on a COM call.

--*/
BOOL InitCOMAccess();

/*++

Routine Description:

Enumerates all threads (or optionally only threads for one
process) in the system. It the calls the WCT API on each thread.

Arguments:

ThreadId--Specifies the thread ID to analyze.

Return Value:

(none)

--*/
std::list<DWORD> PrintWaitChain( DWORD ThreadId);

/*
++
Routine Description:

Walks through the Nodes of the Wait Chain of a thread

Arguments:
fPID	: Whether the process ID is specified in the command line arguments or not
dwPID	: The process ID that was among the arguments -if it was specified at all-
fPName	: Whether the process name is specified in the command line arguments or not
szPName : The process name that was among the arguments -if it was specified at all-
fDump	: Whether the processes' which are involved in the WaitChain Analysis image dump files should be generated or not

Return Value: none

--*/
void GetWaitInformation(BOOL fPID, DWORD dwPID, BOOL fPName, _TCHAR* szPName, BOOL fDump);

/*
++
Routine Description:

Loops the list of the processes which are involved in a process's threads' Wait Chain. By default, it simply list the relevant processes.
Optionally, generates memory dump files of those processes by calling procdump.exe

Arguments:
relevantProcesses	: List of process ID's which is generated after calling the GetThreadWaitChain for the threads of the process
fDump				: Whether a memory dump file should be wirtten or not.

Return Value: none

--*/
void ProcessRelevantProcesses(std::list<DWORD> relevantProcesses, BOOL fDump);