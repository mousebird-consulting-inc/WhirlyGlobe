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

#ifndef KompexSQLitePrerequisites_H
#define KompexSQLitePrerequisites_H

#if _WIN32
#   define _CDECL _cdecl
#   if defined(_KOMPEX_SQLITEWRAPPER_EXPORT) && defined(_KOMPEX_SQLITEWRAPPER_DYN)
#       define _SQLiteWrapperExport __declspec(dllexport)
#   elif defined(_KOMPEX_SQLITEWRAPPER_DYN)
#       define _SQLiteWrapperExport __declspec(dllimport)
#   else
#       define _SQLiteWrapperExport
#   endif
#else
#       define _SQLiteWrapperExport
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
#else
	typedef long long int int64;
	typedef unsigned long long int uint64;
#endif

#endif // KompexSQLitePrerequisites_H
