/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _DLL_LOAD_H__
#define _DLL_LOAD_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Typedef.h"

//指定为unicode
typedef Mass*          (*pGetObjectFunc)(std::wstring Name,int64 ID);
typedef void           (*pDeleteObjectFunc)(Mass* p);
typedef const TCHAR*   (*pGetObjectDocFunc)();
typedef const DLL_TYPE (*pGetObjectType)();


//程序完成后自动释放dll,
class StaticHandle{
public:
	HINSTANCE       m_Handle;
	tstring         m_Doc;

	StaticHandle():m_Handle(NULL){};

	StaticHandle(const StaticHandle& Handle){
		StaticHandle& h = const_cast<StaticHandle&>(Handle);
		m_Handle = h.m_Handle;
		m_Doc = h.m_Doc;
		h.m_Doc = _T("");
		h.m_Handle = NULL;
	}
	StaticHandle& operator=(StaticHandle& h)
	{
		if(&h != this){
			m_Handle = h.m_Handle;
			m_Doc = h.m_Doc;
			h.m_Doc= _T("");
			h.m_Handle = NULL;
		}
		return *this;
	}

	bool IsValid(){ return m_Handle != NULL;};
    void LoadObject(const TCHAR* DllName){
		if(DllName!=NULL)
		     m_Handle  = LoadLibraryW (DllName);
	};

	std::wstring GetObjectDoc(){
		if(m_Handle == NULL
			&& m_Doc.size())return m_Doc;
         pGetObjectDocFunc pFunc = NULL;
         pFunc = ( pGetObjectDocFunc) GetProcAddress(m_Handle, "GetObjectDoc");	
		 if(pFunc !=NULL ){
			 m_Doc = (pFunc)();
		 }
		 return m_Doc;
	}

	Mass* GetObject(std::wstring Name,int64 ID=0){
         if(m_Handle == NULL)return NULL;
         pGetObjectFunc pFunc = NULL;
         pFunc = ( pGetObjectFunc) GetProcAddress(m_Handle, "CreateObject");
		 
		 Mass* Object = NULL;
		 if(pFunc !=NULL )Object =(pFunc)(Name,ID);
	     return Object;
	}

	void  DeleteObject(Mass* p)
	{
         if(m_Handle == NULL)return;
         pDeleteObjectFunc pFunc = NULL;
         pFunc = ( pDeleteObjectFunc) GetProcAddress(m_Handle, "DestroyObject");		 
		 if(pFunc !=NULL )( pFunc)(p);
	}

	~StaticHandle(){
		if(m_Handle)FreeLibrary(m_Handle);
	}
};

#endif // _DLL_LOAD_H__