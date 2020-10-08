/*
    This file is part of Kompex SQLite Wrapper.
	Copyright (c) 2008-2014 Sven Broeske

    Kompex SQLite Wrapper is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Kompex SQLite Wrapper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Kompex SQLite Wrapper. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KompexSQLiteDatabase_H
#define KompexSQLiteDatabase_H

#include <string>

#include "sqlite3.h"

#include "KompexSQLitePrerequisites.h"

namespace Kompex
{
	//! Administration of the database and all concerning settings.
	class _SQLiteWrapperExport SQLiteDatabase
	{
	public:
		//! Default constructor.\n
		//! Closes automatically the connection to a SQLite database file.
		SQLiteDatabase();

		/** 
		Overloaded constructor.\n
		Opens a connection to a SQLite database file.

		@param filename		Database filename (UTF-8)
		@param flags		Flags \n
							SQLITE_OPEN_READONLY\n
							The database is opened in read-only mode. \n
							If the database does not already exist, an error is returned.\n
							SQLITE_OPEN_READWRITE\n
							The database is opened for reading and writing if possible, \n
							or reading only if the file is write protected by the operating system. \n
							In either case the database must already exist, otherwise an error is returned.\n
							SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE\n
							The database is opened for reading and writing, \n
							and is creates it if it does not already exist. 
		@param zVfs			Name of VFS module to use\n
							NULL for default
		*/
		SQLiteDatabase(const char *filename, int flags, const char *zVfs);

		/** 
		Overloaded constructor.\n
		Opens a connection to a SQLite database file.

		@param filename		Database filename (UTF-8)
		@param flags		Flags \n
							SQLITE_OPEN_READONLY\n
							The database is opened in read-only mode. \n
							If the database does not already exist, an error is returned.\n
							SQLITE_OPEN_READWRITE\n
							The database is opened for reading and writing if possible, \n
							or reading only if the file is write protected by the operating system. \n
							In either case the database must already exist, otherwise an error is returned.\n
							SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE\n
							The database is opened for reading and writing, \n
							and is creates it if it does not already exist. 
		@param zVfs			Name of VFS module to use\n
							NULL for default
		*/
		SQLiteDatabase(const std::string &filename, int flags, const char *zVfs);

		//! Overloaded constructor.\n
		//! Opens a connection to a SQLite database file.
		//! @param filename		Database filename (UTF-16)
		SQLiteDatabase(const wchar_t *filename);

		//! Destructor
		virtual ~SQLiteDatabase();

		/**
		Opens a connection to a SQLite database file.\n
		Shut down existing database handle, if one exist.

		@param filename		Database filename (UTF-8)
		@param flags		Flags \n
							SQLITE_OPEN_READONLY\n
							The database is opened in read-only mode. \n
							If the database does not already exist, an error is returned.\n
							SQLITE_OPEN_READWRITE\n
							The database is opened for reading and writing if possible, \n
							or reading only if the file is write protected by the operating system. \n
							In either case the database must already exist, otherwise an error is returned.\n
							SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE\n
							The database is opened for reading and writing, \n
							and is creates it if it does not already exist. 
		@param zVfs			Name of VFS module to use\n
							NULL for default
		*/
		void Open(const char *filename, int flags, const char *zVfs);

		/**
		Opens a connection to a SQLite database file.
		Shut down existing database handle, if one exist.

		@param filename		Database filename (UTF-8)
		@param flags		Flags\n
							SQLITE_OPEN_READONLY\n
							The database is opened in read-only mode. \n
							If the database does not already exist, an error is returned.\n
							SQLITE_OPEN_READWRITE\n
							The database is opened for reading and writing if possible, \n
							or reading only if the file is write protected by the operating system. \n
							In either case the database must already exist, otherwise an error is returned.\n
							SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE\n
							The database is opened for reading and writing, \n
							and is creates it if it does not already exist. 
		@param zVfs			Name of VFS module to use\n
							NULL for default
		*/
		void Open(const std::string &filename, int flags, const char *zVfs);

		//! Opens a connection to a SQLite database file.\n
		//! Shut down existing database handle, if one exist.
		//! @param filename		Database filename (UTF-16)
		void Open(const wchar_t *filename);

		//! Closes a connection to a SQLite database file.
		void Close();

		//! Returns the SQLite db handle.
		sqlite3 *GetDatabaseHandle() const {return mDatabaseHandle;}
		//! Returns the version number of sqlite.
		inline int GetLibVersionNumber() const {return sqlite3_libversion_number();}
		//! Returns the number of database rows that were changed, inserted or deleted\n
		//! by the most recently completed SQL statement.
		int GetDatabaseChanges() const {return sqlite3_changes(mDatabaseHandle);}
		//! Returns the total number of row changes caused by INSERT, UPDATE or DELETE statements\n
		//! since the database connection was opened. The count includes all changes from all trigger contexts.\n
		//! However, the count does not include changes used to implement REPLACE constraints,\n
		//! do rollbacks or ABORT processing, or DROP table processing.\n
		//! The changes are counted as soon as the statement that makes them is completed.
		int GetTotalDatabaseChanges() const {return sqlite3_total_changes(mDatabaseHandle);}
		//! Causes any pending database operation to abort and return at its earliest opportunity.\n
		//! This routine is typically called in response to a user action such as pressing "Cancel"\n
		//! or Ctrl-C where the user wants a long query operation to halt immediately.
		void InterruptDatabaseOperation() const {sqlite3_interrupt(mDatabaseHandle);}
		//! Returns non-zero or zero if the given database connection is or is not in autocommit mode, respectively.\n
		//! Autocommit mode is on by default. Autocommit mode is disabled by a BEGIN statement.\n
		//! Autocommit mode is re-enabled by a COMMIT or ROLLBACK.
		int GetAutoCommit() const {return sqlite3_get_autocommit(mDatabaseHandle);}

		//! Trace() is invoked at various times when a SQL statement is being run by FetchRow(), SqlStatement() or ExecuteAndFree(). \n
		//! The callback returns a UTF-8 rendering of the SQL statement text as the statement first begins executing.\n
		//! Output: std::cout
		inline void ActivateTracing() const {sqlite3_trace(mDatabaseHandle, &Kompex::SQLiteDatabase::TraceOutput, 0);}
		//! Profile() is invoked as each SQL statement finishes.\n
		//! The profile callback contains the original statement text\n
		//! and an estimate of wall-clock time of how long that statement took to run.\n
		//! Note: time in ns\n
		//! Output: std::cout
		inline void ActivateProfiling() const {sqlite3_profile(mDatabaseHandle, &Kompex::SQLiteDatabase::ProfileOutput, 0);}

		//! The SetSoftHeapLimit() interface places a "soft" limit on the amount of heap memory that may be allocated by SQLite.\n
		//! If an internal allocation is requested that would exceed the soft heap limit, sqlite3_release_memory()\n
		//! is invoked one or more times to free up some space before the allocation is performed.\n
		//! The limit is called "soft", because if sqlite3_release_memory() cannot free sufficient memory to\n
		//! prevent the limit from being exceeded, the memory is allocated anyway and the current operation proceeds.\n
		//! A negative or zero value for heapLimit means that there is no soft heap limit and sqlite3_release_memory()\n
		//! will only be called when memory is exhausted. The default value for the soft heap limit is zero.
		//! @param heapLimit		Heap limit in bytes.
		inline void SetSoftHeapLimit(int heapLimit) const {sqlite3_soft_heap_limit(heapLimit);}

		//! The ReleaseMemory() interface attempts to free N bytes of heap memory by deallocating\n
		//! non-essential memory allocations held by the database library.\n
		//! Memory used to cache database pages to improve performance is an example of non-essential memory.\n
		//! ReleaseMemory() returns the number of bytes actually freed, which might be more or less than the amount requested.
		//! @param bytesOfMemory		Memory in bytes, which should be freed
		inline int ReleaseMemory(int bytesOfMemory) const {return sqlite3_release_memory(bytesOfMemory);}

		//! GetMemoryUsed() returns the number of bytes of memory currently outstanding (malloced but not freed).
		long long GetMemoryUsage() const {return sqlite3_memory_used();}

		//! GetMemoryHighwaterMark() returns the maximum value of used bytes since the high-water mark was last reset.
		//! @param resetFlag		The memory high-water mark is reset to the current value of GetMemoryUsage() if resetFlag is true.\n
		//!							The value returned by GetMemoryHighwaterMark(true) is the high-water mark prior to the reset. 
		long long GetMemoryHighwaterMark(bool resetFlag = false) const {return sqlite3_memory_highwater(resetFlag);}

		//! Provided encodings for MoveDatabaseToMemory().
		enum UtfEncoding {UTF8, UTF16};

		//! Move the whole database into memory.\n
		//! Please pay attention, that after a call of MoveDatabaseToMemory() all sql statements are executed into memory.\n
		//! i.e. that all changes will be lost after closing the database!
		//! Hint: this method can only be used for databases which were openend with a UTF8 filename
		//! @param encoding		Encoding which will be used for moving the data.
		void MoveDatabaseToMemory(UtfEncoding encoding = UTF8);

		//! Takes a snapshot of a database which is located in memory and saves it to a database file.
		//! @param filename		Filename for the new database file to which the snapshot will be saved.\n
		//!						When you leave the filename blank, you will overwrite your origin file database.
		void SaveDatabaseFromMemoryToFile(const std::string &filename = "");
		//! Takes a snapshot of a database which is located in memory and saves it to a database file.
		//! @param filename		Filename for the new database file to which the snapshot will be saved.
		void SaveDatabaseFromMemoryToFile(const wchar_t *filename);

		//! This function returns the rowid of the most recent successful INSERT into the database.\n
		//! If no successful INSERTs have ever occurred on that database connection, zero is returned.\n\n
		//! What is the rowid?\n
		//! Each entry in a SQLite table has a unique 64-bit signed integer key called the "rowid".\n
		//! The rowid is always available as an undeclared column named ROWID, OID or _ROWID_ as long as\n
		//! those names are not also used by explicitly declared columns. If the table has a column\n
		//! of type INTEGER PRIMARY KEY then that column is another alias for the rowid.
		long long GetLastInsertRowId() const {return sqlite3_last_insert_rowid(mDatabaseHandle);}

		//! This function returns true if the database was opened in read-only mode an false if the\n
		//! database was opened in read/write mode.
		bool IsDatabaseReadOnly();

		//! This function attempts to free as much heap memory as possible from the database connection.
		inline void ReleaseMemory() {sqlite3_db_release_memory(mDatabaseHandle);}

		/**
		This method can be used to register a new virtual table module name. Module names must be\n
		registered before creating a new virtual table using the module and before using a preexisting\n
		virtual table for the module. The module name will be registered with this database handle.

		@param moduleName		Name of the module.
		@param module			Pointer to the implementation of the virtual table module.
		@param clientData		Arbitrary client data pointer that is passed through into the xCreate\n
								and xConnect methods of the virtual table module when a new virtual\n
								table is be being created or reinitialized.
		@param xDestroy			Pointer to a destructor for the clientData. SQLite will invoke the destructor\n
								function (if it is not NULL) when SQLite no longer needs the clientData pointer.\n
								The destructor will also be invoked if the call to CreateModule() fails.
		*/
		void CreateModule(const std::string &moduleName, const sqlite3_module *module, void *clientData, void(*xDestroy)(void*));

		//! Returns the number of lookaside memory slots currently checked out.
		int GetNumberOfCheckedOutLookasideMemorySlots() const;
		//! Returns the approximate number of bytes of heap memory used by all pager caches.
		int GetHeapMemoryUsedByPagerCaches() const;
		//! Returns the approximate number of bytes of heap memory used to store the schema for\n
		//! all databases associated with the connection - main, temp, and any ATTACH-ed databases.\n
		//! The full amount of memory used by the schemas is reported, even if the schema memory is\n
		//! shared with other database connections due to shared cache mode being enabled. 
		int GetHeapMemoryUsedToStoreSchemas() const;
		//! Returns the approximate number of bytes of heap and lookaside memory used by all\n
		//! prepared statements associated with the database connection.
		int GetHeapAndLookasideMemoryUsedByPreparedStatements() const;
		//! Returns the number of pager cache hits that have occurred.
		int GetPagerCacheHitCount() const;
		//! Returns the number of pager cache misses that have occurred.
		int GetPagerCacheMissCount() const;
		//! Returns the number of dirty cache entries that have been written to disk. Specifically,\n
		//! the number of pages written to the wal file in wal mode databases, or the number of pages\n
		//! written to the database file in rollback mode databases. Any pages written as part of\n
		//! transaction rollback or database recovery operations are not included. If an IO or other\n
		//! error occurs while writing a page to disk, the effect on subsequent SQLITE_DBSTATUS_CACHE_WRITE\n
		//! requests is undefined.
		int GetNumberOfDirtyCacheEntries() const;
		//! Returns zero if all foreign key constraints (deferred or immediate) have been resolved.
		int GetNumberOfUnresolvedForeignKeys() const;
						
		//! Returns the highest number of lookaside memory slots that has been checked out.
		//! @param resetValue	Allows to reset the highest value.
		int GetHighestNumberOfCheckedOutLookasideMemorySlots(bool resetValue = false);
		//! Returns the number malloc attempts that were satisfied using lookaside memory.
		//! @param resetValue	Allows to reset the highest value.
		int GetLookasideMemoryHitCount(bool resetValue = false);
		//! Returns the number malloc attempts that might have been satisfied using lookaside memory\n
		//! but failed due to the amount of memory requested being larger than the lookaside slot size.
		//! @param resetValue	Allows to reset the highest value.
		int GetLookasideMemoryMissCountDueToSmallSlotSize(bool resetValue = false);
		//! Returns the number malloc attempts that might have been satisfied using lookaside memory\n
		//! but failed due to all lookaside memory already being in use.
		//! @param resetValue	Allows to reset the highest value.
		int GetLookasideMemoryMissCountDueToFullMemory(bool resetValue = false);

	protected:
		//! Callback function for ActivateTracing() [sqlite3_trace]
		static void TraceOutput(void *ptr, const char *sql);
		//! Callback function for ActivateProfiling() [sqlite3_profile]
		static void ProfileOutput(void* ptr, const char* sql, sqlite3_uint64 time);
		//! Build and modify the structure of your tables and other objects in the memory database.
		static int ProcessDDLRow(void *db, int nColumns, char **values, char **columns);
		//! Insert all data from the origin database into the memory database.
		static int ProcessDMLRow(void *db, int nColumns, char **values, char **columns);
		//! Takes and saves a snapshot of the memory database in a file.
		void TakeSnapshot(sqlite3 *destinationDatabase);
		//! Returns internal runtime status information associated with the current database connection.
		int GetRuntimeStatusInformation(int operation, bool highwaterValue = false, bool resetFlag = false) const;

	private:
		//! SQLite db handle
		struct sqlite3 *mDatabaseHandle;
		//! Database filename UTF-8
		std::string mDatabaseFilenameUtf8;
		//! Database filename UTF-16
		std::wstring mDatabaseFilenameUtf16;
		//! Is the database currently stored in memory?
		bool mIsMemoryDatabaseActive;

		//! Clean up routine if something failed in MoveDatabaseToMemory() 
		void CleanUpFailedMemoryDatabase(sqlite3 *memoryDatabase, sqlite3 *rollbackDatabase, bool isDetachNecessary, bool isRollbackNecessary, sqlite3_stmt *stmt, const std::string &errMsg, int internalSqliteErrCode);

	};

};

#endif // KompexSQLiteDatabase_H
