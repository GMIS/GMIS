#include "GetDate.h"


CGetDate::CGetDate(int64 ID):Mass(ID){

};
CGetDate::~CGetDate(){

};

tstring  CGetDate::GetName(){ 
	return _T("GetDate");
};
TypeAB  CGetDate::GetTypeAB(){
	return 0x00000000;
} 
bool CGetDate::Do(Energy* E){
	int64 CurentTime = AbstractSpace::CreateTimeStamp();
	tstring s = AbstractSpace::GetTimer()->GetYMD(CurentTime);
	ePipeline* Pipe = (ePipeline*)E;
	Pipe->PushString(s);
	return true;
};

