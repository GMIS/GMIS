/*
* 用独立线程绘制场景
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _OPENGLWIN_H__
#define _OPENGLWIN_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "PhysicSpace.h"
#include <gl\gl.h>
#include <gl\glu.h>
#include "VisibleSpace.h"
#include "WinSpace2.h"

namespace VISUALSPACE{

class CTGLWin : public Object, public CWinSpace2  
{

protected:
	CLockPipe*     m_LocalNerve;   //Windows 与 Do()的之间的私有通讯管道；
	       
	void SendLocalMessage(UINT Msg,int64 wParam, int64 lParam,CVSpace2* Space){
		    assert(m_LocalNerve);
		    ePipeline* Pipe = new ePipeline(Msg);
			Pipe->PushInt(wParam);
			Pipe->PushInt(lParam);
			Pipe->PushInt((int64)Space);
			m_LocalNerve->Push(Pipe);
	}

    bool GetLocalMessage(SpaceRectionMsg& SRM){
         assert(m_LocalNerve); 
		 if (m_LocalNerve->Size()==0)return false;
		 eElectron E;
		 m_LocalNerve->Pop(&E);
		 if(E.EnergyType() != TYPE_PIPELINE)false;
         ePipeline* Pipe = (ePipeline*)E.Value();
		 if(Pipe->Size() != 3)return false;
         SRM.Msg = (uint32)Pipe->GetID();
		 SRM.wParam = Pipe->PopInt();
         SRM.lParam = Pipe->PopInt();
		 SRM.ChildAffected = (CVSpace2*)Pipe->PopInt();
		 SRM.WinSpace = this;
		 return true;
	}

protected:
	/*你只需要根据情况重载一下函数
	 GUI thread        Reaction(){ ...  };
	                       ||  
	 ----------------------||---------------------
     Object thread         ||m_LocalNerve
	                       ||
	 bool Do(Energy* E){   ||
	    InitScene();          ||
		while(alive){      \/
		               GLRection(); 
			           RenderScene(); 
		};
	 }
	*/
	virtual bool InitScene();
	virtual void RenderScene();

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
 	LRESULT OnSize(WPARAM wParam, LPARAM lParam){ SendLocalMessage(WM_SIZE,wParam,lParam,NULL); return 0;};
  
	virtual void GLReaction();
	void    OnGLSceneSize(int width, int height); //WM_SIZE

	virtual bool Do(Energy* E);
	
public:
	CTGLWin();
	virtual ~CTGLWin();
	
};

};

#endif // _OPENGLWIN_H__
