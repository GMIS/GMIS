/*
* author: ZhangHongBing(hongbing75@gmail.com)   
*/

#ifndef _SPACEMUTEX_H__
#define _SPACEMUTEX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ABMutex.h"
#include <deque>
#include <map>

using namespace std;

namespace ABSTRACT{

struct SeatMutex{
	CABMutex*    Mutex;
	int32        RefNum;
	SeatMutex():Mutex(NULL),RefNum(0){};
	~SeatMutex(){
		if (Mutex)
		{
			delete Mutex;
			Mutex = NULL;
		}
	}
};

class CSpaceMutex : public CABMutex  
{

private:
	map<void*,SeatMutex*>    m_UseSeatList;
	deque<SeatMutex*>        m_BlankSeatList; 

	uint32                   m_SeatNum;  //设定座位总数=m_UseSeatList.size()+m_BlankSeatList,default = 2;

public:
	CSpaceMutex();
	virtual ~CSpaceMutex();

	//最好和使用的线程数相等
	void  SetSeatNum(int32 n){
		m_SeatNum = n>0? n:1;
	}

	virtual SeatMutex* CreateSeat()=0;

    virtual bool AcquireThis(void* User){
		
		SeatMutex* Seat = NULL;
		
		Acquire();

		//首先检查User是否已经坐下
        map<void*,SeatMutex*>::iterator it = m_UseSeatList.find(User);
		if (it != m_UseSeatList.end())
		{
			Seat = it->second;
		}else if(m_BlankSeatList.size()){
			Seat = m_BlankSeatList.front();
			m_BlankSeatList.pop_front();
            m_UseSeatList[User] = Seat;
		}else{ //座位会自适应增加
			Seat = CreateSeat();
			assert(Seat);  //这种情况应该非常少见，因为座位数会与线程数相等，目前没处理此例外
			if(Seat==NULL){
			    Release();
				return FALSE;
			}
			m_UseSeatList[User] = Seat;
		}
		++Seat->RefNum;
		Release(); 

        Seat->Mutex->Acquire();
		return TRUE;

	};
	virtual void ReleaseThis(void* User){
		Acquire();
		
        map<void*,SeatMutex*>::iterator it = m_UseSeatList.find(User);
		assert(it != m_UseSeatList.end());
		SeatMutex* sm = it->second;
	    --sm->RefNum;

		sm->Mutex->Release();
		
		if (sm->RefNum==0)
		{
			//对于自适应增加的座位如果大于约定的座位数，使用完毕后则删除
			if (m_UseSeatList.size()+m_BlankSeatList.size()>m_SeatNum)
			{
				delete sm;
			} 
			else //否则放入空白座位列表等待使用
			{
				m_BlankSeatList.push_back(sm);
			}
			m_UseSeatList.erase(it);	
		}

		Release();
	};
};


}//namespace ABSTRACT 
#endif // !defined(AFX_SPACEMUTEX_H__130DB3A1_8912_48B6_BFB6_676BC58287CC__INCLUDED_)
