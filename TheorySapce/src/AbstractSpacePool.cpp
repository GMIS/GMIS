// AbstractSpacePool.cpp: implementation of the CAbstractSpacePool class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "AbstractSpace.h"
#include "AbstractSpacePool.h"
#include <algorithm>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAbstractSpacePool::CAbstractSpacePool(CABMutex* Mutex)
:m_Mutex(Mutex),m_MaxReserveNum(1000),m_MaxTypeNum(20)
{

}

CAbstractSpacePool::~CAbstractSpacePool()
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
				free(Space); //加入链表的指针已经被析构过了
				it2++;
			}
		}
		it++;
	}
}
void CAbstractSpacePool::SetMaxReserveNum(uint32 n){
	CLock lk(m_Mutex);
	m_MaxReserveNum = n;
};

uint32 CAbstractSpacePool::GetMaxReserveNum(){
	CLock lk(m_Mutex);
    return m_MaxReserveNum;
};

void CAbstractSpacePool::SetMaxTypeNum(uint32 n){
	CLock lk(m_Mutex);
	m_MaxTypeNum = n;	
}
uint32 CAbstractSpacePool::GetMaxTypeNum(){
	CLock lk(m_Mutex);
	return m_MaxTypeNum;
}

AbstractSpace* CAbstractSpacePool::RequireAbstractSpace(int32 Type){
	CLock lk(m_Mutex);
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

void  CAbstractSpacePool::ReleaseAbstractSpace(int32 Type,AbstractSpace* Space){
	if (m_Mutex==NULL) //奇怪，似乎SpacePool已经析构了，还有人调用此函数
	{
		free(Space);
		return;
	}
	CLock lk(m_Mutex);
	//memset(Space,0,Type);  导致最后delete 失败
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
				assert(0); //找到相同的，意味着程序两次delete同一个指针
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