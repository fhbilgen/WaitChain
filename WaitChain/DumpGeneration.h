// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

/*************************************************

FUNCTION PROTOTYPES

*************************************************/

/*++

Routine Description:

Spawns a new instance of ProcDump.exe and waits its termination. ProcDump should generate a full memory dump file of the specified process.

Arguments:
procid			:	The ID of the process whose memory dump file will be generated
Return Value	:	TRUE if ProcDump process can be executed otherwise FALSE

--*/
BOOL GenerateDumpFile(DWORD procid);