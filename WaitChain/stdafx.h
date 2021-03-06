// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// Platform and Runtime related header files
#include <Windows.h>
#include <wct.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <list>

// Application specific header files
#include "Utility.h"
#include "WaitEngine.h"
#include "DumpGeneration.h"
#include "ProcessEngine.h"

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Advapi32.lib")
