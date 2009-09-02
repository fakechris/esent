// Version 01.03.03.09
//
// This source file has been released by Centre Technologies Ltd. under The BSD License. 
//
// The BSD License 
// Copyright (c) 2008-2009, Centre Technologies Ltd.
// All rights reserved. 
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met: 
//
// •Redistributions of source code must retain the above copyright notice, this list of conditions
//  and the following disclaimer.
//
// •Redistributions in binary form must reproduce the above copyright notice, this list of conditions
//  and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
// •Neither the name of Centre Technologies Ltd, nor the names of its contributors may be used to
//  endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "stdafx.h"
#include "WykiDatabaseDriver.h"
#include "DbErrors.h"

DbErrors *wtdbErrors = NULL;

WTDB_STR WtDbGetErrorString(WTDB_ERR errcode)
{
	if (wtdbErrors == NULL)
	{
		return L"Out of memory? - the internal error resolution object is not available";
	}

	return wtdbErrors->get_error(errcode);
}

DbErrors::DbErrors(void)
{
	if (emap.size() > 0)
		return;

	emap[0] = L"Successful Operation"; // JET_errSuccess

	/* ERRORS */

	emap[-1] = L"Function Not Yet Implemented"; // JET_wrnNyi

	/*	SYSTEM errors
	/**/
	emap[-100] = L"Resource Failure Simulator failure"; // JET_errRfsFailure
	emap[-101] = L"Resource Failure Simulator not initialized"; // JET_errRfsNotArmed
	emap[-102] = L"Could not close file"; // JET_errFileClose
	emap[-103] = L"Could not start thread"; // JET_errOutOfThreads
	emap[-105] = L"System busy due to too many IOs"; // JET_errTooManyIO
	emap[-106] = L"A requested async task could not be executed"; // JET_errTaskDropped
	emap[-107] = L"Fatal internal error"; // JET_errInternalError

	//	BUFFER MANAGER errors
	//
	emap[-255] = L"Buffer dependencies improperly set. Recovery failure"; // JET_errDatabaseBufferDependenciesCorrupted

	/*	DIRECTORY MANAGER errors
	/**/
	emap[321] = L"The version store is still active"; // JET_wrnRemainingVersions
	emap[-322] = L"Version already existed. Recovery failure"; // JET_errPreviousVersion
	emap[-323] = L"Reached Page Boundary"; // JET_errPageBoundary
	emap[-324] = L"Reached Key Boundary"; // JET_errKeyBoundary
	emap[-327] = L"DbDatabase corrupted"; // JET_errBadPageLink
	emap[-328] = L"Bookmark has no corresponding address in database"; // JET_errBadBookmark
	emap[-334] = L"A call to the operating system failed"; // JET_errNTSystemCallFailed
	emap[-338] = L"DbDatabase corrupted"; // JET_errBadParentPageLink
	emap[-340] = L"AvailExt cache doesn't match btree"; // JET_errSPAvailExtCacheOutOfSync
	emap[-341] = L"AvailExt space tree is corrupt"; // JET_errSPAvailExtCorrupted
	emap[-342] = L"Out of memory allocating an AvailExt cache node"; // JET_errSPAvailExtCacheOutOfMemory
	emap[-343] = L"OwnExt space tree is corrupt"; // JET_errSPOwnExtCorrupted
	emap[-344] = L"Dbtime on current page is greater than global database dbtime"; // JET_errDbTimeCorrupted
	emap[345] = L"seek on non-unique index yielded a unique key"; // JET_wrnUniqueKey
	emap[-346] = L"key truncated on index that disallows key truncation"; // JET_errKeyTruncated

	/*	RECORD MANAGER errors
	/**/
	emap[406] = L"Column is a separated long-value"; // JET_wrnSeparateLongValue
	emap[-408] = L"Key is too large"; // JET_errKeyTooBig

	/*	LOGGING/RECOVERY errors
	/**/
	emap[-500] = L"Logged operation cannot be redone"; // JET_errInvalidLoggedOperation
	emap[-501] = L"Log file is corrupt"; // JET_errLogFileCorrupt
	emap[-503] = L"No backup directory given"; // JET_errNoBackupDirectory
	emap[-504] = L"The backup directory is not emtpy"; // JET_errBackupDirectoryNotEmpty
	emap[-505] = L"Backup is active already"; // JET_errBackupInProgress
	emap[-506] = L"Restore in progress"; // JET_errRestoreInProgress
	emap[-509] = L"Missing the log file for check point"; // JET_errMissingPreviousLogFile
	emap[-510] = L"Failure writing to log file"; // JET_errLogWriteFail
	emap[-511] = L"Try to log something after recovery faild"; // JET_errLogDisabledDueToRecoveryFailure
	emap[-512] = L"Try to log something during recovery redo"; // JET_errCannotLogDuringRecoveryRedo
	emap[-513] = L"Name of logfile does not match internal generation number"; // JET_errLogGenerationMismatch
	emap[-514] = L"Version of log file is not compatible with Jet version"; // JET_errBadLogVersion
	emap[-515] = L"Timestamp in next log does not match expected"; // JET_errInvalidLogSequence
	emap[-516] = L"Log is not active"; // JET_errLoggingDisabled
	emap[-517] = L"Log buffer is too small for recovery"; // JET_errLogBufferTooSmall
	emap[-519] = L"Maximum log file number exceeded"; // JET_errLogSequenceEnd
	emap[-520] = L"No backup in progress"; // JET_errNoBackup
	emap[-521] = L"Backup call out of sequence"; // JET_errInvalidBackupSequence
	emap[-523] = L"Cannot do backup now"; // JET_errBackupNotAllowedYet
	emap[-524] = L"Could not delete backup file"; // JET_errDeleteBackupFileFail
	emap[-525] = L"Could not make backup temp directory"; // JET_errMakeBackupDirectoryFail
	emap[-526] = L"Cannot perform incremental backup when circular logging enabled"; // JET_errInvalidBackup
	emap[-527] = L"Restored with errors"; // JET_errRecoveredWithErrors
	emap[-528] = L"Current log file missing"; // JET_errMissingLogFile
	emap[-529] = L"Log disk full"; // JET_errLogDiskFull
	emap[-530] = L"Bad signature for a log file"; // JET_errBadLogSignature
	emap[-531] = L"Bad signature for a db file"; // JET_errBadDbSignature
	emap[-532] = L"Bad signature for a checkpoint file"; // JET_errBadCheckpointSignature
	emap[-533] = L"Checkpoint file not found or corrupt"; // JET_errCheckpointCorrupt
	emap[-534] = L"Patch file page not found during recovery"; // JET_errMissingPatchPage
	emap[-535] = L"Patch file page is not valid"; // JET_errBadPatchPage
	emap[-536] = L"Redo abruptly ended due to sudden failure in reading logs from log file"; // JET_errRedoAbruptEnded
	emap[-537] = L"Signature in SLV file does not agree with database"; // JET_errBadSLVSignature
	emap[-538] = L"Hard restore detected that patch file is missing from backup set"; // JET_errPatchFileMissing
	emap[-539] = L"DbDatabase does not belong with the current set of log files"; // JET_errDatabaseLogSetMismatch
	emap[-540] = L"DbDatabase and streaming file do not match each other"; // JET_errDatabaseStreamingFileMismatch
	emap[-541] = L"actual log file size does not match JET_paramLogFileSize"; // JET_errLogFileSizeMismatch
	emap[-542] = L"Could not locate checkpoint file"; // JET_errCheckpointFileNotFound
	emap[-543] = L"The required log files for recovery is missing."; // JET_errRequiredLogFilesMissing
	emap[-544] = L"Soft recovery is intended on a backup database. Restore should be used instead"; // JET_errSoftRecoveryOnBackupDatabase
	emap[-545] = L"databases have been recovered, but the log file size used during recovery does not match JET_paramLogFileSize"; // JET_errLogFileSizeMismatchDatabasesConsistent
	emap[-546] = L"the log file sector size does not match the current volume's sector size"; // JET_errLogSectorSizeMismatch
	emap[-547] = L"databases have been recovered, but the log file sector size (used during recovery) does not match the current volume's sector size"; // JET_errLogSectorSizeMismatchDatabasesConsistent
	emap[-548] = L"databases have been recovered, but all possible log generations in the current sequence are used; delete all log files and the checkpoint file and backup the databases before continuing"; // JET_errLogSequenceEndDatabasesConsistent

	emap[-549] = L"Illegal attempt to replay a streaming file operation where the data wasn't logged. Probably caused by an attempt to roll-forward with circular logging enabled"; // JET_errStreamingDataNotLogged

	emap[-550] = L"DbDatabase was not shutdown cleanly. Recovery must first be run to properly complete database operations for the previous shutdown."; // JET_errDatabaseDirtyShutdown
	emap[-551] = L"DbDatabase last consistent time unmatched"; // JET_errConsistentTimeMismatch
	emap[-552] = L"Patch file is not generated from this backup"; // JET_errDatabasePatchFileMismatch
	emap[-553] = L"The starting log number too low for the restore"; // JET_errEndingRestoreLogTooLow
	emap[-554] = L"The starting log number too high for the restore"; // JET_errStartingRestoreLogTooHigh
	emap[-555] = L"Restore log file has bad signature"; // JET_errGivenLogFileHasBadSignature
	emap[-556] = L"Restore log file is not contiguous"; // JET_errGivenLogFileIsNotContiguous
	emap[-557] = L"Some restore log files are missing"; // JET_errMissingRestoreLogFiles
	emap[558] = L"Existing log file has bad signature"; // JET_wrnExistingLogFileHasBadSignature
	emap[559] = L"Existing log file is not contiguous"; // JET_wrnExistingLogFileIsNotContiguous
	emap[-560] = L"The database miss a previous full backup befor incremental backup"; // JET_errMissingFullBackup
	emap[-561] = L"The backup database size is not in 4k"; // JET_errBadBackupDatabaseSize
	emap[-562] = L"Attempted to upgrade a database that is already current"; // JET_errDatabaseAlreadyUpgraded
	emap[-563] = L"Attempted to use a database which was only partially converted to the current format -- must restore from backup"; // JET_errDatabaseIncompleteUpgrade
	emap[564] = L"INTERNAL ERROR"; // JET_wrnSkipThisRecord
	emap[-565] = L"Some current log files are missing for continous restore"; // JET_errMissingCurrentLogFiles

	emap[-566] = L"dbtime on page smaller than dbtimeBefore in record"; // JET_errDbTimeTooOld
	emap[-567] = L"dbtime on page in advance of the dbtimeBefore in record"; // JET_errDbTimeTooNew
	emap[-569] = L"Some log or patch files are missing during backup"; // JET_errMissingFileToBackup

	emap[-570] = L"torn-write was detected in a backup set during hard restore"; // JET_errLogTornWriteDuringHardRestore
	emap[-571] = L"torn-write was detected during hard recovery (log was not part of a backup set)"; // JET_errLogTornWriteDuringHardRecovery
	emap[-573] = L"corruption was detected in a backup set during hard restore"; // JET_errLogCorruptDuringHardRestore
	emap[-574] = L"corruption was detected during hard recovery (log was not part of a backup set)"; // JET_errLogCorruptDuringHardRecovery

	emap[-575] = L"Cannot have logging enabled while attempting to upgrade db"; // JET_errMustDisableLoggingForDbUpgrade

	emap[-577] = L"TargetInstance specified for restore is not found or log files don't match"; // JET_errBadRestoreTargetInstance
	emap[578] = L"TargetInstance specified for restore is running"; // JET_wrnTargetInstanceRunning

	emap[-579] = L"Soft recovery successfully replayed all operations, but the Undo phase of recovery was skipped"; // JET_errRecoveredWithoutUndo

	emap[-580] = L"Databases to be restored are not from the same shadow copy backup"; // JET_errDatabasesNotFromSameSnapshot
	emap[-581] = L"Soft recovery on a database from a shadow copy backup set"; // JET_errSoftRecoveryOnSnapshot
	emap[-583] = L"The physical sector size reported by the disk subsystem, is unsupported by ESE for a specific file type."; // JET_errSectorSizeNotSupported

	emap[595] = L"DbDatabase corruption has been repaired"; // JET_wrnDatabaseRepaired

	emap[-601] = L"Unicode translation buffer too small"; // JET_errUnicodeTranslationBufferTooSmall
	emap[-602] = L"Unicode normalization failed"; // JET_errUnicodeTranslationFail
	emap[-603] = L"OS does not provide support for Unicode normalisation (and no normalisation callback was specified)"; // JET_errUnicodeNormalizationNotSupported

	emap[-610] = L"Existing log file has bad signature"; // JET_errExistingLogFileHasBadSignature
	emap[-611] = L"Existing log file is not contiguous"; // JET_errExistingLogFileIsNotContiguous

	emap[-612] = L"Checksum error in log file during backup"; // JET_errLogReadVerifyFailure
	emap[-613] = L"Checksum error in SLV file during backup"; // JET_errSLVReadVerifyFailure

	emap[-614] = L"too many outstanding generations between checkpoint and current generation"; // JET_errCheckpointDepthTooDeep

	emap[-615] = L"hard recovery attempted on a database that wasn't a backup database"; // JET_errRestoreOfNonBackupDatabase
	emap[-616] = L"log truncation attempted but not all required logs were copied"; // JET_errLogFileNotCopied


	emap[-900] = L"Invalid parameter"; // JET_errInvalidGrbit

	emap[-1000] = L"Termination in progress"; // JET_errTermInProgress
	emap[-1001] = L"API not supported"; // JET_errFeatureNotAvailable
	emap[-1002] = L"Invalid name"; // JET_errInvalidName
	emap[-1003] = L"Invalid API parameter"; // JET_errInvalidParameter
	emap[1004] = L"Column is NULL-valued"; // JET_wrnColumnNull
	emap[1006] = L"Buffer too small for data"; // JET_wrnBufferTruncated
	emap[1007] = L"DbDatabase is already attached"; // JET_wrnDatabaseAttached
	emap[-1008] = L"Tried to attach a read-only database file for read/write operations"; // JET_errDatabaseFileReadOnly
	emap[1009] = L"Sort does not fit in memory"; // JET_wrnSortOverflow
	emap[-1010] = L"Invalid database id"; // JET_errInvalidDatabaseId
	emap[-1011] = L"Out of Memory"; // JET_errOutOfMemory
	emap[-1012] = L"Maximum database size reached"; // JET_errOutOfDatabaseSpace
	emap[-1013] = L"Out of table cursors"; // JET_errOutOfCursors
	emap[-1014] = L"Out of database page buffers"; // JET_errOutOfBuffers
	emap[-1015] = L"Too many indexes"; // JET_errTooManyIndexes
	emap[-1016] = L"Too many columns in an index"; // JET_errTooManyKeys
	emap[-1017] = L"Record has been deleted"; // JET_errRecordDeleted
	emap[-1018] = L"Checksum error on a database page"; // JET_errReadVerifyFailure
	emap[-1019] = L"Blank database page"; // JET_errPageNotInitialized
	emap[-1020] = L"Out of file handles"; // JET_errOutOfFileHandles
	emap[-1022] = L"Disk IO error"; // JET_errDiskIO
	emap[-1023] = L"Invalid file path"; // JET_errInvalidPath
	emap[-1024] = L"Invalid system path"; // JET_errInvalidSystemPath
	emap[-1025] = L"Invalid log directory"; // JET_errInvalidLogDirectory
	emap[-1026] = L"Record larger than maximum size"; // JET_errRecordTooBig
	emap[-1027] = L"Too many open databases"; // JET_errTooManyOpenDatabases
	emap[-1028] = L"Not a database file"; // JET_errInvalidDatabase
	emap[-1029] = L"DbDatabase engine not initialized"; // JET_errNotInitialized
	emap[-1030] = L"DbDatabase engine already initialized"; // JET_errAlreadyInitialized
	emap[-1031] = L"DbDatabase engine is being initialized"; // JET_errInitInProgress
	emap[-1032] = L"Cannot access file, the file is locked or in use"; // JET_errFileAccessDenied
	emap[-1038] = L"Buffer is too small"; // JET_errBufferTooSmall
	emap[1039] = L"Exact match not found during seek"; // JET_wrnSeekNotEqual
	emap[-1040] = L"Too many columns defined"; // JET_errTooManyColumns
	emap[-1043] = L"Container is not empty"; // JET_errContainerNotEmpty
	emap[-1044] = L"Filename is invalid"; // JET_errInvalidFilename
	emap[-1045] = L"Invalid bookmark"; // JET_errInvalidBookmark
	emap[-1046] = L"Column used in an index"; // JET_errColumnInUse
	emap[-1047] = L"Data buffer doesn't match column size"; // JET_errInvalidBufferSize
	emap[-1048] = L"Cannot set column value"; // JET_errColumnNotUpdatable
	emap[-1051] = L"Index is in use"; // JET_errIndexInUse
	emap[-1052] = L"Link support unavailable"; // JET_errLinkNotSupported
	emap[-1053] = L"Null keys are disallowed on index"; // JET_errNullKeyDisallowed
	emap[-1054] = L"Operation must be within a transaction"; // JET_errNotInTransaction
	emap[1055] = L"No extended error information"; // JET_wrnNoErrorInfo
	emap[1058] = L"No idle activity occured"; // JET_wrnNoIdleActivity
	emap[-1059] = L"Too many active database users"; // JET_errTooManyActiveUsers
	emap[-1061] = L"Invalid or unknown country code"; // JET_errInvalidCountry
	emap[-1062] = L"Invalid or unknown language id"; // JET_errInvalidLanguageId
	emap[-1063] = L"Invalid or unknown code page"; // JET_errInvalidCodePage
	emap[-1064] = L"Invalid flags for LCMapString()"; // JET_errInvalidLCMapStringFlags
	emap[-1065] = L"Attempted to create a version store entry (RCE) larger than a version bucket"; // JET_errVersionStoreEntryTooBig
	emap[-1066] = L"Version store out of memory (and cleanup attempt failed to complete)"; // JET_errVersionStoreOutOfMemoryAndCleanupTimedOut
	emap[1067] = L"No write lock at transaction level 0"; // JET_wrnNoWriteLock
	emap[1068] = L"Column set to NULL-value"; // JET_wrnColumnSetNull
	emap[-1069] = L"Version store out of memory (cleanup already attempted)"; // JET_errVersionStoreOutOfMemory
	emap[-1071] = L"Cannot index escrow column or SLV column"; // JET_errCannotIndex
	emap[-1072] = L"Record has not been deleted"; // JET_errRecordNotDeleted
	emap[-1073] = L"Too many mempool entries requested"; // JET_errTooManyMempoolEntries
	emap[-1074] = L"Out of btree ObjectIDs (perform offline defrag to reclaim freed/unused ObjectIds)"; // JET_errOutOfObjectIDs
	emap[-1075] = L"Long-value ID counter has reached maximum value. (perform offline defrag to reclaim free/unused LongValueIDs)"; // JET_errOutOfLongValueIDs
	emap[-1076] = L"Auto-increment counter has reached maximum value (offline defrag WILL NOT be able to reclaim free/unused Auto-increment values)."; // JET_errOutOfAutoincrementValues
	emap[-1077] = L"Dbtime counter has reached maximum value (perform offline defrag to reclaim free/unused Dbtime values)"; // JET_errOutOfDbtimeValues
	emap[-1078] = L"Sequential index counter has reached maximum value (perform offline defrag to reclaim free/unused SequentialIndex values)"; // JET_errOutOfSequentialIndexValues

	emap[-1080] = L"Multi-instance call with single-instance mode enabled"; // JET_errRunningInOneInstanceMode
	emap[-1081] = L"Single-instance call with multi-instance mode enabled"; // JET_errRunningInMultiInstanceMode
	emap[-1082] = L"Global system parameters have already been set"; // JET_errSystemParamsAlreadySet

	emap[-1083] = L"System path already used by another database instance"; // JET_errSystemPathInUse
	emap[-1084] = L"Logfile path already used by another database instance"; // JET_errLogFilePathInUse
	emap[-1085] = L"Temp path already used by another database instance"; // JET_errTempPathInUse
	emap[-1086] = L"Instance Name already in use"; // JET_errInstanceNameInUse

	emap[-1090] = L"This instance cannot be used because it encountered a fatal error"; // JET_errInstanceUnavailable
	emap[-1091] = L"This database cannot be used because it encountered a fatal error"; // JET_errDatabaseUnavailable
	emap[-1092] = L"This instance cannot be used because it encountered a log-disk-full error performing an operation (likely transaction rollback) that could not tolerate failure"; // JET_errInstanceUnavailableDueToFatalLogDiskFull

	emap[-1101] = L"Out of sessions"; // JET_errOutOfSessions
	emap[-1102] = L"Write lock failed due to outstanding write lock"; // JET_errWriteConflict
	emap[-1103] = L"Transactions nested too deeply"; // JET_errTransTooDeep
	emap[-1104] = L"Invalid session handle"; // JET_errInvalidSesid
	emap[-1105] = L"Update attempted on uncommitted primary index"; // JET_errWriteConflictPrimaryIndex
	emap[-1108] = L"Operation not allowed within a transaction"; // JET_errInTransaction
	emap[-1109] = L"Must rollback current transaction -- cannot commit or begin a new one"; // JET_errRollbackRequired
	emap[-1110] = L"Read-only transaction tried to modify the database"; // JET_errTransReadOnly
	emap[-1111] = L"Attempt to replace the same record by two diffrerent cursors in the same session"; // JET_errSessionWriteConflict

	emap[-1112] = L"record would be too big if represented in a database format from a previous version of Jet"; // JET_errRecordTooBigForBackwardCompatibility
	emap[-1113] = L"The temp table could not be created due to parameters that conflict with JET_bitTTForwardOnly"; // JET_errCannotMaterializeForwardOnlySort

	emap[-1114] = L"This session handle can't be used with this table id"; // JET_errSesidTableIdMismatch
	emap[-1115] = L"Invalid instance handle"; // JET_errInvalidInstance


	emap[-1201] = L"DbDatabase already exists"; // JET_errDatabaseDuplicate
	emap[-1202] = L"DbDatabase in use"; // JET_errDatabaseInUse
	emap[-1203] = L"No such database"; // JET_errDatabaseNotFound
	emap[-1204] = L"Invalid database name"; // JET_errDatabaseInvalidName
	emap[-1205] = L"Invalid number of pages"; // JET_errDatabaseInvalidPages
	emap[-1206] = L"Non database file or corrupted db"; // JET_errDatabaseCorrupted
	emap[-1207] = L"DbDatabase exclusively locked"; // JET_errDatabaseLocked
	emap[-1208] = L"Cannot disable versioning for this database"; // JET_errCannotDisableVersioning
	emap[-1209] = L"DbDatabase engine is incompatible with database"; // JET_errInvalidDatabaseVersion

	/*	The following error code are for NT clients only. It will return such error during
	*	JetInit if JET_paramCheckFormatWhenOpenFail is set.
	*/
	emap[-1210] = L"The database is in an older (200) format"; // JET_errDatabase200Format
	emap[-1211] = L"The database is in an older (400) format"; // JET_errDatabase400Format
	emap[-1212] = L"The database is in an older (500) format"; // JET_errDatabase500Format

	emap[-1213] = L"The database page size does not match the engine"; // JET_errPageSizeMismatch
	emap[-1214] = L"Cannot start any more database instances"; // JET_errTooManyInstances
	emap[-1215] = L"A different database instance is using this database"; // JET_errDatabaseSharingViolation
	emap[-1216] = L"An outstanding database attachment has been detected at the start or end of recovery, but database is missing or does not match attachment info"; // JET_errAttachedDatabaseMismatch
	emap[-1217] = L"Specified path to database file is illegal"; // JET_errDatabaseInvalidPath
	emap[-1218] = L"A database is being assigned an id already in use"; // JET_errDatabaseIdInUse
	emap[-1219] = L"Force Detach allowed only after normal detach errored out"; // JET_errForceDetachNotAllowed
	emap[-1220] = L"Corruption detected in catalog"; // JET_errCatalogCorrupted
	emap[-1221] = L"DbDatabase is partially attached. Cannot complete attach operation"; // JET_errPartiallyAttachedDB
	emap[-1222] = L"DbDatabase with same signature in use"; // JET_errDatabaseSignInUse

	emap[-1224] = L"Corrupted db but repair not allowed"; // JET_errDatabaseCorruptedNoRepair
	emap[-1225] = L"recovery tried to replay a database creation, but the database was originally created with an incompatible (likely older) version of the database engine"; // JET_errInvalidCreateDbVersion

	emap[1301] = L"Opened an empty table"; // JET_wrnTableEmpty
	emap[-1302] = L"Table is exclusively locked"; // JET_errTableLocked
	emap[-1303] = L"Table already exists"; // JET_errTableDuplicate
	emap[-1304] = L"Table is in use, cannot lock"; // JET_errTableInUse
	emap[-1305] = L"No such table or object"; // JET_errObjectNotFound
	emap[-1307] = L"Bad file/index density"; // JET_errDensityInvalid
	emap[-1308] = L"Table is not empty"; // JET_errTableNotEmpty
	emap[-1310] = L"Invalid table id"; // JET_errInvalidTableId
	emap[-1311] = L"Cannot open any more tables (cleanup already attempted)"; // JET_errTooManyOpenTables
	emap[-1312] = L"Oper. not supported on table"; // JET_errIllegalOperation
	emap[-1313] = L"Cannot open any more tables (cleanup attempt failed to complete)"; // JET_errTooManyOpenTablesAndCleanupTimedOut
	emap[-1314] = L"Table or object name in use"; // JET_errObjectDuplicate
	emap[-1316] = L"Object is invalid for operation"; // JET_errInvalidObject
	emap[-1317] = L"Use CloseTable instead of DeleteTable to delete temp table"; // JET_errCannotDeleteTempTable
	emap[-1318] = L"Illegal attempt to delete a system table"; // JET_errCannotDeleteSystemTable
	emap[-1319] = L"Illegal attempt to delete a template table"; // JET_errCannotDeleteTemplateTable
	emap[-1322] = L"Must have exclusive lock on table."; // JET_errExclusiveTableLockRequired
	emap[-1323] = L"DDL operations prohibited on this table"; // JET_errFixedDDL
	emap[-1324] = L"On a derived table, DDL operations are prohibited on inherited portion of DDL"; // JET_errFixedInheritedDDL
	emap[-1325] = L"Nesting of hierarchical DDL is not currently supported."; // JET_errCannotNestDDL
	emap[-1326] = L"Tried to inherit DDL from a table not marked as a template table."; // JET_errDDLNotInheritable
	emap[1327] = L"System cleanup has a cursor open on the table"; // JET_wrnTableInUseBySystem
	emap[-1328] = L"System parameters were set improperly"; // JET_errInvalidSettings
	emap[-1329] = L"Client has requested stop service"; // JET_errClientRequestToStopJetService
	emap[-1330] = L"Template table was created with NoFixedVarColumnsInDerivedTables"; // JET_errCannotAddFixedVarColumnToDerivedTable

	emap[-1401] = L"Index build failed"; // JET_errIndexCantBuild
	emap[-1402] = L"Primary index already defined"; // JET_errIndexHasPrimary
	emap[-1403] = L"Index is already defined"; // JET_errIndexDuplicate
	emap[-1404] = L"No such index"; // JET_errIndexNotFound
	emap[-1405] = L"Cannot delete clustered index"; // JET_errIndexMustStay
	emap[-1406] = L"Illegal index definition"; // JET_errIndexInvalidDef
	emap[-1409] = L"Invalid create index description"; // JET_errInvalidCreateIndex
	emap[-1410] = L"Out of index description blocks"; // JET_errTooManyOpenIndexes
	emap[-1411] = L"Non-unique inter-record index keys generated for a multivalued index"; // JET_errMultiValuedIndexViolation
	emap[-1412] = L"Failed to build a secondary index that properly reflects primary index"; // JET_errIndexBuildCorrupted
	emap[-1413] = L"Primary index is corrupt. The database must be defragmented"; // JET_errPrimaryIndexCorrupted
	emap[-1414] = L"Secondary index is corrupt. The database must be defragmented"; // JET_errSecondaryIndexCorrupted
	emap[1415] = L"Out of date index removed"; // JET_wrnCorruptIndexDeleted
	emap[-1416] = L"Illegal index id"; // JET_errInvalidIndexId

	emap[-1430] = L"tuple index can only be on a secondary index"; // JET_errIndexTuplesSecondaryIndexOnly
	emap[-1431] = L"tuple index may only have eleven columns in the index"; // JET_errIndexTuplesTooManyColumns
	emap[-1432] = L"tuple index must be a non-unique index"; // JET_errIndexTuplesNonUniqueOnly
	emap[-1433] = L"tuple index must be on a text/binary column"; // JET_errIndexTuplesTextBinaryColumnsOnly
	emap[-1434] = L"tuple index does not allow setting cbVarSegMac"; // JET_errIndexTuplesVarSegMacNotAllowed
	emap[-1435] = L"invalid min/max tuple length or max characters to index specified"; // JET_errIndexTuplesInvalidLimits
	emap[-1436] = L"cannot call RetrieveColumn() with RetrieveFromIndex on a tuple index"; // JET_errIndexTuplesCannotRetrieveFromIndex
	emap[-1437] = L"specified key does not meet minimum tuple length"; // JET_errIndexTuplesKeyTooSmall

	emap[-1501] = L"Column value is long"; // JET_errColumnLong
	emap[-1502] = L"No such chunk in long value"; // JET_errColumnNoChunk
	emap[-1503] = L"Field will not fit in record"; // JET_errColumnDoesNotFit
	emap[-1504] = L"Null not valid"; // JET_errNullInvalid
	emap[-1505] = L"Column indexed, cannot delete"; // JET_errColumnIndexed
	emap[-1506] = L"Field length is greater than maximum"; // JET_errColumnTooBig
	emap[-1507] = L"No such column"; // JET_errColumnNotFound
	emap[-1508] = L"Field is already defined"; // JET_errColumnDuplicate
	emap[-1509] = L"Attempted to create a multi-valued column, but column was not Tagged"; // JET_errMultiValuedColumnMustBeTagged
	emap[-1510] = L"Second autoincrement or version column"; // JET_errColumnRedundant
	emap[-1511] = L"Invalid column data type"; // JET_errInvalidColumnType
	emap[1512] = L"Max length too big, truncated"; // JET_wrnColumnMaxTruncated
	emap[-1514] = L"No non-NULL tagged columns"; // JET_errTaggedNotNULL
	emap[-1515] = L"Invalid w/o a current index"; // JET_errNoCurrentIndex
	emap[-1516] = L"The key is completely made"; // JET_errKeyIsMade
	emap[-1517] = L"Column Id Incorrect"; // JET_errBadColumnId
	emap[-1518] = L"Bad itagSequence for tagged column"; // JET_errBadItagSequence
	emap[-1519] = L"Cannot delete, column participates in relationship"; // JET_errColumnInRelationship
	emap[1520] = L"Single instance column bursted"; // JET_wrnCopyLongValue
	emap[-1521] = L"AutoIncrement and Version cannot be tagged"; // JET_errCannotBeTagged
	emap[-1524] = L"Default value exceeds maximum size"; // JET_errDefaultValueTooBig
	emap[-1525] = L"Duplicate detected on a unique multi-valued column"; // JET_errMultiValuedDuplicate
	emap[-1526] = L"Corruption encountered in long-value tree"; // JET_errLVCorrupted
	emap[-1528] = L"Duplicate detected on a unique multi-valued column after data was normalized, and normalizing truncated the data before comparison"; // JET_errMultiValuedDuplicateAfterTruncation
	emap[-1529] = L"Invalid column in derived table"; // JET_errDerivedColumnCorruption
	emap[-1530] = L"Tried to convert column to a primary index placeholder, but column doesn't meet necessary criteria"; // JET_errInvalidPlaceholderColumn
	emap[1531] = L"Column value(s) not returned because the corresponding column id or itagSequence requested for enumeration was null"; // JET_wrnColumnSkipped
	emap[1532] = L"Column value(s) not returned because they could not be reconstructed from the data at hand"; // JET_wrnColumnNotLocal
	emap[1533] = L"Column values exist that were not requested for enumeration"; // JET_wrnColumnMoreTags
	emap[1534] = L"Column value truncated at the requested size limit during enumeration"; // JET_wrnColumnTruncated
	emap[1535] = L"Column values exist but were not returned by request"; // JET_wrnColumnPresent
	emap[1536] = L"Column value returned in JET_COLUMNENUM as a result of JET_bitEnumerateCompressOutput"; // JET_wrnColumnSingleValue
	emap[1537] = L"Column value(s) not returned because they were set to their default value(s) and JET_bitEnumerateIgnoreDefault was specified"; // JET_wrnColumnDefault

	emap[-1601] = L"The key was not found"; // JET_errRecordNotFound
	emap[-1602] = L"No working buffer"; // JET_errRecordNoCopy
	emap[-1603] = L"Currency not on a record"; // JET_errNoCurrentRecord
	emap[-1604] = L"Primary key may not change"; // JET_errRecordPrimaryChanged
	emap[-1605] = L"Illegal duplicate key"; // JET_errKeyDuplicate
	emap[-1607] = L"Attempted to update record when record update was already in progress"; // JET_errAlreadyPrepared
	emap[-1608] = L"No call to JetMakeKey"; // JET_errKeyNotMade
	emap[-1609] = L"No call to JetPrepareUpdate"; // JET_errUpdateNotPrepared
	emap[1610] = L"Data has changed"; // JET_wrnDataHasChanged
	emap[-1611] = L"Data has changed, operation aborted"; // JET_errDataHasChanged
	emap[1618] = L"Moved to new key"; // JET_wrnKeyChanged
	emap[-1619] = L"Windows installation does not support language"; // JET_errLanguageNotSupported

	emap[-1701] = L"Too many sort processes"; // JET_errTooManySorts
	emap[-1702] = L"Invalid operation on Sort"; // JET_errInvalidOnSort
	emap[-1803] = L"Temp file could not be opened"; // JET_errTempFileOpenError
	emap[-1805] = L"Too many open databases"; // JET_errTooManyAttachedDatabases
	emap[-1808] = L"No space left on disk"; // JET_errDiskFull
	emap[-1809] = L"Permission denied"; // JET_errPermissionDenied
	emap[-1811] = L"File not found"; // JET_errFileNotFound
	emap[-1812] = L"Invalid file type"; // JET_errFileInvalidType
	emap[1813] = L"DbDatabase file is read only"; // JET_wrnFileOpenReadOnly
	emap[-1850] = L"Cannot Restore after init."; // JET_errAfterInitialization
	emap[-1852] = L"Logs could not be interpreted"; // JET_errLogCorrupted
	emap[-1906] = L"Invalid operation"; // JET_errInvalidOperation
	emap[-1907] = L"Access denied"; // JET_errAccessDenied
	emap[1908] = L"Idle registry full"; // JET_wrnIdleFull
	emap[-1909] = L"Infinite split"; // JET_errTooManySplits
	emap[-1910] = L"Multiple threads are using the same session"; // JET_errSessionSharingViolation
	emap[-1911] = L"An entry point in a DLL we require could not be found"; // JET_errEntryPointNotFound
	emap[-1912] = L"Specified session already has a session context set"; // JET_errSessionContextAlreadySet
	emap[-1913] = L"Tried to reset session context, but current thread did not orignally set the session context"; // JET_errSessionContextNotSetByThisThread
	emap[-1914] = L"Tried to terminate session in use"; // JET_errSessionInUse

	emap[-1915] = L"Internal error during dynamic record format conversion"; // JET_errRecordFormatConversionFailed
	emap[-1916] = L"Just one open user database per session is allowed (JET_paramOneDatabasePerSession)"; // JET_errOneDatabasePerSession
	emap[-1917] = L"error during rollback"; // JET_errRollbackError

	emap[2000] = L"Online defrag already running on specified database"; // JET_wrnDefragAlreadyRunning
	emap[2001] = L"Online defrag not running on specified database"; // JET_wrnDefragNotRunning

	emap[2100] = L"Unregistered a non-existant callback function"; // JET_wrnCallbackNotRegistered
	emap[-2101] = L"A callback failed"; // JET_errCallbackFailed
	emap[-2102] = L"A callback function could not be found"; // JET_errCallbackNotResolved


	emap[-2401] = L"OS Shadow copy API used in an invalid sequence"; // JET_errOSSnapshotInvalidSequence
	emap[-2402] = L"OS Shadow copy ended with time-out"; // JET_errOSSnapshotTimeOut
	emap[-2403] = L"OS Shadow copy not allowed (backup or recovery in progress)"; // JET_errOSSnapshotNotAllowed
	emap[-2404] = L"invalid JET_OSSNAPID"; // JET_errOSSnapshotInvalidSnapId

	emap[-3000] = L"Attempted to use Local Storage without a callback function being specified"; // JET_errLSCallbackNotSpecified
	emap[-3001] = L"Attempted to set Local Storage for an object which already had it set"; // JET_errLSAlreadySet
	emap[-3002] = L"Attempted to retrieve Local Storage from an object which didn't have it set"; // JET_errLSNotSet

	/** FILE ERRORS
	**/
	//JET_errFileAccessDenied					-1032
	//JET_errFileNotFound						-1811
	//JET_errInvalidFilename					-1044
	emap[-4000] = L"an I/O was issued to a location that was sparse"; // JET_errFileIOSparse
	emap[-4001] = L"a read was issued to a location beyond EOF (writes will expand the file)"; // JET_errFileIOBeyondEOF
	emap[-4002] = L"instructs the JET_ABORTRETRYFAILCALLBACK caller to abort the specified I/O"; // JET_errFileIOAbort
	emap[-4003] = L"instructs the JET_ABORTRETRYFAILCALLBACK caller to retry the specified I/O"; // JET_errFileIORetry
	emap[-4004] = L"instructs the JET_ABORTRETRYFAILCALLBACK caller to fail the specified I/O"; // JET_errFileIOFail
	emap[-4005] = L"read/write access is not supported on compressed files"; // JET_errFileCompressed

	//
	// Wyki Database Driver Specific Errors
	//
	
	emap[WTDB_errWyki_Driver_Error] = L"The Wyki database driver has experienced an internal problem";
	emap[WTDB_errError_Lost_Update] = L"The modification timestamp is different (the record has been indpendently updated since it the caller last loaded it, which would cause data to be lost if this update proceeded)";
	emap[WTDB_errPost_Has_Children] = L"This post cannot be deleted because it has reply posts that cannot be deleted";
	emap[WTDB_errNot_Implemented] = L"This function call has not been implemented yet";
	emap[WTDB_errRuntime_Lib_Error] = L"A function in the C library has thrown an exception, for example to indicate failure to allocate new memory";
	emap[WTDB_wrnPost_Created_Not_Set] = L"An post object is being created with a pre-specified id indicating restoration of a backup copy, yet the created date attribute of the backup copy is missing";
	emap[WTDB_errSchema_Error] = L"An operation cannot proceed because there has been, or would be, a violation of the current Wyki database schema";
}

DbErrors::~DbErrors(void)
{
}
