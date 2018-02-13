// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.


#pragma once

/*************************************************

DEFINITIONS

*************************************************/

#define HEADER_WIDTH 40
#define OUTPUT_FILE_PREFIX	_TEXT("WAITCHAIN_")
#define OUTPUT_FILE_SUFFIX	_TEXT(".LOG")


/*************************************************
 
   FUNCTION PROTOTYPES

*************************************************/



/**********************	REPORT FILE RELATED FUNCTIONS **********************/


/*++

Routine Description:

Output either the start or the finish time of the progress

Arguments
fStart			:	If TRUE the function outputs to the report file a "start time" string else an "end time" string
Return Value	:	None

--*/
void PutTime(BOOL fStart);

/*++

Routine Description:

Gets the Process ID and command line information of the WaitChain.exe process
Outputs this information to the report file.

Arguments		:	None
Return Value	:	None

--*/
void GetProcessInfo(void);

// Necessary functions for file output
/*++

Routine Description:

Overwrites any existing report file and creates a new one. The report file contains information about the WaitChain of processes

Arguments		:	None

Return Value	:	None

--*/
void OutputSetFile(void);

/*++

Routine Description:

Closes the global file handle g_FileStream

Arguments		:	None

Return Value	:	None

--*/
void OutputCloseFile(void);

/*++

Routine Description:

Small function that makes a header in the text report file

Arguments
szHeaderName	:	The text that is going to be displayed as a header in the report file

Return Value	:	None

--*/

void OutputDoHeader(_TCHAR* szHeaderName);


/**********************	INPUT ARGUMENT PROCESSING **********************/


/*++

Routine Description:

Outputs a small description of the WaitChain.exe tool

Arguments		:	None

Return Value	:	None

--*/
void DisplayDescription();

/*++

Routine Description:

Displays the WaitChain.exe program's input syntax on to the screen

Arguments		:	None

Return Value	:	None

--*/
void DisplaySyntax();

/*++

Routine Description:

Displays the WaitChain.exe program's input argument explanation and usage information

Arguments		:	None

Return Value	:	None

--*/
void DisplayUsage();

/*++

Routine Description:

The core function that processes the input arguments and chooses the execution paths

Arguments
argc		:	Count of the input arguments
argv		:	String array that contains the input arguments

Return Value	:	TRUE if the arguments are valid, FALSE if the arguments are invalid

--*/
BOOL ProcessArguments(int argc, _TCHAR **argv);