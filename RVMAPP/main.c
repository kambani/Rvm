// RvmApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <ntstatus.h>
#include <stdio.h>
#include "rvmapi.h"
#define MAX_FORMATS 8

typedef struct _RVM_PARSE_ENTRY {
	PWCHAR Command;
	PWCHAR SubCommand;
	ULONG MinParameters;
	ULONG MaxParameters;
	PVOID FunctionPointer;
	ULONG Formats[MAX_FORMATS];
} RVM_PARSE_ENTRY, *PRVM_PARSE_ENTRY;

RVM_PARSE_ENTRY RvmParseTableEntries[1] =
{
	{L"workingset", L"create", 1, 1, RvmAppWorkingSetCreateDispatch, {'%s'}}
};

typedef
NTSTATUS
(*PDISPATCH_FUNCTION) (
	__in PCWSTR Arg0,
	__in PCWSTR Arg1,
	__in PCWSTR Arg2,
	__in PCWSTR Arg3,
	__in PCWSTR Arg4,
	__in PCWSTR Arg5,
	__in PCWSTR Arg6,
	__in PCWSTR Arg7
	);

VOID
RvmUsage() {
	wprintf(L"\nrvm workingset create [VoumeLetter:]\n");
}

NTSTATUS
RvmParseEntryAndDispatch(__in int argc, 
						 __in PCWSTR argv[], 
						 __out PRVM_PARSE_ENTRY* ParseEntry) 

{
	
	PCWSTR Command;
	PCWSTR SubCommand;
	ULONG Index;
	ULONG FormatEntry;
	NTSTATUS Status;
	PRVM_PARSE_ENTRY TableEntry;
	PVOID Arguments[MAX_FORMATS];

	argc -= 1;
	argv += 1;
	Index = 0;
	*ParseEntry = NULL;
	FormatEntry = 0;
	Status = STATUS_UNSUCCESSFUL;

	if (argc < 2) {
		//
		// Better have Command and Subcommand
		//

		Status = STATUS_INVALID_PARAMETER_MIX;
		goto Done;
	}
	else {
		//
		// Reduce Command and SubCommand
		//

		argc = -2;
	}

	Command = argv[Index++];
	SubCommand = argv[Index++];

	for (int i = 0; i < RTL_NUMBER_OF(RvmParseTableEntries); i++) {
		if (_wcsicmp(Command, RvmParseTableEntries[i].Command) == 0 &&
			_wcsicmp(Command, RvmParseTableEntries[i].SubCommand == 0)) {

			//
			// Better have required number of remaining parameters
			//
			if (argc >= RTL_NUMBER_OF(RvmParseTableEntries[i].Formats)) {

				TableEntry = &RvmParseTableEntries[i];
				while (FormatEntry <= argc) {
					switch (RvmParseTableEntries[i].Formats[FormatEntry]) {
						case '%s':
							Arguments[FormatEntry] = (PVOID)argv[Index + FormatEntry];
							break;
						case '%u32':
							Arguments[FormatEntry] = (PVOID)wcstoul(argv[Index + FormatEntry], NULL, 0);
							break;
						case '%u64':
							Arguments[FormatEntry] = (PVOID)_wcstoui64(argv[Index + FormatEntry], NULL, 0);
							break;
						default:
							Status = STATUS_INVALID_PARAMETER;
							goto Done;
					}

					FormatEntry += 1;
				}
			} else {
				return Status;
			}

			//
			// Call the dispatch function
			//
			Status = ((PDISPATCH_FUNCTION)(TableEntry->FunctionPointer))(Arguments[0],
																		 Arguments[1],
																		 Arguments[2],
																		 Arguments[3],
																		 Arguments[4],
																		 Arguments[5],
																		 Arguments[6],
																		 Arguments[7]);
		}
	}

Done:
	return Status;
}

int wmain(int argc,
		  PCWSTR argv[])
{
	PRVM_PARSE_ENTRY ParseEntry;
}

