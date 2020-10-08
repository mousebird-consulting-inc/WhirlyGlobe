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

#ifndef KompexSQLiteStatement_H
#define KompexSQLiteStatement_H

#include <map>
#include <string>

#include "sqlite3.h"

#include "KompexSQLitePrerequisites.h"

namespace Kompex
{	
	class SQLiteDatabase;

	//! Execution of SQL statements and result processing.
	class _SQLiteWrapperExport SQLiteStatement
	{
	public:
		//! Constructor.\n
		//! @param db		Database in which the SQL should be performed
		SQLiteStatement(SQLiteDatabase *db);
		//! Destructor.
		virtual ~SQLiteStatement();

		//! Only for SQL statements which have no result (e.g. INSERT, UPDATE, etc).\n
		//! Can be used for transactions; if you want, you can use an own error handling.\n
		//! Please use 1 (true) and 0 (false) as bool values instead of 'true' and 'false' as string.\n
		//! sqlite doesn't know these key words.
		//! @param sqlStatement		SQL statement (UTF-8) 
		inline void SqlStatement(const std::string &sqlStatement) {SqlStatement(sqlStatement.c_str());}
		//! Only for SQL statements which have no result (e.g. INSERT, UPDATE, etc).\n
		//! Can be used for transactions; if you want, you can use an own error handling.\n
		//! Please use 1 (true) and 0 (false) as bool values instead of 'true' and 'false' as string.\n
		//! sqlite doesn't know these key words.
		//! @param sqlStatement		SQL statement (UTF-8) 
		void SqlStatement(const char *sqlStatement);
		//! Only for SQL statements which have no result (e.g. INSERT, UPDATE, etc).\n
		//! Can be used for transactions; if you want, you can use an own error handling.\n
		//! Please use 1 (true) and 0 (false) as bool values instead of 'true' and 'false' as string.\n
		//! sqlite doesn't know these key words.
		//! @param sqlStatement		SQL statement (UTF-16) 
		inline void SqlStatement(const std::wstring &sqlStatement) {SqlStatement(sqlStatement.c_str());}
		//! Only for SQL statements which have no result (e.g. INSERT, UPDATE, etc).\n
		//! Can be used for transactions; if you want, you can use an own error handling.\n
		//! Please use 1 (true) and 0 (false) as bool values instead of 'true' and 'false' as string.\n
		//! sqlite doesn't know these key words.
		//! @param sqlStatement		SQL statement (UTF-16) 
		void SqlStatement(const wchar_t *sqlStatement);

		//! Only for SQL queries/statements which have a result or for prepared statements.\n
		//! e.g. SELECT's or INSERT's which use Bind..() methods!\n
		//! Do not forget to call FreeQuery() when you have finished.
		inline void Sql(const std::string &sql) {Prepare(sql.c_str());}
		//! Only for SQL queries/statements which have a result or for prepared statements.\n
		//! e.g. SELECT's or INSERT's which use Bind..() methods!\n
		//! Do not forget to call FreeQuery() when you have finished.
		inline void Sql(const char *sql) {Prepare(sql);}
		//! Only for SQL queries/statements which have a result or for prepared statements.\n
		//! e.g. SELECT's or INSERT's which use Bind..() methods!\n
		//! Do not forget to call FreeQuery() when you have finished.
		inline void Sql(const std::wstring &sql) {Prepare(sql.c_str());}
		//! Only for SQL queries/statements which have a result or for prepared statements.\n
		//! e.g. SELECT's or INSERT's which use Bind..() methods!\n
		//! Do not forget to call FreeQuery() when you have finished.
		inline void Sql(const wchar_t *sql) {Prepare(sql);}

		//! If you have called Sql(), you can step throw all results.
		//! @return		'true' if there are further result rows and 'false' if there is no further result row
		bool FetchRow() const;
		//! Call FreeQuery() after Sql(), Execute() and FetchRow() to clean-up.
		void FreeQuery();

		//! Can be used for all SQLite aggregate functions.\n
		//! Here you can see all available aggregate functions: 
		//! http://sqlite.org/lang_aggfunc.html
		//! @param countSql		Complete SQL query string (UTF-16).
		float SqlAggregateFuncResult(const std::string &countSql);
		//! Can be used for all SQLite aggregate functions.\n
		//! Here you can see all available aggregate functions: 
		//! http://sqlite.org/lang_aggfunc.html
		//! @param countSql		Complete SQL query string (UTF-16).
		float SqlAggregateFuncResult(wchar_t *countSql);
		//! Can be used for all SQLite aggregate functions.\n
		//! Here you can see all available aggregate functions: 
		//! http://sqlite.org/lang_aggfunc.html
		//! @param countSql		Complete SQL query string (UTF-16).
		float SqlAggregateFuncResult(const char *countSql);

		//! Returns the name (UTF-8) assigned to a particular column in the result set of a SELECT statement.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the column name.
		const char *GetColumnName(int column) const;
		//! Returns the name (UTF-8) assigned to a particular column in the result set of a SELECT statement.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the column name.
		const char *GetColumnName(const std::string &column) const;

		//! Returns the name (UTF-16) assigned to a particular column in the result set of a SELECT statement.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the column name.
		wchar_t *GetColumnName16(int column) const;
		//! Returns the name (UTF-16) assigned to a particular column in the result set of a SELECT statement.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the column name.
		wchar_t *GetColumnName16(const std::string &column) const;

		//! Returns the datatype code for the initial data type of the result column.\n
		//! SQLITE_INTEGER		1\n
		//! SQLITE_FLOAT		2\n
		//! SQLITE_TEXT			3\n
		//! SQLITE3_TEXT		3\n
		//! SQLITE_BLOB			4\n
		//! SQLITE_NULL			5\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the column type.
		int GetColumnType(int column) const;
		//! Returns the datatype code for the initial data type of the result column.\n
		//! SQLITE_INTEGER		1\n
		//! SQLITE_FLOAT		2\n
		//! SQLITE_TEXT			3\n
		//! SQLITE3_TEXT		3\n
		//! SQLITE_BLOB			4\n
		//! SQLITE_NULL			5\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the column type.
		int GetColumnType(const std::string &column) const;

		//! Returns a character-string from a single column of the current result row of a query.\n
		//! NULL values will be returned as null pointer.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the data.
		const unsigned char *GetColumnCString(int column) const;
		//! Returns a character-string from a single column of the current result row of a query.\n
		//! NULL values will be returned as null pointer.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the data.
		const unsigned char *GetColumnCString(const std::string &column) const;

		//! Returns a std::string from a single column of the current result row of a query.\n
		//! NULL values will be represented as an empty string [std::string("")].\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the data.
		std::string GetColumnString(int column) const;
		//! Returns a std::string from a single column of the current result row of a query.\n
		//! NULL values will be represented as an empty string [std::string("")].\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the data.
		std::string GetColumnString(const std::string &column) const;

		//! Returns a UTF-16 string from a single column of the current result row of a query.\n
		//! NULL values will be returned as null pointer.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the data.
		wchar_t *GetColumnString16(int column) const;
		//! Returns a UTF-16 string from a single column of the current result row of a query.\n
		//! NULL values will be returned as null pointer.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the data.
		wchar_t *GetColumnString16(const std::string &column) const;

		//! Returns a double from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the data.
		double GetColumnDouble(int column) const;
		//! Returns a double from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the data.
		double GetColumnDouble(const std::string &column) const;

		//! Returns a int from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the data.
		int GetColumnInt(int column) const;
		//! Returns a int from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the data.
		int GetColumnInt(const std::string &column) const;

		//! Returns a bool from a single column of the current result row of a query.\n
		//! You must first call Sql()!\n
		//! Please note that GetColumnBool() will also return "false" if you use\n
		//! SqlStatement() with 'true' or 'false' as value for a bool column. Please use 1 (true)\n
		//! or 0 (false) as bool value because sqlite doesn't know the key words 'true' and 'false'.
		//! @param column		Number of the column from which we want to read the data.
		bool GetColumnBool(int column) const;
		//! Returns a bool from a single column of the current result row of a query.\n
		//! You must first call Sql()!\n
		//! Please note that GetColumnBool() will also return "false" if you use\n
		//! SqlStatement() with 'true' or 'false' as value for a bool column. Please use 1 (true)\n
		//! or 0 (false) as bool value because sqlite doesn't know the key words 'true' and 'false'.
		//! @param column		Name of the column from which we want to read the data.
		bool GetColumnBool(const std::string &column) const;

		//! Returns a int64 from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the data.
		int64 GetColumnInt64(int column) const;
		//! Returns a int64 from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the data.
		int64 GetColumnInt64(const std::string &column) const;

		//! Returns a void* from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the data.
		const void *GetColumnBlob(int column) const;
		//! Returns a void* from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the data.
		const void *GetColumnBlob(const std::string &column) const;		
		
		//! Returns the number of bytes in a column that has type BLOB or the number of bytes in a TEXT string with UTF-8 encoding.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the bytes.
		int GetColumnBytes(int column) const;
		//! Returns the same value for BLOBs but for TEXT strings returns the number of bytes in a UTF-16 encoding.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the bytes.
		int GetColumnBytes(const std::string &column) const;		
		//! Returns the same value for BLOBs but for TEXT strings returns the number of bytes in a UTF-16 encoding.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the bytes.
		int GetColumnBytes16(int column) const;
		//! Returns the same value for BLOBs but for TEXT strings returns the number of bytes in a UTF-16 encoding.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the bytes.
		int GetColumnBytes16(const std::string &column) const;		
		
		//! Returns a UTF-8 zero-terminated name of the database.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the database name.
		const char *GetColumnDatabaseName(int column) const;
		//! Returns a UTF-8 zero-terminated name of the database.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the database name.
		const char *GetColumnDatabaseName(const std::string &column) const;
		//! Returns a UTF-16 zero-terminated name of the database.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the database name.
		wchar_t *GetColumnDatabaseName16(int column) const;
		//! Returns a UTF-8 zero-terminated name of the database.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the database name.
		wchar_t *GetColumnDatabaseName16(const std::string &column) const;

		//! Returns a UTF-8 zero-terminated name of the table.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the table name.
		const char *GetColumnTableName(int column) const;
		//! Returns a UTF-8 zero-terminated name of the table.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the table name.
		const char *GetColumnTableName(const std::string &column) const;
		//! Returns a UTF-16 zero-terminated name of the table.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the table name.
		wchar_t *GetColumnTableName16(int column) const;
		//! Returns a UTF-8 zero-terminated name of the table.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the table name.
		wchar_t *GetColumnTableName16(const std::string &column) const;

		//! Returns a UTF-8 zero-terminated name of the table column.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the table column name.
		const char *GetColumnOriginName(int column) const;
		//! Returns a UTF-8 zero-terminated name of the table column.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the table column name.
		const char *GetColumnOriginName(const std::string &column) const;
		//! Returns a UTF-16 zero-terminated name of the table column.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the table column name.
		wchar_t *GetColumnOriginName16(int column) const;
		//! Returns a UTF-8 zero-terminated name of the table column.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the table column name.
		wchar_t *GetColumnOriginName16(const std::string &column) const;

		//! Returns a zero-terminated UTF-8 string containing the declared datatype of the table column.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the declared datatype of the table column.
		const char *GetColumnDeclaredDatatype(int column) const;
		//! Returns a zero-terminated UTF-8 string containing the declared datatype of the table column.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the declared datatype of the table column.
		const char *GetColumnDeclaredDatatype(const std::string &column) const;
		//! Returns a zero-terminated UTF-16 string containing the declared datatype of the table column.\n
		//! You must first call Sql()!
		//! @param column		Number of the column from which we want to read the declared datatype of the table column.
		wchar_t *GetColumnDeclaredDatatype16(int column) const;
		//! Returns a zero-terminated UTF-8 string containing the declared datatype of the table column.\n
		//! You must first call Sql()!
		//! @param column		Name of the column from which we want to read the declared datatype of the table column.
		wchar_t *GetColumnDeclaredDatatype16(const std::string &column) const;

		//! Return the number of columns in the result set.\n
		//! You must first call Sql()!
		int GetColumnCount() const;
		//! Returns the number of values in the current row of the result set.\n
		//! You must first call Sql()!
		inline int GetDataCount() const {return sqlite3_data_count(mStatement);}

		//! Overrides prior binding on the same parameter with an int value.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param value		int value which should be inserted in the indicated column
		void BindInt(int column, int value) const;
		//! Overrides prior binding on the same parameter with an bool value.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param value		bool which should be inserted in the indicated column
		void BindBool(int column, bool value) const;
		//! Overrides prior binding on the same parameter with an UTF-8 string.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param string		UTF-8 string which should be inserted in the indicated column
		void BindString(int column, const std::string &string) const;
		//! Overrides prior binding on the same parameter with an UTF-16 string.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param string		UTF-16 string which should be inserted in the indicated column
		void BindString16(int column, const wchar_t *string) const;
		//! Overrides prior binding on the same parameter with a double value.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param value		double value which should be inserted in the indicated column
		void BindDouble(int column, double value) const;
		//! Overrides prior binding on the same parameter with an int64 value.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param value		int64 value which should be inserted in the indicated column
		void BindInt64(int column, int64 value) const;
		//! Overrides prior binding on the same parameter with NULL.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		void BindNull(int column) const;
		//! Overrides prior binding on the same parameter with a BLOB.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column			Column, in which the data should be inserted
		//! @param data				BLOB data which should inserted in the indicated column
		//! @param numberOfBytes	The size of the second parameter (const void *data) in bytes.\n
		//!							Please pay attention, that numberOfBytes is not the number of characters!
		//!							Default: -1.\n
		//!							Negative numberOfBytes means, that the length of the string is the number of\n
		//!							bytes up to the first zero terminator.
		void BindBlob(int column, const void* data, int numberOfBytes = -1) const;
		//! Overrides prior binding on the same parameter with a blob that is filled with zeroes.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param length		length of BLOB, which is filled with zeroes
		void BindZeroBlob(int column, int length) const;

		//! Executes a prepared statement and doesn't clean-up so that you can reuse the prepared statement.\n
		//! You must first call Sql() and Bind..() methods!\n
		//! After you executed the prepared statement while calling Execute() you must call Reset() afterwards\n
		//! so that the previous bindings will be removed. After this you can bind values with Bind() again.\n
		//! Don't forget to call FreeQuery() to clean-up if you have finsihed your work.
		void Execute() const;
		//! Executes a prepared statement and clean-up.\n
		//! You must first call Sql() and Bind..() methods! 
		void ExecuteAndFree();

		//! Returns the result as a complete table.\n
		//! Note: only for console (textoutput)\n
		//! Output: std::cout
		//! @param sql							SQL query string
		//! @param consoleOutputColumnWidth		Width of the output column within the console
		void GetTable(const std::string &sql, unsigned short consoleOutputColumnWidth = 20) const;

		//! Returns metadata about a specific column of a specific database table.
		//! Note: only console output\n
		//! Output: std::cout
		//! @param tableName		Table in which the column is found
		//! @param columnName		Column for which we want the metadata
		void GetTableColumnMetadata(const std::string &tableName, const std::string &columnName) const;

		//! Resets all SQL parameter bindings back to NULL.\n
		//! ClearBindings() does not reset the bindings on a prepared statement!
		void ClearBindings() const;

		//! Reset() is called to reset a prepared statement object back to its initial state,\n
		//! ready to be re-executed. Any SQL statement variables that had values bound to them\n
		//! using the Bind*() functions retain their values. Use ClearBindings() to reset the bindings.
		void Reset() const;

		//! Begins a transaction.
		void BeginTransaction();
		//! Commits a transaction.\n
		//! Exception output: std::cerr
		void CommitTransaction();
		//! Rollback a transaction.
		inline void RollbackTransaction()
		{
			FreeQuery();
			SqlStatement("ROLLBACK;");
		}

		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want to use the default error handling.
		//! Please note that there is only used a reference of your sql statement.\n
		//! If your sql statement variable is invalid before you called CommitTransaction()
		//! you need to use SecureTransaction(), which creates a internal copy of your sql statement.
		//! @param sql		SQL statement
		inline void Transaction(const char *sql) {mTransactionSQL[mTransactionID++] = std::make_pair(sql, false);}
		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want to use the default error handling.
		//! Please note that there is only used a reference of your sql statement.\n
		//! If your sql statement variable is invalid before you called CommitTransaction()
		//! you need to use SecureTransaction(), which creates a internal copy of your sql statement.
		//! @param sql		SQL statement
		inline void Transaction(const std::string &sql)	{mTransactionSQL[mTransactionID++] = std::make_pair(sql.c_str(), false);}
		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want to use the default error handling.
		//! Please note that there is only used a reference of your sql statement.\n
		//! If your sql statement variable is invalid before you called CommitTransaction()
		//! you need to use SecureTransaction(), which creates a internal copy of your sql statement.
		//! @param sql		SQL statement
		inline void Transaction(const wchar_t *sql) {mTransactionSQL16[mTransactionID++] = std::make_pair(sql, false);}

		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want to use the default error handling.\n
		//! The SecureTransaction() method creates a internal copy of the given sql statement string,\n
		//! so that you do not run into danger if the string will be invalid due to deletion or local scope.
		//! @param sql		SQL statement
		void SecureTransaction(const char *sql);
		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want to use the default error handling.\n
		//! The SecureTransaction() method creates a internal copy of the given sql statement string,\n
		//! so that you do not run into danger if the string will be invalid due to deletion or local scope.
		//! @param sql		SQL statement
		void SecureTransaction(const std::string sql);
		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want to use the default error handling.\n
		//! The SecureTransaction() method creates a internal copy of the given sql statement string,\n
		//! so that you do not run into danger if the string will be invalid due to deletion or local scope.
		//! @param sql		SQL statement
		void SecureTransaction(const wchar_t *sql);

		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		std::string GetSqlResultString(const std::string &sql, const std::string &defaultReturnValue = "");
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		std::string GetSqlResultString(const char *sql, const std::string &defaultReturnValue = "");
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		std::string GetSqlResultString(const wchar_t *sql, const std::string &defaultReturnValue = "");

		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		int GetSqlResultInt(const std::string &sql, int defaultReturnValue = -1);
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		int GetSqlResultInt(const char *sql, int defaultReturnValue = -1);
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		int GetSqlResultInt(const wchar_t *sql, int defaultReturnValue = -1);

		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		int64 GetSqlResultInt64(const std::string &sql, int64 defaultReturnValue = -1);
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		int64 GetSqlResultInt64(const char *sql, int64 defaultReturnValue = -1);
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		int64 GetSqlResultInt64(const wchar_t *sql, int64 defaultReturnValue = -1);

		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		double GetSqlResultDouble(const std::string &sql, double defaultReturnValue = -1);
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		double GetSqlResultDouble(const char *sql, double defaultReturnValue = -1);
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		double GetSqlResultDouble(const wchar_t *sql, double defaultReturnValue = -1);

		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.\n
		//! Important: You must delete the returned pointer if you don't need it anymore.\n
		//! e.g. \n
		//! const unsigned char *myValue = GetSqlResultCString("SELECT name FROM user WHERE userID = 1");\n
		//! // do domething with myValue\n
		//! delete[] myValue;
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		const unsigned char *GetSqlResultCString(const std::string &sql, const unsigned char *defaultReturnValue = 0);
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.\n
		//! Important: You must delete the returned pointer if you don't need it anymore.\n
		//! e.g. \n
		//! const unsigned char *myValue = GetSqlResultCString("SELECT name FROM user WHERE userID = 1");\n
		//! // do domething with myValue\n
		//! delete[] myValue;
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		const unsigned char *GetSqlResultCString(const char *sql, const unsigned char *defaultReturnValue = 0);
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.\n
		//! Important: You must delete the returned pointer if you don't need it anymore.\n
		//! e.g. \n
		//! const unsigned char *myValue = GetSqlResultCString("SELECT name FROM user WHERE userID = 1");\n
		//! // do domething with myValue\n
		//! delete[] myValue;
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		const unsigned char *GetSqlResultCString(const wchar_t *sql, const unsigned char *defaultReturnValue = 0); 

		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.\n
		//! Important: You must delete the returned pointer if you don't need it anymore.\n
		//! e.g. \n
		//! wchar_t *myValue = GetSqlResultString16("SELECT name FROM user WHERE userID = 1");\n
		//! // do domething with myValue\n
		//! delete[] myValue;
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		wchar_t *GetSqlResultString16(const std::string &sql, wchar_t *defaultReturnValue = 0); 
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.\n
		//! Important: You must delete the returned pointer if you don't need it anymore.\n
		//! e.g. \n
		//! wchar_t *myValue = GetSqlResultString16("SELECT name FROM user WHERE userID = 1");\n
		//! // do domething with myValue\n
		//! delete[] myValue;
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		wchar_t *GetSqlResultString16(const char *sql, wchar_t *defaultReturnValue = 0); 
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.\n
		//! Important: You must delete the returned pointer if you don't need it anymore.\n
		//! e.g. \n
		//! wchar_t *myValue = GetSqlResultString16("SELECT name FROM user WHERE userID = 1");\n
		//! // do domething with myValue\n
		//! delete[] myValue;
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		wchar_t *GetSqlResultString16(const wchar_t *sql, wchar_t *defaultReturnValue = 0); 

		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.\n
		//! Important: You must delete the returned pointer if you don't need it anymore.\n
		//! e.g. \n
		//! const void *myValue = GetSqlResultBlob("SELECT picture FROM user WHERE userID = 1");\n
		//! // do domething with myValue\n
		//! delete[] myValue;
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		const void *GetSqlResultBlob(const std::string &sql, const void *defaultReturnValue = 0); 
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.\n
		//! Important: You must delete the returned pointer if you don't need it anymore.\n
		//! e.g. \n
		//! const void *myValue = GetSqlResultBlob("SELECT picture FROM user WHERE userID = 1");\n
		//! // do domething with myValue\n
		//! delete[] myValue;
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		const void *GetSqlResultBlob(const char *sql, const void *defaultReturnValue = 0); 
		//! Executes a SQL statement and returns instantly the result value of the first column from the first row.\n
		//! Important: You must delete the returned pointer if you don't need it anymore.\n
		//! e.g. \n
		//! const void *myValue = GetSqlResultBlob("SELECT picture FROM user WHERE userID = 1");\n
		//! // do domething with myValue\n
		//! delete[] myValue;
		//! @param sql						SQL statement
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		const void *GetSqlResultBlob(const wchar_t *sql, const void *defaultReturnValue = 0);

		//! If you call this function directly after you called the Sql() function then you will get\n
		//! the total number of returned rows. If you call this function after you already requested data from the\n
		//! result then you will get the number of the remaining rows. Please note that the prepared statement will\n
		//! be reset. Therefore, you would request the data of the first result row after you called this function.
		unsigned int GetNumberOfRows();
				
		//! Formatted String Printing Function\n
		//! This function works alike the "printf()" family of functions from the standard C library.\n
		//! Furthermore, it implements some additional formatting options that are useful for constructing SQL statements.\n
		//! All of the usual printf() formatting options apply. In addition, there are "%q", "%Q", and "%z" options.\n\n
		//! <B>%q option</B>\n
		//! The %q option works like %s in that it substitutes a null-terminated string from the argument list.\n
		//! But %q also doubles every '\'' character. %q is designed for use inside a string literal.\n
		//! By doubling each '\'' character it escapes that character and allows it to be inserted into the string.\n
		//! As a general rule you should always use %q instead of %s when inserting text into a string literal.\n\n
		//! <B>%Q option</B>\n
		//! The %Q option works like %q except it also adds single quotes around the outside of the total string.\n
		//! Additionally, if the parameter in the argument list is a NULL pointer, %Q substitutes the text "NULL"\n
		//! (without single quotes).\n\n
		//! <B>%z option</B>\n
		//! The "%z" formatting option works like "%s" but with the addition that after the string has been read and\n
		//! copied into the result, sqlite3_free() is called on the input string. 
		static std::string Mprintf(const char *sql, ...);
		//! Formatted String Printing Function\n
		//! This function works alike the "printf()" family of functions from the standard C library.\n
		//! Furthermore, it implements some additional formatting options that are useful for constructing SQL statements.\n
		//! All of the usual printf() formatting options apply. In addition, there are "%q", "%Q", and "%z" options.\n\n
		//! <B>%q option</B>\n
		//! The %q option works like %s in that it substitutes a null-terminated string from the argument list.\n
		//! But %q also doubles every '\'' character. %q is designed for use inside a string literal.\n
		//! By doubling each '\'' character it escapes that character and allows it to be inserted into the string.\n
		//! As a general rule you should always use %q instead of %s when inserting text into a string literal.\n\n
		//! <B>%Q option</B>\n
		//! The %Q option works like %q except it also adds single quotes around the outside of the total string.\n
		//! Additionally, if the parameter in the argument list is a NULL pointer, %Q substitutes the text "NULL"\n
		//! (without single quotes).\n\n
		//! <B>%z option</B>\n
		//! The "%z" formatting option works like "%s" but with the addition that after the string has been read and\n
		//! copied into the result, sqlite3_free() is called on the input string. 
		static std::string Vmprintf(const char *sql, va_list args);

	protected:
		//! Compile sql query into a byte-code program.
		//! @param sqlStatement			SQL statement (UTF-8) 
		void Prepare(const char *sqlStatement);
		//! Compile sql query into a byte-code program.
		//! @param sqlStatement			SQL statement (UTF-16) 
		void Prepare(const wchar_t *sqlStatement);
		//! Must be called one or more times to evaluate the statement.
		bool Step() const;
		//! Checks if the statement pointer is valid
		void CheckStatement() const;
		//! Checks if the database pointer is valid
		void CheckDatabase() const;

		//! Returns the SQLite statement handle.
		sqlite3_stmt *GetStatementHandle() const {return mStatement;}

		//! Returns the first value of the first row. Internally used in GetSqlResultCString().
		const unsigned char *SqlResultCString(const unsigned char *defaultReturnValue);
		//! Returns the first value of the first row. Internally used in GetSqlResultString16().
		wchar_t *SqlResultString16(wchar_t *defaultReturnValue);
		//! Returns the first value of the first row. Internally used in GetSqlResultBlob().
		const void *SqlResultBlob(const void *defaultReturnValue);

		//! Checks whether the given column number is located within the available column range.
		//! @param columnNumber			column number which shall be checked
		//! @param functionName                 name of the function which shall be shown in the exception message
		void CheckColumnNumber(int columnNumber, const std::string &functionName = "") const;
		
		//! Free the allocated memory and clean the containers
		void CleanUpTransaction();

         	//! Free the allocated memory of sql statements
		//! @param isMemAllocated		Was memory allocated?
		//! @param str					SQL statement string
		template<class T>
		inline void DeleteTransactionSqlStr(bool isMemAllocated, T *str)
		{
			if(isMemAllocated)
				delete[] str;
		}

		//! Returns the first value of the first row from the given sql statement result.
		//! @param sql						SQL statement
		//! @param getColumnFunc			Function to get the SQL statement result
		//! @param defaultReturnValue		Default return value when the SQL statement has no result
		template<class S, class T>
		T GetColumnValue(S sql, T(Kompex::SQLiteStatement::*getColumnFunc)(int columnNumber)const, T defaultReturnValue);

	private:
		//! Assigns to every column number the corresponding column name.
		void AssignColumnNumberToColumnName() const;
		//! Returns the column number for a given column name.
		int GetAssignedColumnNumber(const std::string &columnName) const;

		//! SQL statement
		struct sqlite3_stmt *mStatement;
		//! Database pointer
		SQLiteDatabase *mDatabase;

		//! typedef for UTF-8 transaction statements
		typedef std::map<unsigned short /* transaction id */, std::pair<const char* /* sql */, bool /* memory allocated */> > TTransactionSQL;
		//! typedef for UTF-16 transaction statements
		typedef std::map<unsigned short /* transaction id */, std::pair<const wchar_t* /* sql */, bool /*  is memory allocated */> > TTransactionSQL16;
	
		//! Stores UTF-8 transaction statements
		TTransactionSQL mTransactionSQL;
		//! Stores UTF-16 transaction statements
		TTransactionSQL16 mTransactionSQL16;

		//! ID for transactions
		unsigned short mTransactionID;
		
		//! Container which stores the assignments for every column number and the corresponding column name (cache the results).
		mutable std::map<std::string /* column name */, int /* column number */> mColumnNumberToColumnNameAssignment;
		//! Saves whether the assignments for every column number and the corresponding column name was already done.
		mutable bool mIsColumnNumberAssignedToColumnName;

	};
};

#endif // KompexSQLiteStatement_H
