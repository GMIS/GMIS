
/*
* author: ZhangHongBing(hongbing75@gmail.com)  
*  
*
* root class based on energy-mass viewpoint, inherit it to get world
*/


#ifndef _ABSTRACTSPACE_H_
#define _ABSTRACTSPACE_H_

#include "Typedef.h"

using namespace ABSTRACT;
using namespace std;

#ifdef WIN32
#define UNICODE_16_BIT
#else 
#define UNICODE_32_BIT
#pragma message("!!! Are you sure the unicode char is 32 bit on your platform ?")
#endif

namespace ABSTRACT { 


AnsiString  WStoUTF8(const std::wstring& s);
std::wstring UTF8toWS(const AnsiString& s);

enum  SPACE_TYPE{     
		ENERGY_SPACE,
		MASS_SPACE, 
};


class CABTime;
class CABSpacePool;

class  AbstractSpace  
{

private:
	static CABTime*          m_StaticTimer;
    static CABSpacePool*  m_StaticSpacePool;

protected:
	AbstractSpace(){

	};
	AbstractSpace(CABTime* Timer,CABSpacePool* Pool)
	{
		m_StaticTimer = Timer;
		m_StaticSpacePool=Pool;
	}
public:	
	virtual ~AbstractSpace(){
		
	};

	static void InitTimer(CABTime* Timer);
	static void InitSpacePool(CABSpacePool* Pool);

	static int64 CreateTimeStamp();
	static CABTime*  GetTimer();

    static CABSpacePool* GetSpacePool();

    virtual SPACE_TYPE  SpaceType()=0;  
    virtual tstring GetName() = 0;

	//The interaction between space
	//virtual bool Do(AbstractSpace* Other) = 0;
};


}// namespace ABSTRACT

#endif // _ABSTRACTSPACE_H_

