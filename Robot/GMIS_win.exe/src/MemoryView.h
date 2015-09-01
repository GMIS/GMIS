/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _MEMORYVIEW_H__
#define _MEMORYVIEW_H__


#include "WSTreeView.h"

using namespace VISUALSPACE;



#define INFO_PROC    1001

#define  BNT_CLOSE_ONE      100
#define  BNT_CLOSE_ALL      101


class CMemoryView : public CWSTreeView  
{
	
public:
	class CTitle: public CVSpace2{
	public:
		int32    m_ObjectCount;   //计数器，总是增长
		tstring  m_Text;
	
	public:
		CTitle(int64 ID, const tstring& Name);
        virtual ~CTitle(){};
		
		virtual void Draw(HDC hDC,ePipeline* Pipe /* = NULL */);
	};

	class CTextItem : public CVSpace2{
	public:
		tstring   m_Text;
		int       m_Index;
	public:
		CTextItem(int Index,int64 ID,tstring Text);
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
	};


protected:

	CTextItem* Data2Item(int Index,Energy* Data);

	void CreateObjectInstance(ePipeline* ObjData);
    void NameObjectInstance(tstring SrcObjectName,int64 InstanceID,tstring InstanceName);
	void CloseObjectInstance(tstring SrcObjectName,int64 InstanceID);


	void CreateDataInstance(tstring InstanceName);
	void InsertData(tstring InstanceName,int32 Index,Energy* Data);
	void ModifyData(tstring InstanceName,int32 Index,Energy* Data);
	void RemoveData(tstring InstanceName,int32 Index);
    void CloseDataInstance(tstring InstanceName);
	void ImportData(tstring InstanceName,ePipeline* DataList);
	void ExportData(tstring InstanceName);
public:
	CMemoryView();
	virtual ~CMemoryView();


	void InfoProc(ePipeline& Info);
	
	void ResetData(ePipeline& DataList);

protected:
	virtual void Layout(bool Redraw =true);
	void Draw(HDC hDC, ePipeline* Pipe=NULL);
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	virtual LRESULT ToolbarReaction(ButtonItem* Bnt);

	LRESULT OnInfoProc(WPARAM wParam, LPARAM lParam);
	
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnRButtnDown(WPARAM wParam, LPARAM lParam); 
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(_MEMORYVIEW_H__)
