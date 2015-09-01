////////////////////////////////////////////////////////////////////////////////
// CppSQLite3U is a C++ unicode wrapper around the SQLite3 embedded database library.
//
// Copyright (c) 2006  Tyushkov Nikolay.  All Rights Reserved. http://softvoile.com
//
//
// Based on beautiful wrapper written by Rob Groves  
// (https://secure.codeproject.com/database/CppSQLite.asp). 
// Very good wrapper, but without unicode support unfortunately. 
// So, I have reconstructed it for unicode.
//
//  CppSQLite3 wrapper:
//  Copyright (c) 2004 Rob Groves. All Rights Reserved. rob.groves@btinternet.com
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without a written
// agreement, is hereby granted, provided that the above copyright notice, 
// this paragraph and the following two paragraphs appear in all copies, 
// modifications, and distributions.
//
// IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
// PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
// EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
// ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". THE AUTHOR HAS NO OBLIGATION
// TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
// 
// If you want to get some documentation look at 
// https://secure.codeproject.com/database/CppSQLite.asp
// Note, not all features from CppSQLite3 were implemented in CppSQLite3U
//
// V1.0		11/06/2006	- Initial Public Version
//
//  Noteses : 
//   I have tested this wrapper only in unicode version, so I have no idea 
//   about its work in ANSI configuration, I think it doesn't work without modification;)
//
//  Home page : http://softvoile.com/development/CppSQLite3U/
//  Please send all bug report and comment to mail2@softvoile.com
// 
// 
////////////////////////////////////////////////////////////////////////////////

#include "CppSQLite3U.h"


/////////////////////////////////////////////////////////////////////////////
// CppSQLite3Exception

CppSQLite3Exception::CppSQLite3Exception(const int nErrCode,
									LPTSTR szErrMess,
									bool bDeleteMsg/*=true*/) :
									mnErrCode(nErrCode)
{
	mpszErrMess=new TCHAR[ szErrMess ? _tcslen(szErrMess)+50 : 50];
	_stprintf(mpszErrMess, _T("%s[%d]: %s"),
								errorCodeAsString(nErrCode),
								nErrCode,
								szErrMess ? szErrMess : _T(""));

	if (bDeleteMsg && szErrMess)
	{
		_sqlite3_free((char*)szErrMess);
	}
}


CppSQLite3Exception::CppSQLite3Exception(const CppSQLite3Exception&  e) :
									mnErrCode(e.mnErrCode)
{
	mpszErrMess = 0;
	if (e.mpszErrMess)
	{
		mpszErrMess=new TCHAR[_tcslen(e.mpszErrMess)+10];
		_stprintf(mpszErrMess, _T("%s"), e.mpszErrMess);
	}
}


LPCTSTR CppSQLite3Exception::errorCodeAsString(int nErrCode)
{
	switch (nErrCode)
	{
		case SQLITE_OK          : return _T("SQLITE_OK");
		case SQLITE_ERROR       : return _T("SQLITE_ERROR");
		case SQLITE_INTERNAL    : return _T("SQLITE_INTERNAL");
		case SQLITE_PERM        : return _T("SQLITE_PERM");
		case SQLITE_ABORT       : return _T("SQLITE_ABORT");
		case SQLITE_BUSY        : return _T("SQLITE_BUSY");
		case SQLITE_LOCKED      : return _T("SQLITE_LOCKED");
		case SQLITE_NOMEM       : return _T("SQLITE_NOMEM");
		case SQLITE_READONLY    : return _T("SQLITE_READONLY");
		case SQLITE_INTERRUPT   : return _T("SQLITE_INTERRUPT");
		case SQLITE_IOERR       : return _T("SQLITE_IOERR");
		case SQLITE_CORRUPT     : return _T("SQLITE_CORRUPT");
		case SQLITE_NOTFOUND    : return _T("SQLITE_NOTFOUND");
		case SQLITE_FULL        : return _T("SQLITE_FULL");
		case SQLITE_CANTOPEN    : return _T("SQLITE_CANTOPEN");
		case SQLITE_PROTOCOL    : return _T("SQLITE_PROTOCOL");
		case SQLITE_EMPTY       : return _T("SQLITE_EMPTY");
		case SQLITE_SCHEMA      : return _T("SQLITE_SCHEMA");
		case SQLITE_TOOBIG      : return _T("SQLITE_TOOBIG");
		case SQLITE_CONSTRAINT  : return _T("SQLITE_CONSTRAINT");
		case SQLITE_MISMATCH    : return _T("SQLITE_MISMATCH");
		case SQLITE_MISUSE      : return _T("SQLITE_MISUSE");
		case SQLITE_NOLFS       : return _T("SQLITE_NOLFS");
		case SQLITE_AUTH        : return _T("SQLITE_AUTH");
		case SQLITE_FORMAT      : return _T("SQLITE_FORMAT");
		case SQLITE_RANGE       : return _T("SQLITE_RANGE");
		case SQLITE_ROW         : return _T("SQLITE_ROW");
		case SQLITE_DONE        : return _T("SQLITE_DONE");
		case CPPSQLITE_ERROR    : return _T("CPPSQLITE_ERROR");
		default: return _T("UNKNOWN_ERROR");
	}
}


CppSQLite3Exception::~CppSQLite3Exception()
{
	if (mpszErrMess)
	{
 		delete [] mpszErrMess;
		mpszErrMess = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Prototypes for SQLite functions not included in SQLite DLL, but copied below
// from SQLite encode.c
////////////////////////////////////////////////////////////////////////////////
int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out);
int sqlite3_decode_binary(const unsigned char *in, unsigned char *out);

////////////////////////////////////////////////////////////////////////////////

CppSQLite3Binary::CppSQLite3Binary() :
						mpBuf(0),
						mnBinaryLen(0),
						mnBufferLen(0),
						mnEncodedLen(0),
						mbEncoded(false)
{
}


CppSQLite3Binary::~CppSQLite3Binary()
{
	clear();
}


void CppSQLite3Binary::setBinary(const unsigned char* pBuf, int nLen)
{
	mpBuf = allocBuffer(nLen);
	memcpy(mpBuf, pBuf, nLen);
}


void CppSQLite3Binary::setEncoded(const unsigned char* pBuf)
{
	clear();

	mnEncodedLen = strlen((const char*)pBuf);
	mnBufferLen = mnEncodedLen + 1; // Allow for NULL terminator

	mpBuf = (unsigned char*)malloc(mnBufferLen);

	if (!mpBuf)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								_T("Cannot allocate memory"),
								DONT_DELETE_MSG);
	}

	memcpy(mpBuf, pBuf, mnBufferLen);
	mbEncoded = true;
}


const unsigned char* CppSQLite3Binary::getEncoded()
{
	if (!mbEncoded)
	{
		unsigned char* ptmp = (unsigned char*)malloc(mnBinaryLen);
		memcpy(ptmp, mpBuf, mnBinaryLen);
		mnEncodedLen = sqlite3_encode_binary(ptmp, mnBinaryLen, mpBuf);
		free(ptmp);
		mbEncoded = true;
	}

	return mpBuf;
}


const unsigned char* CppSQLite3Binary::getBinary()
{
	if (mbEncoded)
	{
		// in/out buffers can be the same
		mnBinaryLen = sqlite3_decode_binary(mpBuf, mpBuf);

		if (mnBinaryLen == -1)
		{
			throw CppSQLite3Exception(CPPSQLITE_ERROR,
									_T("Cannot decode binary"),
									DONT_DELETE_MSG);
		}

		mbEncoded = false;
	}

	return mpBuf;
}


int CppSQLite3Binary::getBinaryLength()
{
	getBinary();
	return mnBinaryLen;
}


unsigned char* CppSQLite3Binary::allocBuffer(int nLen)
{
	clear();

	// Allow extra space for encoded binary as per comments in
	// SQLite encode.c See bottom of this file for implementation
	// of SQLite functions use 3 instead of 2 just to be sure ;-)
	mnBinaryLen = nLen;
	mnBufferLen = 3 + (257*nLen)/254;

	mpBuf = (unsigned char*)malloc(mnBufferLen);

	if (!mpBuf)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								_T("Cannot allocate memory"),
								DONT_DELETE_MSG);
	}

	mbEncoded = false;

	return mpBuf;
}


void CppSQLite3Binary::clear()
{
	if (mpBuf)
	{
		mnBinaryLen = 0;
		mnBufferLen = 0;
		free(mpBuf);
		mpBuf = 0;
	}
}



/////////////////////////////////////////////////////////////////////////////
// CppSQLite3DB

CppSQLite3DB::CppSQLite3DB()
{
	mpDB = 0;
	mnBusyTimeoutMs = 60000; // 60 seconds
}

CppSQLite3DB::CppSQLite3DB(const CppSQLite3DB& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
}


CppSQLite3DB::~CppSQLite3DB()
{
	close();
}

////////////////////////////////////////////////////////////////////////////////

CppSQLite3DB& CppSQLite3DB::operator=(const CppSQLite3DB& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
	return *this;
}

void CppSQLite3DB::open(LPCTSTR szFile)
{
    int nRet;

#if defined(_UNICODE) || defined(UNICODE)

	nRet = sqlite3_open16(szFile, &mpDB); // not tested under window 98 

#else // For Ansi Version
//*************-  Added by Begemot  szFile must be in unicode- 23/03/06 11:04 - ****
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx ((OSVERSIONINFO *) &osvi);
   
	if ( osvi.dwMajorVersion == 5) 
    {
          WCHAR pMultiByteStr[MAX_PATH+1];
          MultiByteToWideChar( CP_ACP, 0, szFile,
		                    _tcslen(szFile)+1, pMultiByteStr,   
			                sizeof(pMultiByteStr)/sizeof(pMultiByteStr[0]) );
          nRet = sqlite3_open16(pMultiByteStr, &mpDB);
    }
    else
          nRet = sqlite3_open(szFile,&mpDB);
#endif
//*************************
	if (nRet != SQLITE_OK)
	{
		LPCTSTR szError = (LPCTSTR) _sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
	}
	setBusyTimeout(mnBusyTimeoutMs);
}

void CppSQLite3DB::close()
{
	if (mpDB)
	{
		int nRet = _sqlite3_close(mpDB);

		if (nRet != SQLITE_OK)
		{
			LPCTSTR szError = (LPCTSTR)_sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
		}

		mpDB = 0;
	}
}


CppSQLite3Statement CppSQLite3DB::compileStatement(LPCTSTR szSQL)
{
	checkDB();

	sqlite3_stmt* pVM = compile(szSQL);

	return CppSQLite3Statement(mpDB, pVM);
}


bool CppSQLite3DB::tableExists(LPCTSTR szTable)
{
	TCHAR szSQL[128];
	_stprintf(szSQL, _T("select count(*) from sqlite_master where type='table' and name='%s'"),	szTable);
	int nRet = execScalar(szSQL);
	return (nRet > 0);
}


int CppSQLite3DB::execDML(LPCTSTR szSQL)
{
	int nRet;
	sqlite3_stmt* pVM; 
	checkDB();

	do{ 
		pVM = compile(szSQL);

		nRet = _sqlite3_step(pVM);
	
		if (nRet == SQLITE_ERROR)
		{
			LPCTSTR szError = (LPCTSTR) _sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
		}
		nRet = _sqlite3_finalize(pVM);
	} 
	while( nRet == SQLITE_SCHEMA );
	return nRet;
}

CppSQLite3Query CppSQLite3DB::execQuery(LPCTSTR szSQL)
{
	checkDB();
	int nRet;
	sqlite3_stmt* pVM; 
	
	do{ 
		pVM = compile(szSQL);

		nRet = _sqlite3_step(pVM);

		if (nRet == SQLITE_DONE)
		{	// no rows
			return CppSQLite3Query(mpDB, pVM, true/*eof*/);
		}
		else if (nRet == SQLITE_ROW)
		{	// at least 1 row
			return CppSQLite3Query(mpDB, pVM, false/*eof*/);
		}
		nRet = _sqlite3_finalize(pVM);
	} 
	while( nRet == SQLITE_SCHEMA ); // Edit By Begemot 08/16/06 12:44:35 -   read SQLite FAQ 
	
	LPCTSTR szError = (LPCTSTR) _sqlite3_errmsg(mpDB);
	throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
}


int CppSQLite3DB::execScalar(LPCTSTR szSQL)
{
	CppSQLite3Query q = execQuery(szSQL);

	if (q.eof() || q.numFields() < 1)
		throw CppSQLite3Exception(CPPSQLITE_ERROR, _T("Invalid scalar query"),	DONT_DELETE_MSG);

	return _ttoi(q.fieldValue(0));
}

// Added By Begemot, exact as execScalar but return CString  08/06/06 16:30:37
/*
CString CppSQLite3DB::execScalarStr(LPCTSTR szSQL)
{
	CppSQLite3Query q = execQuery(szSQL);

	if (q.eof() || q.numFields() < 1)
		throw CppSQLite3Exception(CPPSQLITE_ERROR, _T("Invalid scalar query"),	DONT_DELETE_MSG);
	
	return (CString)q.getStringField(0);
}
*/
sqlite_int64 CppSQLite3DB::lastRowId()
{
	return sqlite3_last_insert_rowid(mpDB);
}


void CppSQLite3DB::setBusyTimeout(int nMillisecs)
{
	mnBusyTimeoutMs = nMillisecs;
	sqlite3_busy_timeout(mpDB, mnBusyTimeoutMs);
}


void CppSQLite3DB::checkDB()
{
	if (!mpDB)
		throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Database not open"), DONT_DELETE_MSG);
	
}


sqlite3_stmt* CppSQLite3DB::compile(LPCTSTR szSQL)
{	
	checkDB();
	sqlite3_stmt* pVM;

	int nRet = _sqlite3_prepare(mpDB, szSQL, -1, &pVM, NULL);

	if (nRet != SQLITE_OK)
	{
		pVM=NULL;
		LPCTSTR szError = (LPCTSTR) _sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
	}
	return pVM;
}

////////////////////////////////////////////////////////////////////////////////

CppSQLite3Table::CppSQLite3Table()
{
	mpaszResults = 0;
	mnRows = 0;
	mnCols = 0;
	mnCurrentRow = 0;
}


CppSQLite3Table::CppSQLite3Table(const CppSQLite3Table& rTable)
{
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<CppSQLite3Table&>(rTable).mpaszResults = 0;
	mnRows = rTable.mnRows;
	mnCols = rTable.mnCols;
	mnCurrentRow = rTable.mnCurrentRow;
}


CppSQLite3Table::CppSQLite3Table(TCHAR** paszResults, int nRows, int nCols)
{
	mpaszResults = paszResults;
	mnRows = nRows;
	mnCols = nCols;
	mnCurrentRow = 0;
}


CppSQLite3Table::~CppSQLite3Table()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CppSQLite3Table& CppSQLite3Table::operator=(const CppSQLite3Table& rTable)
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<CppSQLite3Table&>(rTable).mpaszResults = 0;
	mnRows = rTable.mnRows;
	mnCols = rTable.mnCols;
	mnCurrentRow = rTable.mnCurrentRow;
	return *this;
}


void CppSQLite3Table::finalize()
{
	if (mpaszResults)
	{
		sqlite3_free_table((char**)mpaszResults);
		mpaszResults = 0;
	}
}


int CppSQLite3Table::numFields()
{
	checkResults();
	return mnCols;
}


int CppSQLite3Table::numRows()
{
	checkResults();
	return mnRows;
}


const TCHAR* CppSQLite3Table::fieldValue(int nField)
{
	checkResults();

	if (nField < 0 || nField > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								_T("Invalid field index requested"),
								DONT_DELETE_MSG);
	}

	int nIndex = (mnCurrentRow*mnCols) + mnCols + nField;
	return mpaszResults[nIndex];
}


const TCHAR* CppSQLite3Table::fieldValue(const TCHAR* szField)
{
	checkResults();

	if (szField)
	{
		for (int nField = 0; nField < mnCols; nField++)
		{
			if (_tcscmp(szField, mpaszResults[nField]) == 0)
			{
				int nIndex = (mnCurrentRow*mnCols) + mnCols + nField;
				return mpaszResults[nIndex];
			}
		}
	}

	throw CppSQLite3Exception(CPPSQLITE_ERROR,
							_T("Invalid field name requested"),
							DONT_DELETE_MSG);
}


int CppSQLite3Table::getIntField(int nField, int nNullValue/*=0*/)
{
	if (fieldIsNull(nField))
	{
		return nNullValue;
	}
	else
	{
		return _ttoi(fieldValue(nField));
	}
}


int CppSQLite3Table::getIntField(const TCHAR* szField, int nNullValue/*=0*/)
{
	if (fieldIsNull(szField))
	{
		return nNullValue;
	}
	else
	{
		return _ttoi(fieldValue(szField));
	}
}

// #ZHB#
sqlite_int64 CppSQLite3Table::getInt64Field(int nField, sqlite_int64 nNullValue/*=0*/)
{
	if (fieldIsNull(nField))
	{
		return nNullValue;
	}
	else
	{      
		return _ttoi64(fieldValue(nField));  
	}
}


sqlite_int64 CppSQLite3Table::getInt64Field(const TCHAR* szField, sqlite_int64 nNullValue/*=0*/)
{
	if (fieldIsNull(szField))
	{
		return nNullValue;
	}
	else
	{
		return _ttoi64(fieldValue(szField));
	}
}
// END #ZHB#

double CppSQLite3Table::getFloatField(int nField, double fNullValue/*=0.0*/)
{
	if (fieldIsNull(nField))
	{
		return fNullValue;
	}
	else
	{
		TCHAR*  end;
		return _tcstod(fieldValue(nField),&end);
	}
}


double CppSQLite3Table::getFloatField(const TCHAR* szField, double fNullValue/*=0.0*/)
{
	if (fieldIsNull(szField))
	{
		return fNullValue;
	}
	else
	{
		TCHAR*  end;
		return _tcstod(fieldValue(szField),&end);
	}
}


const TCHAR* CppSQLite3Table::getStringField(int nField, const TCHAR* szNullValue/*=""*/)
{
	if (fieldIsNull(nField))
	{
		return szNullValue;
	}
	else
	{
		return fieldValue(nField);
	}
}


const TCHAR* CppSQLite3Table::getStringField(const TCHAR* szField, const TCHAR* szNullValue/*=""*/)
{
	if (fieldIsNull(szField))
	{
		return szNullValue;
	}
	else
	{
		return fieldValue(szField);
	}
}


bool CppSQLite3Table::fieldIsNull(int nField)
{
	checkResults();
	return (fieldValue(nField) == 0);
}


bool CppSQLite3Table::fieldIsNull(const TCHAR* szField)
{
	checkResults();
	return (fieldValue(szField) == 0);
}


const TCHAR* CppSQLite3Table::fieldName(int nCol)
{
	checkResults();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								_T("Invalid field index requested"),
								DONT_DELETE_MSG);
	}

	return mpaszResults[nCol];
}


void CppSQLite3Table::setRow(int nRow)
{
	checkResults();

	if (nRow < 0 || nRow > mnRows-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								_T("Invalid row index requested"),
								DONT_DELETE_MSG);
	}

	mnCurrentRow = nRow;
}


void CppSQLite3Table::checkResults()
{
	if (mpaszResults == 0)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								_T("Null Results pointer"),
								DONT_DELETE_MSG);
	}
}

//////////////////////// CppSQLite3Statement  ///////////////////////////////////////////
CppSQLite3Statement::CppSQLite3Statement()
{
	mpDB = 0;
	mpVM = 0;
}

CppSQLite3Statement::CppSQLite3Statement(const CppSQLite3Statement& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<CppSQLite3Statement&>(rStatement).mpVM = 0;
}

CppSQLite3Statement::CppSQLite3Statement(sqlite3* pDB, sqlite3_stmt* pVM)
{
	mpDB = pDB;
	mpVM = pVM;
}

CppSQLite3Statement::~CppSQLite3Statement()
{
	try
	{
		finalize();
	}
	catch (...)	{}
}

CppSQLite3Statement& CppSQLite3Statement::operator=(const CppSQLite3Statement& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<CppSQLite3Statement&>(rStatement).mpVM = 0;
	return *this;
}

int CppSQLite3Statement::execDML()
{
	checkDB();
	checkVM();

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = sqlite3_changes(mpDB);

		nRet = sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			LPCTSTR szError = (LPCTSTR) _sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
		}
		return nRowsChanged;
	}
	else
	{
		nRet = sqlite3_reset(mpVM);
		LPCTSTR szError = (LPCTSTR) _sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
	}
}


void CppSQLite3Statement::bind(int nParam, LPCTSTR szValue)
{
	checkVM();
	int nRes = _sqlite3_bind_text(mpVM, nParam, szValue, -1, SQLITE_TRANSIENT);
	if (nRes != SQLITE_OK)
		throw CppSQLite3Exception(nRes,_T("Error binding string param"), DONT_DELETE_MSG);
};

void CppSQLite3Statement::bindUtf8(int nParam, const char* szValue){
	checkVM();
	int nRes = sqlite3_bind_text(mpVM, nParam, szValue, -1, SQLITE_TRANSIENT);
	if (nRes != SQLITE_OK)
		throw CppSQLite3Exception(nRes,_T("Error binding string param"), DONT_DELETE_MSG);

}
void CppSQLite3Statement::bind(int nParam, const int nValue)
{
	checkVM();
	int nRes = sqlite3_bind_int(mpVM, nParam, nValue);
	if (nRes != SQLITE_OK)
		throw CppSQLite3Exception(nRes,_T("Error binding int param"), DONT_DELETE_MSG);
}


void CppSQLite3Statement::bind(int nParam, const double dValue)
{
	checkVM();
	int nRes = sqlite3_bind_double(mpVM, nParam, dValue);
	if (nRes != SQLITE_OK)
		throw CppSQLite3Exception(nRes,	_T("Error binding double param"), DONT_DELETE_MSG);
}


void CppSQLite3Statement::bind(int nParam, const unsigned char* blobValue, int nLen)
{
	checkVM();
	int nRes = sqlite3_bind_blob(mpVM, nParam,(const void*)blobValue, nLen, SQLITE_TRANSIENT);
	if (nRes != SQLITE_OK)
		throw CppSQLite3Exception(nRes,_T("Error binding blob param"),DONT_DELETE_MSG);
}

//#ZHB#
void CppSQLite3Statement::bind(int nParam, const sqlite_int64 nValue)
{
	checkVM();
	int nRes = sqlite3_bind_int64(mpVM, nParam, nValue);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes,
								_T("Error binding sqlite_int64 param"),
								DONT_DELETE_MSG);
	}
}
//#ZHB#
void CppSQLite3Statement::bindNull(int nParam)
{
	checkVM();
	int nRes = sqlite3_bind_null(mpVM, nParam);

	if (nRes != SQLITE_OK)
  		throw CppSQLite3Exception(nRes,_T("Error binding NULL param"),DONT_DELETE_MSG);
}


void CppSQLite3Statement::reset()
{
	if (mpVM)
	{
		int nRet = sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			LPCTSTR szError = (LPCTSTR) _sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
		}
	}
}


void CppSQLite3Statement::finalize()
{
	if (mpVM)
	{
		int nRet = sqlite3_finalize(mpVM);
		mpVM = 0;

		if (nRet != SQLITE_OK)
		{
			LPCTSTR szError = (LPCTSTR) _sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
		}
	}
}


void CppSQLite3Statement::checkDB()
{
	if (mpDB == 0) throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Database not open"), DONT_DELETE_MSG);
}

void CppSQLite3Statement::checkVM()
{
	if (mpVM == 0)
		throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Null Virtual Machine pointer"), DONT_DELETE_MSG);
}


/////////////////////  CppSQLite3Query  //////////////////////////////////////////////////
CppSQLite3Query::CppSQLite3Query()
{
	mpVM = 0;
	mbEof = true;
	mnCols = 0;
	mbOwnVM = false;
}


CppSQLite3Query::CppSQLite3Query(const CppSQLite3Query& rQuery)
{
	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<CppSQLite3Query&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
}


CppSQLite3Query::CppSQLite3Query(sqlite3* pDB, sqlite3_stmt* pVM,
								 bool bEof,	 bool bOwnVM/*=true*/)
{
	mpDB = pDB;
	mpVM = pVM;
	mbEof = bEof;
	mnCols = _sqlite3_column_count(mpVM);
	mbOwnVM = bOwnVM;
}

CppSQLite3Query::~CppSQLite3Query()
{
	try
	{
		finalize();
	}
	catch (...) {}
}


CppSQLite3Query& CppSQLite3Query::operator=(const CppSQLite3Query& rQuery)
{
	try
	{
		finalize();
	}
	catch (...)	{ }

	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<CppSQLite3Query&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
	return *this;
}


int CppSQLite3Query::numFields()
{
	checkVM();
	return mnCols;
}


LPCTSTR CppSQLite3Query::fieldValue(int nField)
{
	checkVM();

	if (nField < 0 || nField > mnCols-1)
		throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Invalid field index requested"),DONT_DELETE_MSG);

	return (LPCTSTR)_sqlite3_column_text(mpVM, nField);
}


LPCTSTR CppSQLite3Query::fieldValue(LPCTSTR szField)
{
	int nField = fieldIndex(szField);
	return (LPCTSTR)_sqlite3_column_text(mpVM, nField);
}


int CppSQLite3Query::getIntField(int nField, int nNullValue/*=0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return _sqlite3_column_int(mpVM, nField);
	}
}


int CppSQLite3Query::getIntField(LPCTSTR szField, int nNullValue/*=0*/)
{
	int nField = fieldIndex(szField);
	return getIntField(nField, nNullValue);
}


double CppSQLite3Query::getFloatField(int nField, double fNullValue/*=0.0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return fNullValue;
	}
	else
	{
		return _sqlite3_column_double(mpVM, nField);
	}
}
// #ZHB#
sqlite_int64 CppSQLite3Query::getInt64Field(int nField, sqlite_int64 nNullValue/*=0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{ 
		return sqlite3_column_int64(mpVM, nField);
	}
}

// END #ZHB#

double CppSQLite3Query::getFloatField(LPCTSTR szField, double fNullValue/*=0.0*/)
{
	int nField = fieldIndex(szField);
	return getFloatField(nField, fNullValue);
}


LPCTSTR CppSQLite3Query::getStringField(int nField, LPCTSTR szNullValue/*=""*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return szNullValue;
	}
	else
	{
		return (LPCTSTR)_sqlite3_column_text(mpVM, nField);
	}
}


LPCTSTR CppSQLite3Query::getStringField(LPCTSTR szField, LPCTSTR szNullValue/*=""*/)
{
	int nField = fieldIndex(szField);
	return getStringField(nField, szNullValue);
}
	
const char* CppSQLite3Query::getAnsiStringField(int nField, const char*  szNullValue /*=""*/){
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return szNullValue;
	}
	else
	{
		return (const char*)sqlite3_column_text(mpVM, nField);
	}
};

const unsigned char* CppSQLite3Query::getBlobField(int nField, int& nLen)
{
	checkVM();

	if (nField < 0 || nField > mnCols-1)
		throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Invalid field index requested"),DONT_DELETE_MSG);

	nLen = _sqlite3_column_bytes(mpVM, nField);
	return (const unsigned char*)sqlite3_column_blob(mpVM, nField);
}


const unsigned char* CppSQLite3Query::getBlobField(LPCTSTR szField, int& nLen)
{
	int nField = fieldIndex(szField);
	return getBlobField(nField, nLen);
}


bool CppSQLite3Query::fieldIsNull(int nField)
{
	return (fieldDataType(nField) == SQLITE_NULL);
}


bool CppSQLite3Query::fieldIsNull(LPCTSTR szField)
{
	int nField = fieldIndex(szField);
	return (fieldDataType(nField) == SQLITE_NULL);
}


int CppSQLite3Query::fieldIndex(LPCTSTR szField)
{
	checkVM();

	if (szField)
	{
		for (int nField = 0; nField < mnCols; nField++)
		{
			LPCTSTR szTemp = (LPCTSTR)_sqlite3_column_name(mpVM, nField);

			if (_tcscmp(szField, szTemp) == 0)
			{
				return nField;
			}
		}
	}
	throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Invalid field name requested"),DONT_DELETE_MSG);
}


LPCTSTR CppSQLite3Query::fieldName(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Invalid field index requested"),DONT_DELETE_MSG);
	}
	return (LPCTSTR)_sqlite3_column_name(mpVM, nCol);
}


LPCTSTR CppSQLite3Query::fieldDeclType(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Invalid field index requested"),DONT_DELETE_MSG);
	}
	return (LPCTSTR)_sqlite3_column_decltype(mpVM, nCol);
}


int CppSQLite3Query::fieldDataType(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Invalid field index requested"), DONT_DELETE_MSG);
	}
	return _sqlite3_column_type(mpVM, nCol);
}


bool CppSQLite3Query::eof()
{
	checkVM();
	return mbEof;
}


void CppSQLite3Query::nextRow()
{
	checkVM();

	int nRet = _sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		mbEof = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		// more rows, nothing to do
	}
	else
	{
		nRet = _sqlite3_finalize(mpVM);
		mpVM = 0;
		LPCTSTR szError = (LPCTSTR)_sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet,	(LPTSTR)szError, DONT_DELETE_MSG);
	}
}


void CppSQLite3Query::finalize()
{
	if (mpVM && mbOwnVM)
	{
		int nRet = _sqlite3_finalize(mpVM);
		mpVM = 0;
		if (nRet != SQLITE_OK)
		{
			LPCTSTR szError = (LPCTSTR)_sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (LPTSTR)szError, DONT_DELETE_MSG);
		}
	}
}

void CppSQLite3Query::checkVM()
{
	if (mpVM == 0)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,_T("Null Virtual Machine pointer"),DONT_DELETE_MSG);
	}
}


////////////////////////////////////////////////////////////////////////////////
//**************************
//*************-  Added By Begemot - 28/02/06 20:25 - ****
/*
CString DoubleQuotes(CString in)
{
	in.Replace(_T("\'"),_T("\'\'"));
	return in;
}
*/

////////////////////////////////////////////////////////////////////////////////
// SQLite encode.c reproduced here, containing implementation notes and source
// for sqlite3_encode_binary() and sqlite3_decode_binary() 
////////////////////////////////////////////////////////////////////////////////

/*
** 2002 April 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains helper routines used to translate binary data into
** a null-terminated string (suitable for use in SQLite) and back again.
** These are convenience routines for use by people who want to store binary
** data in an SQLite database.  The code in this file is not used by any other
** part of the SQLite library.
**
** $Id: encode.c,v 1.10 2004/01/14 21:59:23 drh Exp $
*/

/*
** How This Encoder Works
**
** The output is allowed to contain any character except 0x27 (') and
** 0x00.  This is accomplished by using an escape character to encode
** 0x27 and 0x00 as a two-byte sequence.  The escape character is always
** 0x01.  An 0x00 is encoded as the two byte sequence 0x01 0x01.  The
** 0x27 character is encoded as the two byte sequence 0x01 0x03.  Finally,
** the escape character itself is encoded as the two-character sequence
** 0x01 0x02.
**
** To summarize, the encoder works by using an escape sequences as follows:
**
**       0x00  ->  0x01 0x01
**       0x01  ->  0x01 0x02
**       0x27  ->  0x01 0x03
**
** If that were all the encoder did, it would work, but in certain cases
** it could double the size of the encoded string.  For example, to
** encode a string of 100 0x27 characters would require 100 instances of
** the 0x01 0x03 escape sequence resulting in a 200-character output.
** We would prefer to keep the size of the encoded string smaller than
** this.
**
** To minimize the encoding size, we first add a fixed offset value to each 
** byte in the sequence.  The addition is modulo 256.  (That is to say, if
** the sum of the original character value and the offset exceeds 256, then
** the higher order bits are truncated.)  The offset is chosen to minimize
** the number of characters in the string that need to be escaped.  For
** example, in the case above where the string was composed of 100 0x27
** characters, the offset might be 0x01.  Each of the 0x27 characters would
** then be converted into an 0x28 character which would not need to be
** escaped at all and so the 100 character input string would be converted
** into just 100 characters of output.  Actually 101 characters of output - 
** we have to record the offset used as the first byte in the sequence so
** that the string can be decoded.  Since the offset value is stored as
** part of the output string and the output string is not allowed to contain
** characters 0x00 or 0x27, the offset cannot be 0x00 or 0x27.
**
** Here, then, are the encoding steps:
**
**     (1)   Choose an offset value and make it the first character of
**           output.
**
**     (2)   Copy each input character into the output buffer, one by
**           one, adding the offset value as you copy.
**
**     (3)   If the value of an input character plus offset is 0x00, replace
**           that one character by the two-character sequence 0x01 0x01.
**           If the sum is 0x01, replace it with 0x01 0x02.  If the sum
**           is 0x27, replace it with 0x01 0x03.
**
**     (4)   Put a 0x00 terminator at the end of the output.
**
** Decoding is obvious:
**
**     (5)   Copy encoded characters except the first into the decode 
**           buffer.  Set the first encoded character aside for use as
**           the offset in step 7 below.
**
**     (6)   Convert each 0x01 0x01 sequence into a single character 0x00.
**           Convert 0x01 0x02 into 0x01.  Convert 0x01 0x03 into 0x27.
**
**     (7)   Subtract the offset value that was the first character of
**           the encoded buffer from all characters in the output buffer.
**
** The only tricky part is step (1) - how to compute an offset value to
** minimize the size of the output buffer.  This is accomplished by testing
** all offset values and picking the one that results in the fewest number
** of escapes.  To do that, we first scan the entire input and count the
** number of occurances of each character value in the input.  Suppose
** the number of 0x00 characters is N(0), the number of occurances of 0x01
** is N(1), and so forth up to the number of occurances of 0xff is N(255).
** An offset of 0 is not allowed so we don't have to test it.  The number
** of escapes required for an offset of 1 is N(1)+N(2)+N(40).  The number
** of escapes required for an offset of 2 is N(2)+N(3)+N(41).  And so forth.
** In this way we find the offset that gives the minimum number of escapes,
** and thus minimizes the length of the output string.
*/

/*
** Encode a binary buffer "in" of size n bytes so that it contains
** no instances of characters '\'' or '\000'.  The output is 
** null-terminated and can be used as a string value in an INSERT
** or UPDATE statement.  Use sqlite3_decode_binary() to convert the
** string back into its original binary.
**
** The result is written into a preallocated output buffer "out".
** "out" must be able to hold at least 2 +(257*n)/254 bytes.
** In other words, the output will be expanded by as much as 3
** bytes for every 254 bytes of input plus 2 bytes of fixed overhead.
** (This is approximately 2 + 1.0118*n or about a 1.2% size increase.)
**
** The return value is the number of characters in the encoded
** string, excluding the "\000" terminator.
*/
int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out){
  int i, j, e, m;
  int cnt[256];
  if( n<=0 ){
    out[0] = 'x';
    out[1] = 0;
    return 1;
  }
  memset(cnt, 0, sizeof(cnt));
  for(i=n-1; i>=0; i--){ cnt[in[i]]++; }
  m = n;
  for(i=1; i<256; i++){
    int sum;
    if( i=='\'' ) continue;
    sum = cnt[i] + cnt[(i+1)&0xff] + cnt[(i+'\'')&0xff];
    if( sum<m ){
      m = sum;
      e = i;
      if( m==0 ) break;
    }
  }
  out[0] = e;
  j = 1;
  for(i=0; i<n; i++){
    int c = (in[i] - e)&0xff;
    if( c==0 ){
      out[j++] = 1;
      out[j++] = 1;
    }else if( c==1 ){
      out[j++] = 1;
      out[j++] = 2;
    }else if( c=='\'' ){
      out[j++] = 1;
      out[j++] = 3;
    }else{
      out[j++] = c;
    }
  }
  out[j] = 0;
  return j;
}

/*
** Decode the string "in" into binary data and write it into "out".
** This routine reverses the encoding created by sqlite3_encode_binary().
** The output will always be a few bytes less than the input.  The number
** of bytes of output is returned.  If the input is not a well-formed
** encoding, -1 is returned.
**
** The "in" and "out" parameters may point to the same buffer in order
** to decode a string in place.
*/
int sqlite3_decode_binary(const unsigned char *in, unsigned char *out){
  int i, c, e;
  e = *(in++);
  i = 0;
  while( (c = *(in++))!=0 ){
    if( c==1 ){
      c = *(in++);
      if( c==1 ){
        c = 0;
      }else if( c==2 ){
        c = 1;
      }else if( c==3 ){
        c = '\'';
      }else{
        return -1;
      }
    }
    out[i++] = (c + e)&0xff;
  }
  return i;
}
