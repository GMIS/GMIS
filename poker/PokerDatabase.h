// PokerDatabase.h: interface for the CPokerDatabase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GameDatabase_H__A6CF1AE9_BDD4_473E_B9D0_C581FF99161E__INCLUDED_)
#define AFX_GameDatabase_H__A6CF1AE9_BDD4_473E_B9D0_C581FF99161E__INCLUDED_

#include "TheorySpace.h"
#include "CppSQLite3.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define POKER_TABLE  "Object"

enum OBJECTTYPE{BUTTON_OBJECT, NUM_OBJECT, TEXT_OBJECT,LABEL_OBJECT};

class CObjectData : public ePipeline
{
public:
	CObjectData();
	virtual ~CObjectData();

	CObjectData(const CObjectData& C) { 
		CObjectData& temp = const_cast<CObjectData&>(C);
		*this << temp;
		m_ID    = C.m_ID;
		m_Label = C.m_Label;
		assert(temp.Size()==0);
	};
	CObjectData& operator=(const CObjectData& C ){
		if(this != &C){
			CObjectData& temp = const_cast<CObjectData&>(C);
			*this << temp;
			assert(temp.Size()==0);
			m_ID    = C.m_ID;
			m_Label = C.m_Label;
		}
		return *this;
	}
	CObjectData& operator << (CObjectData& C){
		ePipeline* p1 = (ePipeline*)this;
		ePipeline* p2 = (ePipeline*)&C;
		*p1<<*p2;
		assert(C.Size()==0);
		return *this;
	}

	void Reset();
	void SetImage(HBITMAP hBitmap);
	HBITMAP GetImage(){
		if (Size()==10)
		{
			uint32  handle   = *(uint32*)GetData(9);
			HBITMAP hBitmap  = (HBITMAP)handle;
			return hBitmap;
		}
		return NULL;
	}
};


class CPokerDatabase  
{
protected:
	CppSQLite3DB   m_Database;

public:
	CPokerDatabase();
	virtual ~CPokerDatabase();

	BOOL Open(AnsiString DBFileName);
	void GetObjectList(tstring FileName,vector<CObjectData>& ObjectList);
	void GetObjectList(vector<CObjectData>& ObjectList);

	void SaveObject(CObjectData& ObjectData);
	BOOL GetObject(int64 ID,tstring& Name,CObjectData& ObjectData);
	void DeleteObject(tstring& Name);
	void ClearObject();

	void SaveCrc32(uint32 crc32, tstring ocrStr,HBITMAP hBitmap);
	void UpdateCrc32(uint32 crc32,  tstring ocrStr);
	tstring GetCrc32(uint32 crc32);
	void GetCrc32List(ePipeline& Pipe);
    void ClearCrc32();
};

#endif // !defined(AFX_GameDatabase_H__A6CF1AE9_BDD4_473E_B9D0_C581FF99161E__INCLUDED_)
