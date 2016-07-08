#include "NameObjectElt.h"
#include "..\LogicDialog.h"

CNameObject::CNameObject(int64 ID)
	:CElement(ID,_T("Name object"))
{

}

CNameObject::~CNameObject(){

};

bool CNameObject::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ePipeline* ObjectInstance = Dialog->FindObjectInstance(Dialog->m_ObjectFocus);
	if (ObjectInstance==NULL)
	{
		return false;
	}

	tstring SrcObjectName = *(tstring*)ObjectInstance->GetData(0);

	tstring InstanceName = ExePipe.PopString();

	if (Dialog->m_NamedObjectList.HasName(Dialog,InstanceName))
	{
		return false;
	}
	Dialog->m_NamedObjectList.RegisterNameByTask(InstanceName,Dialog->m_TaskID,Dialog->m_ObjectFocus);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(NAME_INSTANCE);
	nf.PushString(SrcObjectName);
	nf.PushInt(Dialog->m_ObjectFocus);
	nf.PushString(InstanceName);
	nf.Notify(Dialog);

	return true;
}

CNameObject_Static::CNameObject_Static(int64 ID,tstring Name)
	:CElement(ID,_T("Name object")),m_Name(Name)
{

}

CNameObject_Static::~CNameObject_Static(){

};

bool CNameObject_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ePipeline* ObjectInstance = Dialog->FindObjectInstance(Dialog->m_ObjectFocus);
	if (ObjectInstance==NULL)
	{
		return false;
	}

	tstring SrcObjectName = *(tstring*)ObjectInstance->GetData(0);

	tstring& InstanceName = m_Name;

	if (Dialog->m_NamedObjectList.HasName(Dialog,InstanceName))
	{
		return false;
	}
	Dialog->m_NamedObjectList.RegisterNameByTask(InstanceName,Dialog->m_TaskID,Dialog->m_ObjectFocus);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(NAME_INSTANCE);
	nf.PushString(SrcObjectName);
	nf.PushInt(Dialog->m_ObjectFocus);
	nf.PushString(InstanceName);
	nf.Notify(Dialog);

	return true;
}