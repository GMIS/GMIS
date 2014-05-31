/*
* ��λ��
* ����m���˾���n����λ
* author: ZhangHongBing(hongbing75@gmail.com)   
*/

#ifndef _SPACEMUTEX_H__
#define _SPACEMUTEX_H__

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

	uint32                   m_SeatNum;  //�趨��λ����=m_UseSeatList.size()+m_BlankSeatList,default = 2;

public:
	CSpaceMutex();
	virtual ~CSpaceMutex();

	//��ú�ʹ�õ��߳������
	void  SetSeatNum(int32 n){
		m_SeatNum = n>0? n:1;
	}

	virtual SeatMutex* CreateSeat()=0;

    virtual bool AcquireThis(void* User){
		
		SeatMutex* Seat = NULL;
		
		Acquire();

		//���ȼ��User�Ƿ��Ѿ�����
        map<void*,SeatMutex*>::iterator it = m_UseSeatList.find(User);
		if (it != m_UseSeatList.end())
		{
			Seat = it->second;
		}else if(m_BlankSeatList.size()){
			Seat = m_BlankSeatList.front();
			m_BlankSeatList.pop_front();
            m_UseSeatList[User] = Seat;
		}else{ //��λ������Ӧ����
			Seat = CreateSeat();
			assert(Seat);  //�������Ӧ�÷ǳ��ټ���Ϊ��λ������߳�����ȣ�Ŀǰû���������
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
			//��������Ӧ���ӵ���λ������Լ������λ��ʹ����Ϻ���ɾ��
			if (m_UseSeatList.size()+m_BlankSeatList.size()>m_SeatNum)
			{
				delete sm;
			} 
			else //�������հ���λ�б�ȴ�ʹ��
			{
				m_BlankSeatList.push_back(sm);
			}
			m_UseSeatList.erase(it);	
		}

		Release();
	};
};


}//namespace ABSTRACT 
#endif // _SPACEMUTEX_H__
