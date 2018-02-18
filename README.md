
# WaitChain

DESCRIPTION

The WaitChain.exe tool is based on the WCT API example in MSDN. A detailed explanation is at https://blogs.msdn.microsoft.com/hmm/2018/02/18/waitchain/

The tool generates a text file reporting either:
Every process’s Wait Chain when no particular process is specified
A particular process’s Wait Chain list if the ProcessID is specified 
The Wait Chain list of each process instance of an application if the ProcessName is specified

It can also generate the dump file of the particular process(es) and each process that shows up in the Wait Chain list. 
This feature requires procdump to produce the dump files. 
If WaitChain is asked to generate the Wait Chain for a particular process it reports all the running processes and their IDs. 
This comes handy when dump files are generated and a callstack waits on a process that was not captured by WCT API 
and we want to know the name of the process that thread is waiting on.

USAGE

WaitChain v1.0 Wait Chain traversal utility
Reports the Wait Chain of a process.
Optionally, generates memory dump files of the processes listed in the Wait Chain.

Usage:
WaitChain.exe [ProcID | ProcName, [/d]] | [/?]

Options:
ProcID          ProcessID for which Wait Chain analysis will be made.

ProcName        All process instances with ProcName will be analysed.
                If neither ProcID nor ProcName is specified then Wait Chain Analysis report
                for each running process will be generated and the /d option is discarded.

/d              The memory dump file of each process that is in the Wait Chain will be generated.

/?              Displays this help

WaitChain.exe generates a text file report in the same folder where it runs.
The file name format is WAITCHAIN_computername.LOG

FUTURE WORK

The output file format is not text. This can be HTML for better visualization.
Current implementation allows only one ProcessName to be specified as an argument. Multiple ProcessName could be allowed.
There is no customization for the command line arguments of procdump. This can be configurable.
The NetworkIO and SMB wait types are not explored yet. They can be added and tested.
