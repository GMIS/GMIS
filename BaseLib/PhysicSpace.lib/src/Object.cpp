#include "Object.h"


namespace PHYSIC{

Object::Object(CABTime* Timer,CABSpacePool* Pool):
Mass(Timer,Pool),
	m_Name(_T("MainObject")),
	m_Alive(FALSE),
	m_SleepTime(50)
{
#if defined(USING_POCO)
	AnsiString ThreadName = "ObjectMainThread";
	m_ObjectDefaultThread.setName(ThreadName);
#endif
}

Object::Object()
		:m_Name(_T("Object")),
		m_Alive(FALSE),
		m_SleepTime(50)
{
#if defined(USING_POCO)
	AnsiString ThreadName = "ObjectMainThread";
	m_ObjectDefaultThread.setName(ThreadName);
#endif	
};

Object::~Object(){
	if (m_Alive)
	{
		Dead();
	}
};

bool Object::Activate(){
	if(m_Alive)return TRUE;

	m_Alive  = TRUE;
#if defined(USING_POCO)
	m_ObjectDefaultThread.start(ObjectDefaultThreadFunc,(void*)this);

#elif defined(USING_WIN32)
	uint32 ThreadID = 0;
	m_hObjectDefaultThread = (HANDLE)_beginthreadex(NULL, // Security
		0,							                     // Stack size - use default
		ObjectDefaultThreadFunc,     		
		(void*)this,	      
		0,					                			 // Init flag
		&ThreadID);					                     // Thread address

	assert( m_hObjectDefaultThread != NULL );        

	m_hObjectDefaultThreadID = ThreadID;
#endif

	return TRUE;
}
void Object::Dead(){
	m_Alive = FALSE;
#if defined(USING_POCO)
	m_ObjectDefaultThread.join();
#elif defined(USING_WIN32)
	WaitForSingleObject (m_hObjectDefaultThread, INFINITE);
#endif
}

} //name PHYSIC