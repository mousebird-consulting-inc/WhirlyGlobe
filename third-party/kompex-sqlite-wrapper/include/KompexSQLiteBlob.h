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

#ifndef KompexSQLiteBlob_H
#define KompexSQLiteBlob_H

#include "sqlite3.h"

#include "KompexSQLitePrerequisites.h"

namespace Kompex
{
	class SQLiteDatabase;

	enum BLOB_ACCESS_MODE
	{
		BLOB_READONLY = 0,
		BLOB_READWRITE
	};		

	//! Administration of existing BLOBs.
	class _SQLiteWrapperExport SQLiteBlob
	{
	public:
		//! Constructor.
		SQLiteBlob();
		//! Constructor.\n
		//! Opens a handle to a BLOB located in row 'rowId', column 'columnName', table 'tableName' in database 'symbolicDatabaseName',\n
		//! in other words, the same BLOB that would be selected by: SELECT columnName FROM symbolicDatabaseName.tableName WHERE rowid = rowId;
		//! @param db						Database in which we will work.
		//! @param symbolicDatabaseName		Name of the database in which the BLOB is located - note that the database name is not the filename\n
		//!									that contains the database but rather the symbolic name of the database that appears after the AS keyword\n
		//!									when the database is connected using ATTACH. For the main database file, the database name is "main".\n
		//!									For TEMP tables, the database name is "temp".
		//! @param tableName				Name of the table in which the BLOB is located.
		//! @param columnName				Name of the column in which the BLOB is located.
		//! @param rowId					ID of the dataset in which the BLOB is located.
		//! @param accessMode				BLOB_READONLY - opens the blob in read-only mode.\n
		//! 								BLOB_READWRITE - opens the blob in read and write mode.\n
		//! 								It is not possible to open a column that is part of an index or primary key for writing.\n
		//! 								If foreign key constraints are enabled, it is not possible to open a column that is part of a child key for writing.
		SQLiteBlob(SQLiteDatabase *db, std::string symbolicDatabaseName, std::string tableName, std::string columnName, int64 rowId, BLOB_ACCESS_MODE accessMode = BLOB_READWRITE);
		//! Destructor.\n
		//! Calls also CloseBlob() so that you don't need to close the BLOB explicitly.
		virtual ~SQLiteBlob();

		//! Opens a handle to a BLOB located in row 'rowId', column 'columnName', table 'tableName' in database 'symbolicDatabaseName',\n
		//! in other words, the same BLOB that would be selected by: SELECT columnName FROM symbolicDatabaseName.tableName WHERE rowid = rowId;
		//! @param db						Database in which we will work.
		//! @param symbolicDatabaseName		Name of the database in which the BLOB is located - note that the database name is not the filename\n
		//!									that contains the database but rather the symbolic name of the database that appears after the AS keyword\n
		//!									when the database is connected using ATTACH. For the main database file, the database name is "main".\n
		//!									For TEMP tables, the database name is "temp".
		//! @param tableName				Name of the table in which the BLOB is located.
		//! @param columnName				Name of the column in which the BLOB is located.
		//! @param rowId					ID of the dataset in which the BLOB is located.
		//! @param accessMode				BLOB_READONLY - opens the blob in read-only mode.\n
		//! 								BLOB_READWRITE - opens the blob in read and write mode.\n
		//! 								It is not possible to open a column that is part of an index or primary key for writing.\n
		//! 								If foreign key constraints are enabled, it is not possible to open a column that is part of a child key for writing.
		void OpenBlob(SQLiteDatabase *db, std::string symbolicDatabaseName, std::string tableName, std::string columnName, int64 rowId, BLOB_ACCESS_MODE accessMode = BLOB_READWRITE);
		
		//! Closes an open BLOB handle.\n
		//! Shall cause the current transaction to commit if there are no other BLOBs,\n
		//! no pending prepared statements, and the database connection is in autocommit mode.\n
		//! If any writes were made to the BLOB, they might be held in cache until the close operation if they will fit.
		void CloseBlob();

		//! Returns the size in bytes of the BLOB.
		int GetBlobSize() const;

		//! Reads the data from the BLOB into your supplied buffer.\n
		//! Please note: If the row that a BLOB handle points to is modified by an UPDATE, DELETE, or by ON CONFLICT side-effects\n
		//! then the BLOB handle is marked as "expired". This is true if any column of the row is changed, even a column other than\n
		//! the one the BLOB handle is open on. If a BLOB handle is marked as "expired", the call of ReadBlob() will fail and throw an exception.
		//! @param buffer			Buffer in which the BLOB data will be read.
		//! @param numberOfBytes	Number of bytes which will be copied in the buffer.
		//! @param offset			Read the BLOB data starting at this offset.
		void ReadBlob(void *buffer, int numberOfBytes, int offset = 0);

		//! Writes the data from your supplied buffer into the BLOB.\n
		//! This function may only modify the contents of the BLOB; it is not possible to increase the size of the BLOB.\n
		//! Please note: If the row that a BLOB handle points to is modified by an UPDATE, DELETE, or by ON CONFLICT side-effects\n
		//! then the BLOB handle is marked as "expired". This is true if any column of the row is changed, even a column other than\n
		//! the one the BLOB handle is open on. If a BLOB handle is marked as "expired", the call of WriteBlob() will fail and throw an exception.\n
		//! Writes to the BLOB that occurred before the BLOB handle expired are not rolled back by the expiration of the handle,\n
		//! though of course those changes might have been overwritten by the statement that expired the BLOB handle or by other independent statements.
		//! @param buffer			Buffer in which the BLOB data will be read.
		//! @param numberOfBytes	Number of bytes which will be copied in the BLOB.
		//! @param offset			Write the buffer data in the BLOB starting at this offset.
		void WriteBlob(const void *buffer, int numberOfBytes, int offset = 0);

	protected:
		//! Returns the BLOB handle.
		sqlite3_blob *GetBlobHandle() const {return mBlobHandle;}

	private:
		//! BLOB handle
		sqlite3_blob *mBlobHandle;
		//! Database pointer
		SQLiteDatabase *mDatabase;

	};

};

#endif // KompexSQLiteBlob_H
