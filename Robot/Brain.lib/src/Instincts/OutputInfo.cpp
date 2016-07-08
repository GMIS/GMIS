#include "OutputInfo.h"
#include "..\LogicDialog.h"

COutputInfo::COutputInfo(int64 ID)
	:CElement(ID,_T("Output Info")){

};

COutputInfo::~COutputInfo(){

};

TypeAB  COutputInfo::GetTypeAB(){ 
	return 0x30000000;
};

bool COutputInfo::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	if (ExePipe.Size()==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find any string info in ExePipe"));
		return false;
	}
	Energy* e = ExePipe.GetEnergy(0);
	if(e->EnergyType() != TYPE_STRING){
		ExePipe.GetLabel() = Format1024(_T("Error: not find valid string info in ExePipe"));
		return false;
	}
	tstring Info = ExePipe.PopString();
	Dialog->RuntimeOutput(m_ID,Info);
	return true;
};