// MouseWheelMgr.h: interface for the CMouseWheelMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOUSEWHEELMGR_H__6738593F_D10B_45D9_ACA0_335DA7C0F630__INCLUDED_)
#define AFX_MOUSEWHEELMGR_H__6738593F_D10B_45D9_ACA0_335DA7C0F630__INCLUDED_

#include "WSScrollView.h"
#include "HookMgr.h"

class CMouseWheelMgr : public CHookMgr<CMouseWheelMgr>  
{
   friend class CHookMgr<CMouseWheelMgr>;

public:
	virtual ~CMouseWheelMgr();
 	static BOOL Initialize();

protected:
	CMouseWheelMgr();
 	static CMouseWheelMgr& Instance() { return CHookMgr<CMouseWheelMgr>::GetInstance(); }

  	virtual BOOL OnMouse(UINT uMouseMsg, const _MOUSEHOOKSTRUCTEX& info);
};

#endif // !defined(AFX_MOUSEWHEELMGR_H__6738593F_D10B_45D9_ACA0_335DA7C0F630__INCLUDED_)
