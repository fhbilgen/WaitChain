// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

/*************************************************

FUNCTION PROTOTYPES

*************************************************/

/*++

Routine Description:

Loops through the running processes and outputs its hex and decimal ProcID along with the name

Arguments:
szProcName		:	None
Return Value	:	TRUE if it can enumerate the running processes in the system otherwise FALSE

--*/
BOOL DisplayRunningProcessInformation();

/*++

Routine Description:

Loops through the running processes and adds the ProcID of a process if it's name matches the szProcName

Arguments:
szProcName		:	The process name that was specified in the command argument
Return Value	:	A list of ProcessIDs. It is possible that for a process name there are multiple instances running. e.g. w3wp.exe
Then all of the process ID with the same name will be returned to the caller.

--*/
std::list<DWORD> GetProcIDsFromProcName(_TCHAR* szProcName);