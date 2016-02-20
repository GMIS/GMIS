/*
*
* author: ZhangHongBing(hongbing75@gmail.com)   
*/

#ifndef _SPACEMUTEX_H__
#define _SPACEMUTEX_H__

#include "UserMutex.h"
#include <deque>
#include <map>

using namespace std;

namespace PHYSIC{

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

class CSpaceMutex : public CUserMutex  
{

private:
	map<void*,SeatMutex*>    m_UseSeatList;
	deque<SeatMutex*>        m_BlankSeatList; 

	uint32                   m_SeatNum;  //Set total number of seats = m_UseSeatList.size()+m_BlankSeatList,default = 2;

public:
	CSpaceMutex();
	virtual ~CSpaceMutex();

	//It best  equal to the number of threads
	void  SetSeatNum(int32 n){
		m_SeatNum = n>0? n:1;
	}

	virtual SeatMutex* CreateSeat();

    virtual bool AcquireThis(void* User){
		
		SeatMutex* Seat = NULL;
		
		Acquire();

		//Check whether the User has sat down
        map<void*,SeatMutex*>::iterator it = m_UseSeatList.find(User);
		if (it != m_UseSeatList.end())
		{
			Seat = it->second;
		}else if(m_BlankSeatList.size()){
			Seat = m_BlankSeatList.front();
			m_BlankSeatList.pop_front();
            m_UseSeatList[User] = Seat;
		}else{ //Adaptive increase in seats
			Seat = CreateSeat();
			assert(Seat);  //This situation should be very rare, because the number of seats should be equal to the number of threads, so this exception no need to handle currently
			if(Seat==NULL){
			    Release();
				return FALSE;
			}
			m_UseSeatList[User] = Seat;
		}
		++Seat->RefNum;
		Release(); 

        Seat->Mutex->Acquire();
#if _DEBUG
		Seat->Mutex->m_Memo = m_Memo;
#endif
		
		return TRUE;

	};
	virtual void ReleaseThis(void* User){
		Acquire();
		
        map<void*,SeatMutex*>::iterator it = m_UseSeatList.find(User);
		assert(it != m_UseSeatList.end());
		SeatMutex* sm = it->second;
	    
		sm->Mutex->Release();
		--sm->RefNum;

		if (sm->RefNum==0)
		{
			m_UseSeatList.erase(it);	
			
			//The increased  seat ,if it is greater than the number of appointed seats, will be  deleted after used 
			if (m_UseSeatList.size()+m_BlankSeatList.size()>m_SeatNum)
			{
				delete sm;
			} 
			else //Or put it into a list of empty seats to  wait for use 
			{
				m_BlankSeatList.push_back(sm);
			}
		}

		Release();
	};
};


}//namespace ABSTRACT 
#endif // _SPACEMUTEX_H__
