/* author: ZhangHongBing(hongbing75@gmail.com) 
*  a memory pool, define _USE_SPACE_POOL before using
*  referto BaseEnergy.h
*/
#ifndef _ABSTRACTSPACEPOOL_H__
#define _ABSTRACTSPACEPOOL_H__


#include <map>
#include <list>
#include <set>
#include "ABMutex.h"

namespace ABSTRACT{

class AbstractSpace;
	
class CAbstractSpacePool  
{
protected:
	CABMutex*   m_Mutex;

	uint32  m_MaxTypeNum;    //default = 20;
	uint32  m_MaxReserveNum; //default= 1000

	typedef std::list<AbstractSpace*> AbstractSpaceList;
	std::map<size_t,AbstractSpaceList> m_AbstractSpacePool;

public:
	CAbstractSpacePool(CABMutex* Mutex);
	virtual ~CAbstractSpacePool();

	AbstractSpace* RequireAbstractSpace(int32 Type);
    void  ReleaseAbstractSpace(int32 Type,AbstractSpace* Space);

	void SetMaxTypeNum(uint32 n);
	uint32 GetMaxTypeNum();

	void SetMaxReserveNum(uint32 n);
	uint32 GetMaxReserveNum();

};

#ifdef _USE_SPACE_POOL

#define SUPPORT_ABSTRACT_SAPCE_POOL(ClassName)\
 public:\
	void* operator new(size_t size){\
		assert(sizeof(ClassName)==size);\
		void* Space =  (void*)(AbstractSpace::GetSpacePool()->RequireAbstractSpace(size));\
		if (Space)return Space; \
		Space = ::operator new(size);\
		return Space;\
	};\
	void operator delete(void* p){\
		try{\
			ClassName* ptr = (ClassName*)p;\
			ptr->~ClassName();\
		}catch(...){\
			return;\
		}\
		size_t size = sizeof(ClassName);\
		AbstractSpace::GetSpacePool()->ReleaseAbstractSpace(size,(AbstractSpace*)p);\
	};

#else
	#define SUPPORT_ABSTRACT_SAPCE_POOL(ClassName) 
#endif

}
#endif // !defined  _ABSTRACTSPACEPOOL_H__
