/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"


class  CCreateMemory : public CElement  
{
	tstring  m_MemoryName;
public:
	CCreateMemory(int64 ID,tstring MemoryName)
		:CElement(ID,_T("Create Memory")),m_MemoryName(MemoryName){
	};
	virtual ~CCreateMemory(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};


class CFocusMemory: public CElement
{
public:
	CFocusMemory(int64 ID)
		:CElement(ID,_T("Focus Memory")){
	};;
	virtual ~CFocusMemory(){};

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};

//静态版本，即focus memory名字直接由参数提供
class CFocusMemoryStatic: public CElement
{
	tstring m_FocusName;
public:
	CFocusMemoryStatic(int64 ID,tstring FocusName)
		:CElement(ID,_T("Focus Memory")),m_FocusName(FocusName){
	};;
	virtual ~CFocusMemoryStatic(){};

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};
class  CGetMemoryFocus : public CElement  
{
public:
	CGetMemoryFocus(int64 ID)
		:CElement(ID,_T("Get Memory Focus")){
	};
	virtual ~CGetMemoryFocus(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class  CSetMemoryAddress : public CElement  
{
public:
	CSetMemoryAddress(int64 ID)
		:CElement(ID,_T("Set Memory Address")){
	};
	virtual ~CSetMemoryAddress(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};


class  CGetMemoryAddress : public CElement  
{
public:
	CGetMemoryAddress(int64 ID)
		:CElement(ID,_T("Get Memory Address")){
	};
	virtual ~CGetMemoryAddress(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class  CCreateMemoryNode : public CElement  
{
public:
	CCreateMemoryNode(int64 ID)
		:CElement(ID,_T("Create Memory Node")){
	};
	virtual ~CCreateMemoryNode(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class  CImportMemory : public CElement  
{
public:
	CImportMemory(int64 ID)
		:CElement(ID,_T("Import memory")){
	};	

	virtual ~CImportMemory(){
	};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};	
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};

class  CExportMemory : public CElement  
{
public:
	CExportMemory(int64 ID)
		:CElement(ID,_T("Export memory")){
	};	

	virtual ~CExportMemory(){
	};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};	
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};



class  CGetMemory : public CElement  
{
public:
	CGetMemory(int64 ID)
		:CElement(ID,_T("Get memory")){
	};	

	virtual ~CGetMemory(){};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};
class  CGetMemorySize : public CElement  
{
public:
	CGetMemorySize(int64 ID)
		:CElement(ID,_T("Get memory size")){
	};	

	virtual ~CGetMemorySize(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class  CInsertMemory : public CElement  
{
public:
	CInsertMemory(int64 ID)
		:CElement(ID,_T("Insert memory")){
	};	

	virtual ~CInsertMemory(){
	};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};	
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};


class CModifyMemory: public CElement  
{
public:
	CModifyMemory(int64 ID)
		:CElement(ID,_T("Modify memory")){
	};	

	virtual ~CModifyMemory(){
	};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};	
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};

class  CRemoveMemory : public CElement  
{
public:
	CRemoveMemory(int64 ID)
		:CElement(ID,_T("Remove memory")){
	};	

	virtual ~CRemoveMemory(){};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class  CCloseMemory : public CElement  
{
public:
	CCloseMemory(int64 ID)
		:CElement(ID,_T("Close memory")){
	};	

	virtual ~CCloseMemory(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};


