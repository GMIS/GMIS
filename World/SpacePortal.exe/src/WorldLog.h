/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _WORLDLOG_H__
#define _WORLDLOG_H__


#include "PhysicSpace.h"
#include "WSListView.h"

using namespace VISUALSPACE;

class InfoItem : public CVSpace2{
public:
	tstring  m_Text;
public:
	InfoItem(){
	};
	InfoItem(int64 ID,const TCHAR* text);
	virtual ~InfoItem(){};

	virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
};

#define ADD_INFO 1
#define CLR_INFO 2

class CWorldLog : public CWSListView  
{
	int32             m_MaxItemNum; //default = 200;

public:
	CWorldLog();
	virtual ~CWorldLog();

	InfoItem* AddInfo(const TCHAR* text);
	void      ClearAllItem();

protected:
    virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);

};

#endif // _WORLDLOG_H__