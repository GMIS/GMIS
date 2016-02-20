// ABSpacePool.cpp: implementation of the CABSpacePool class.
//
//////////////////////////////////////////////////////////////////////

#include "AbstractSpace.h"
#include "ABSpacePool.h"
#include <algorithm>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CABSpacePool::CABSpacePool(CABMutex* Mutex)
:m_Mutex(Mutex),m_MaxTypeNum(20),m_MaxReserveNum(1000)
{

}

CABSpacePool::~CABSpacePool()
{
	std::map<size_t,AbstractSpaceList>::iterator it = m_AbstractSpacePool.begin();
	while (it != m_AbstractSpacePool.end())
	{
		AbstractSpaceList& List = it->second;
		if (List.size())
		{
			AbstractSpaceList::iterator it2 = List.begin();
			while(it2 != List.end()){
				AbstractSpace* Space = *it2;
				free(Space);
				it2++;
			}
		}
		it++;
	}
}
void CABSpacePool::SetMaxReserveNum(uint32 n){
	_CLOCK(m_Mutex);
	m_MaxReserveNum = n;
};

uint32 CABSpacePool::GetMaxReserveNum(){
	_CLOCK(m_Mutex);
    return m_MaxReserveNum;
};

void CABSpacePool::SetMaxTypeNum(uint32 n){
	_CLOCK(m_Mutex);
	m_MaxTypeNum = n;	
}
uint32 CABSpacePool::GetMaxTypeNum(){
	_CLOCK(m_Mutex);
	return m_MaxTypeNum;
}

AbstractSpace* CABSpacePool::RequireAbstractSpace(int32 Type){
	_CLOCK(m_Mutex);
	std::map<size_t,AbstractSpaceList>::iterator it = m_AbstractSpacePool.find(Type);
	if (it != m_AbstractSpacePool.end())
	{
		AbstractSpaceList& List = it->second;
		if (List.size())
		{
			AbstractSpace* Space = List.front();
            List.pop_front();
			memset(Space,0,Type);
			return Space;
		}
	}
	return NULL;
}

void  CABSpacePool::ReleaseAbstractSpace(int32 Type,AbstractSpace* Space){
	if (m_Mutex==NULL) 
	{
		free(Space);
		return;
	}
	_CLOCK(m_Mutex);
	//memset(Space,0,Type); i don't know why it lead to the deleting fail
	std::map<size_t,AbstractSpaceList>::iterator it = m_AbstractSpacePool.find(Type);
	if (it != m_AbstractSpacePool.end())
	{
		AbstractSpaceList& List = it->second;
#ifdef _DEBUG
		AbstractSpaceList::iterator it1 = lower_bound(List.begin(),List.end(),Space);
		if (it1 != List.end() ) 
		{
			it1++;
			if(it1 !=List.end() && Space==*it1){
				assert(0); //It means that the same pointer was twice deleted
				return;
			}else{
				List.insert(it1,Space);
			}
		}else{
			List.insert(it1,Space);
		}
#else
		List.push_back(Space);		
#endif
		int n= List.size()- m_MaxReserveNum;
		if (n>0)
		{
			do{
               AbstractSpace* Space = List.front();
			   free(Space);
	           List.pop_front();
			}while(--n>0);
		}
	}else{
		if(m_AbstractSpacePool.size()<m_MaxTypeNum){
			AbstractSpaceList& List = m_AbstractSpacePool[Type];
			List.push_back(Space);
		}else{
			assert(0);
		}
	}
}
