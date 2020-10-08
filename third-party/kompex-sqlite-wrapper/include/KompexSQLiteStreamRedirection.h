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

#ifndef KompexSQLiteCerrRedirection_H
#define KompexSQLiteCerrRedirection_H

#include <fstream>
#include <iostream>
#include "KompexSQLitePrerequisites.h"

namespace Kompex
{
	//! Base-class for std redirections
	class _SQLiteWrapperExport Redirection
	{
	public:
		//! Standard constructor
		Redirection() 
		{
			pOutputFile = new std::ofstream();
		}
		//! Destrctor
		virtual ~Redirection() 	
		{
			delete pOutputFile;
		}

	protected:
		//! File handle
		std::ofstream *pOutputFile;
		//! Stream buffer
		std::streambuf *mBuffer;

	private:
		//! Copy constrctor
		Redirection(const Redirection& r);
		//! Assignment operator
		Redirection& operator=(const Redirection& r) {return *this;} 
	};

	//! std::cerr redirection.
	class _SQLiteWrapperExport CerrRedirection : public Redirection
	{
	public:
		//! Overloaded constructor
		CerrRedirection(const std::string &filename)
		{
			pOutputFile->open(filename.c_str(), std::ios_base::out);
			std::streambuf *errbuf = pOutputFile->rdbuf();
			mBuffer = std::cerr.rdbuf();
			std::cerr.rdbuf(errbuf);
		};
		//! Destrctor
		virtual ~CerrRedirection()
		{
			std::cerr.rdbuf(mBuffer);
		}

	private:
		//! Copy constrctor
		CerrRedirection(const CerrRedirection& cr) {}
		//! Assignment operator
		CerrRedirection& operator=(const CerrRedirection& cr) {return *this;} 
	};

	//! std::cout redirection.
	class _SQLiteWrapperExport CoutRedirection : public Redirection
	{
	public:
		//! Overloaded constructor
		CoutRedirection(const std::string &filename)
		{
			pOutputFile->open(filename.c_str(), std::ios_base::out);
			std::streambuf *buf = pOutputFile->rdbuf();
			mBuffer = std::cout.rdbuf();
			std::cout.rdbuf(buf);
		};
		//! Destrctor
		virtual ~CoutRedirection()
		{
			std::cerr.rdbuf(mBuffer);
		}

	private:
		//! Copy constrctor
		CoutRedirection(const CoutRedirection& cr) {}
		//! Assignment operator
		CoutRedirection& operator=(const CoutRedirection& cr) {return *this;} 
	};

};

#endif // KompexSQLiteCerrRedirection_H
