#include "FocusObjectElt.h"
#include "..\LogicDialog.h"

bool CFocusObject::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ExePipe.AutoTypeAB();   
	uint32 Type = GetTypeAB();             		
	if( !ExePipe.HasTypeAB(Type))   
	{
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("%I64ld: %s input data type checking failure:(%x,%x)"),m_ID,GetName().c_str(),Type,ExePipe.GetTypeAB());
		return true;
	}		

	tstring ObjectName = ExePipe.PopString();
	int64 InstanceID = Dialog->m_NamedObjectList.GetInstanceID(ObjectName);
	Dialog->m_ObjectFocus = InstanceID;
	if (InstanceID==0)
	{
		return false;
	}

	return true;
}


bool  CFocusObject_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	int64 InstanceID = Dialog->m_NamedObjectList.GetInstanceID(m_FocusName);
	Dialog->m_ObjectFocus = InstanceID;
	if (InstanceID==0)
	{
		return false;
	}

	return true;
};


bool  CFocusObject_Inter::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	Dialog->m_ObjectFocus = m_InstanceID;

	return true;
};
