#include "GetTime.h"

CGetTime::CGetTime(int64 ID)
	:Mass(ID){

};
CGetTime::~CGetTime(){};

tstring CGetTime::GetName(){
	return _T("GetTime");
};

TypeAB  CGetTime::GetTypeAB(){ 
	return 0x00000000;
}

bool CGetTime::Do(Energy* E){
	int64 CurentTime = AbstractSpace::CreateTimeStamp();
	tstring s = AbstractSpace::GetTimer()->GetHMS(CurentTime);
	ePipeline* Pipe = (ePipeline*)E;
	Pipe->PushString(s);
	return true;
};