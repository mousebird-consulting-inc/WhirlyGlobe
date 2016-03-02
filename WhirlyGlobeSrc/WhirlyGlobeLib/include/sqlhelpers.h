/*
 *  sqlhelpers.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/8/09.
 *  Copyright 2011-2015 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include <Foundation/Foundation.h>
#include "sqlite3.h"

namespace sqlhelpers
{

/// Create the statement, run it, finalize it.
/// Kicks out an exception on failure
void OneShot(sqlite3 *,const char *);
/// NSString version of OneShot
void OneShot(sqlite3 *,NSString *);

/** Encapsulates a SQLite3 statement in a way that does not make me
    want to punch someone.
 */
class StatementRead
{
public:
	/// Construct with the statement and maybe just run the damn thing
	StatementRead(sqlite3 *db,const char *,bool justRun=false);
	StatementRead(sqlite3 *db,NSString *,bool justRun=false);
	/// Destructor will call finalize
	~StatementRead();
    
    /// Returns false if initialization failed
    bool isValid();
	
	/// Calls step, expecting a row.
	/// Returns false if we're done, throws an exception on error
	bool stepRow();

	/// You can force a finalize here
	void finalize();
	
	/// Return an int from the current row
	int getInt();
	/// Return a double from the current row
	double getDouble();	
	/// Return an NSString from the current row
	NSString *getString();
	/// Return a boolean from the current row
	BOOL getBool();
    /// Return a blob from the current row
    NSData *getBlob();
	
protected:
	void init(sqlite3 *db,const char *,bool justRun=false);
	
    bool valid;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	bool isFinalized;
	int curField;
};

/** This version is for an insert or update.
 */
class StatementWrite
{
public:
	StatementWrite(sqlite3 *db,const char *);
	StatementWrite(sqlite3 *db,NSString *);
	~StatementWrite();
	
	/// Run the insert/update.
	/// Triggers an exception on failure
	void go();
	
	/// Finalize it (optional)
	void finalize();
	
	/// Add an integer
	void add(int);
	/// Add a double
	void add(double);
	/// Add a string
	void add(NSString *);
	/// Add a boolean
	void add(BOOL);
	
protected:
	void init(sqlite3 *db,const char *);
	
	sqlite3_stmt *stmt;
	bool isFinalized;
	int curField;
};	

}
