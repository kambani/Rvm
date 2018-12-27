///
/// @file globals.h
///
/// Globals used in RVM.
///
/// Author: Kaushal Ambani (2018)
///

#pragma once

#define RVM_DEVICE_NAME  L"\\Device\\rvm"

#define RVM_MAX_SEGMENTS_PER_WORKING_SET 128
#define RVM_MAX_TRANSACTIONS_PER_SEGMENT 256
#define RVM_MAX_LOGRECORDS_PER_TRANSACTION 512

#define RVM_MAX_WORKING_SETS 4
#define RVM_MAX_SEGMENTS (RVM_MAX_WORKING_SETS * RVM_MAX_SEGMENTS_PER_WORKING_SET)
#define RVM_MAX_TRANSACTIONS (RVM_MAX_SEGMENTS * RVM_MAX_TRANSACTIONS_PER_SEGMENT)
#define RVM_MAX_LOGRECORDS (RVM_MAX_TRANSACTIONS * RVM_MAX_LOGRECORDS_PER_TRANSACTION)

//#define DECLARE_CONST_UNICODE_STRING(_var, _string) \
//const WCHAR _var ## _buffer[] = _string; \
//const UNICODE_STRING _var = { sizeof(_string) - sizeof(WCHAR), sizeof(_string), (PWCH) _var ## _buffer }

#define RVM_PT 'mvR'
#define RVM_INVALID_INDEX ((ULONG) - 1)

typedef struct _RVM_HANDLE_TABLE_ENTRY {
	PVOID Address;
} RVM_HANDLE_TABLE_ENTRY, *PRVM_HANDLE_TABLE_ENTRY;

typedef struct _RVM_GLOBAL_DATA {

	//
	// Chain of all working Sets in the RVM subsystem
	// All other RVM subsystem structs can be parsed through
	// this chain as they are all interlinked
	//

	LIST_ENTRY WorkingSetHead;

	//
	// RVM device Object
	//

	PDEVICE_OBJECT RvmdeviceObject;

	RVM_HANDLE_TABLE_ENTRY WorkingSetHandleTable[RVM_MAX_WORKING_SETS];
	RVM_HANDLE_TABLE_ENTRY SegmentHandleTable[RVM_MAX_SEGMENTS];
	RVM_HANDLE_TABLE_ENTRY TransactionHandleTable[RVM_MAX_TRANSACTIONS];
	RVM_HANDLE_TABLE_ENTRY LogRecordsHandleTable[RVM_MAX_LOGRECORDS];
} RVM_GLOBAL_DATA, *PRVM_GLOBAL_DATA;

extern RVM_GLOBAL_DATA RvmGlobalData;