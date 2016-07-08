#include "MemoryDataElt.h"
#include "..\LogicDialog.h"

bool  CCreateMemory::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 


	int64 InstanceID = AbstractSpace::CreateTimeStamp();
	ePipeline Pipe(m_MemoryName.c_str(),0);
	Dialog->AddMemoryInstance(InstanceID,Pipe);
	Dialog->m_MemoryFocus = InstanceID;
	Dialog->m_NamedMemoryList.RegisterNameByTask(m_MemoryName,Dialog->m_TaskID,InstanceID);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_DATA);
	nf.PushInt(CREATE_INSTANCE);
	nf.PushString(m_MemoryName);
	nf.Notify(Dialog);

	return true;
}


bool  CFocusMemory::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	if (ExePipe.Size()==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find any focus memory name in ExePipe"));
		return false;
	}
	Energy* e = ExePipe.GetEnergy(0);
	if(e->EnergyType() != TYPE_STRING){
		ExePipe.GetLabel() = Format1024(_T("Error: not find valid focus memory name in ExePipe"));
		return false;
	}
	tstring MemoryName = ExePipe.PopString();
	int64 InstanceID = Dialog->m_NamedMemoryList.GetInstanceID(MemoryName);
	Dialog->m_MemoryFocus = InstanceID;
	if (InstanceID==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find focus memory '%s'"),MemoryName.c_str());
		return false;
	}


	return true;
}

bool  CFocusMemoryStatic::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{

	int64 InstanceID = Dialog->m_NamedMemoryList.GetInstanceID(m_FocusName);
	Dialog->m_MemoryFocus = InstanceID;
	if (InstanceID==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find focus memory(%s)"),m_FocusName.c_str());
		return false;
	}
	return true;
}

bool CGetMemoryFocus::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	if (Dialog->m_MemoryFocus==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find focus memory"));
		return false;
	}
	tstring Name = Dialog->m_NamedMemoryList.GetInstanceName(Dialog->m_MemoryFocus);
	if(Name.size()==0){
		ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
		if(MemoryInstance==0){
			Dialog->m_MemoryFocus=0;
			ExePipe.GetLabel() = Format1024(_T("Error: the focus memory invalid"));
		}else{
			ExePipe.GetLabel() = Format1024(_T("Error: the focus memory name losed"));
		}
		return false;
	}
	ExePipe.PushString(Name);
	return true;
}
bool CSetMemoryAddress::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	for(int i=0; i<ExePipe.Size();i++){
		ENERGY_TYPE Type = ExePipe.GetEnergy(i)->EnergyType();
		if(Type != TYPE_INT){
			ExePipe.GetLabel() = Format1024(_T("Error: Memory address invalid"));
			return false;
		}
	}
	Dialog->m_MemoryAddress.Clear();
	Dialog->m_MemoryAddress << ExePipe;
	return true;
}

bool CGetMemoryAddress::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	ExePipe << Dialog->m_MemoryAddress;
	return true;
}

bool CCreateMemoryNode::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
	if (MemoryInstance==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: no find the focus memory instance"));
		return false;
	}
	tstring& InstanceName = MemoryInstance->GetLabel();

	ePipeline* ParentPipe = MemoryInstance;
	Energy*    Target = NULL;
	int64 AddressLength = Dialog->m_MemoryAddress.Size();
	if (AddressLength==0)
	{
		Energy* e = new ePipeline;
		if(!e){
			ExePipe.GetLabel() = Format1024(_T("Error: create new memory node failed"));
			return false;
		}
		ParentPipe->InsertEnergy(0,e);
		ePipeline Address;
		Address.PushInt(0);

		CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
		nf.PushInt(INSTANCE_DATA);
		nf.PushInt(INSERT_DATA);
		nf.PushString(InstanceName);
		nf.PushPipe(Address);
		nf.Push_Directly(e->Clone());

		nf.Notify(Dialog);

		return true;
	}
	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Dialog->m_MemoryAddress.GetData(i);
		if(Index<0 || Index>ParentPipe->Size()){
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid"));
			return false;
		}
	    
		if(i==AddressLength-1){ //最后一个地址
			Energy* e = new ePipeline;
			if(!e){
				ExePipe.GetLabel() = Format1024(_T("Error: create new memory node failed"));
				return false;
			}
			ParentPipe->InsertEnergy(Index,e);

			CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
			nf.PushInt(INSTANCE_DATA);
			nf.PushInt(INSERT_DATA);
			nf.PushString(InstanceName);
			nf.Push_Directly(Dialog->m_MemoryAddress.Clone());
			nf.Push_Directly(e->Clone());

			nf.Notify(Dialog);

			return true;
		}

		Target = ParentPipe->GetEnergy(Index);
		if(Target->EnergyType()==TYPE_PIPELINE){
			ParentPipe = (ePipeline*)Target->Value();
		}else{
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid"));
			return false;
		} 
	}
	
	ExePipe.GetLabel() = Format1024(_T("Error: memory address out of instance"));
	return false;
		
}

bool  CImportMemory::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
	if (MemoryInstance==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: no find the focus memory instance"));
		return false;
	}

	if(ExePipe.Size()==0){
		return true;
	}
	tstring& InstanceName = MemoryInstance->GetLabel();

	ePipeline* ParentPipe = MemoryInstance;
	Energy*    Target = NULL;
	int64 AddressLength = Dialog->m_MemoryAddress.Size();

	if (AddressLength==0)
	{
		ePipeline DataList = ExePipe;
		int InsertIndex = ParentPipe->Size();
		ePipeline InsertAddress;

		InsertAddress.PushInt(InsertIndex);

		*ParentPipe << ExePipe;

		CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
		nf.PushInt(INSTANCE_DATA);
		nf.PushInt(IMPORT_DATA);
		nf.PushString(InstanceName);
		nf.PushPipe(InsertAddress);
		nf.PushPipe(DataList);

		nf.Notify(Dialog);
		return true;
	}

	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Dialog->m_MemoryAddress.GetData(i);
		if(Index<0 || Index>ParentPipe->Size()){
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid in memory instance %s"),InstanceName.c_str());
			return false;
		}

		if(i==AddressLength-1){ //最后一个地址
			if(Index>=ParentPipe->Size()){
				ExePipe.GetLabel() = Format1024(_T("Error: the memroy address pointed target  %s is not a node(ePipeline)"),InstanceName.c_str());
				return false;
			}
			Target = ParentPipe->GetEnergy(Index);
			if (Target->EnergyType() != TYPE_PIPELINE)
			{
				ExePipe.GetLabel() = Format1024(_T("Error: the memroy address pointed target  %s is not a node(ePipeline)"),InstanceName.c_str());
				return false;
			}

			ePipeline* TargetPipe  = (ePipeline*)Target->Value();
			
			ePipeline DataList = ExePipe;
			int InsertIndex = TargetPipe->Size();
			ePipeline InsertAddress = Dialog->m_MemoryAddress;
			InsertAddress.PushInt(InsertIndex);

			*TargetPipe << ExePipe;

			CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
			nf.PushInt(INSTANCE_DATA);
			nf.PushInt(IMPORT_DATA);
			nf.PushString(InstanceName);
			nf.PushPipe(InsertAddress);
			nf.PushPipe(DataList);

			nf.Notify(Dialog);

			return true;
		}

		Target = ParentPipe->GetEnergy(Index);
		if(Target->EnergyType()==TYPE_PIPELINE){
			ParentPipe = (ePipeline*)Target->Value();
		}else{
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid"));
			return false;
		} 
	}

	ExePipe.GetLabel() = Format1024(_T("Error: memory address out of instance"));
	return false;

}
bool  CExportMemory::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
	if (MemoryInstance==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find the focus memory instance"));
		return false;
	}

	tstring& InstanceName = MemoryInstance->GetLabel();

	ePipeline* ParentPipe = MemoryInstance;
	Energy*    Target = NULL;
	int64 AddressLength = Dialog->m_MemoryAddress.Size();
	if (AddressLength==0)
	{
		ePipeline Temp = *ParentPipe;
		ExePipe << Temp;
		return true;
	}

	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Dialog->m_MemoryAddress.GetData(i);
		if(Index<0 || Index>=ParentPipe->Size()){
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid in memory instance %s"),InstanceName.c_str());
			return false;
		}

		if(i==AddressLength-1){ //最后一个地址
			Target = ParentPipe->GetEnergy(Index);
			if (Target->EnergyType() != TYPE_PIPELINE)
			{
				ExePipe.GetLabel() = Format1024(_T("Error: the target of exporting memory is not a node(ePipeline)"));
				return false;
			}

			ePipeline* TargetPipe  = (ePipeline*)Target->Value();

			ePipeline Temp = *TargetPipe;
			ExePipe << Temp;
			return true;
		}

		Target = ParentPipe->GetEnergy(Index);
		if(Target->EnergyType()==TYPE_PIPELINE){
			ParentPipe = (ePipeline*)Target->Value();
		}else{
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid"));
			return false;
		} 
	}

	ExePipe.GetLabel() = Format1024(_T("Error: memory address out of instance"));
	return false;

}


bool  CGetMemory::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
	if (MemoryInstance==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: no find the focus memory instance"));
		return false;
	}

	tstring& InstanceName = MemoryInstance->GetLabel();

	ePipeline* ParentPipe = MemoryInstance;
	Energy*    Target = NULL;
	int64 AddressLength = Dialog->m_MemoryAddress.Size();

	if (AddressLength==0)
	{
		ExePipe.Push_Directly(ParentPipe->Clone());
		return true;
	}


	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Dialog->m_MemoryAddress.GetData(i);
		if(Index<0 || Index>=ParentPipe->Size()){
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid in memory instance %s"),InstanceName.c_str());
			return false;
		}

		if(i==AddressLength-1){ //最后一个地址
			Target = ParentPipe->GetEnergy(Index);
			ExePipe.Push_Directly(Target->Clone());
			return true;
		}

		Target = ParentPipe->GetEnergy(Index);
		if(Target->EnergyType()==TYPE_PIPELINE){
			ParentPipe = (ePipeline*)Target->Value();
		}else{
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid"));
			return false;
		} 
	}

	ExePipe.GetLabel() = Format1024(_T("Error: memory address out of instance"));
	return false;
}


bool  CGetMemorySize::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
	if (MemoryInstance==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: no find the focus memory instance"));
		return false;
	}

	tstring& InstanceName = MemoryInstance->GetLabel();

	ePipeline* ParentPipe = MemoryInstance;
	Energy*    Target = NULL;
	int64 AddressLength = Dialog->m_MemoryAddress.Size();

	if (AddressLength==0)
	{
		ExePipe.PushInt(ParentPipe->Size());
		return true;;
	}

	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Dialog->m_MemoryAddress.GetData(i);
		if(Index<0 || Index>=ParentPipe->Size()){
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid in memory instance %s"),InstanceName.c_str());
			return false;
		}

		if(i==AddressLength-1){ //最后一个地址
			Target = ParentPipe->GetEnergy(Index);

			if (Target->EnergyType()==TYPE_PIPELINE)
			{
				ePipeline* Pipe = (ePipeline*)Target->Value();
				ExePipe.PushInt(Pipe->Size());
			}else{
				ExePipe.PushInt(-1);
			}
			return true;
		}

		Target = ParentPipe->GetEnergy(Index);
		if(Target->EnergyType()==TYPE_PIPELINE){
			ParentPipe = (ePipeline*)Target->Value();
		}else{
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid"));
			return false;
		} 
	}

	ExePipe.GetLabel() = Format1024(_T("Error: memory address out of instance"));
	return false;

}
bool  CInsertMemory::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
	if (MemoryInstance==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: no find the focus memory instance"));
		return false;
	}

	tstring& InstanceName = MemoryInstance->GetLabel();

    if(ExePipe.Size()==0){
		ExePipe.GetLabel() = Format1024(_T("Error: ExePipe no data to insert to the memory instance %"),InstanceName.c_str());
		return false;
	}

	ePipeline* ParentPipe = MemoryInstance;
	Energy*    Target = NULL;
	int64 AddressLength = Dialog->m_MemoryAddress.Size();

	if (AddressLength==0)
	{
		eElectron E;
		ExePipe.Pop(&E);

	    ePipeline Address;
		Address.PushInt(0);


		CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
		nf.PushInt(INSTANCE_DATA);
		nf.PushInt(INSERT_DATA);
		nf.PushString(InstanceName);
		nf.PushPipe(Address);
		nf.Push_Directly(E.Clone());


		ParentPipe->InsertEnergy(0,E.Release());
		nf.Notify(Dialog);

		return true;
	}
	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Dialog->m_MemoryAddress.GetData(i);
		if(Index<0 || Index>ParentPipe->Size()){
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid in memory instance %s"),InstanceName.c_str());
			return false;
		}

		if(i==AddressLength-1){ //最后一个地址
			eElectron E;
			ExePipe.Pop(&E);

			Energy* Temp = E.Clone();

			ePipeline Address = Dialog->m_MemoryAddress;
			ParentPipe->InsertEnergy(Index,E.Release());

			CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
			nf.PushInt(INSTANCE_DATA);
			nf.PushInt(INSERT_DATA);
			nf.PushString(InstanceName);
			nf.PushPipe(Address);
			nf.Push_Directly(Temp);
			nf.Notify(Dialog);
			return true;
		}

		Target = ParentPipe->GetEnergy(Index);
		if(Target->EnergyType()==TYPE_PIPELINE){
			ParentPipe = (ePipeline*)Target->Value();
		}else{
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid"));
			return false;
		} 
	}

	ExePipe.GetLabel() = Format1024(_T("Error: memory address out of instance"));
	return false;

}

bool CModifyMemory::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
	if (MemoryInstance==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: no find the focus memory instance"));
		return false;
	}
	tstring& InstanceName = MemoryInstance->GetLabel();

	if(ExePipe.Size()==0){
		ExePipe.GetLabel() = Format1024(_T("Error: ExePipe no data to insert to the memory instance %"),InstanceName.c_str());
		return false;
	}


	ePipeline* ParentPipe = MemoryInstance;
	Energy*    Target = NULL;
	int64 AddressLength = Dialog->m_MemoryAddress.Size();

	if (AddressLength==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid in memory instance %s"),InstanceName.c_str());
		return false;
	}

	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Dialog->m_MemoryAddress.GetData(i);
		if(Index<0 || Index>=ParentPipe->Size()){
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid in memory instance %s"),InstanceName.c_str());
			return false;
		}

		if(i==AddressLength-1){ //最后一个地址
			eElectron E;
			ExePipe.Pop(&E);

			ePipeline Address = Dialog->m_MemoryAddress;

			CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
			nf.PushInt(INSTANCE_DATA);
			nf.PushInt(MODIFY_DATA);
			nf.PushString(InstanceName);
			nf.PushPipe(Address);
			nf.Push_Directly(E.Clone());

			Energy* Old = ParentPipe->ReplaceEnergy(Index,E.Release());
			delete Old;

			nf.Notify(Dialog);
			return true;
		}

		Target = ParentPipe->GetEnergy(Index);
		if(Target->EnergyType()==TYPE_PIPELINE){
			ParentPipe = (ePipeline*)Target->Value();
		}else{
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid"));
			return false;
		} 
	}

	ExePipe.GetLabel() = Format1024(_T("Error: memory address out of instance"));
	return false;

}

bool  CRemoveMemory::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 
	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
	if (MemoryInstance==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: no find the focus memory instance"));
		return false;
	}
	tstring& InstanceName = MemoryInstance->GetLabel();

	ePipeline* ParentPipe = MemoryInstance;
	Energy*    Target = NULL;
	int64 AddressLength = Dialog->m_MemoryAddress.Size();

	if (AddressLength==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid in memory instance %s"),InstanceName.c_str());
		return false;
	}

	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Dialog->m_MemoryAddress.GetData(i);
		if(Index<0 || Index>=ParentPipe->Size()){
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid in memory instance %s"),InstanceName.c_str());
			return false;
		}

		if(i==AddressLength-1){ //最后一个地址
			ParentPipe->EraseEnergy(Index,1);

			ePipeline Address = Dialog->m_MemoryAddress;
			CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
			nf.PushInt(INSTANCE_DATA);
			nf.PushInt(REMOVE_DATA);
			nf.PushString(InstanceName);
			nf.PushPipe(Address);
			nf.Notify(Dialog);
			return true;
		}

		Target = ParentPipe->GetEnergy(Index);
		if(Target->EnergyType()==TYPE_PIPELINE){
			ParentPipe = (ePipeline*)Target->Value();
		}else{
			ExePipe.GetLabel() = Format1024(_T("Error: memory address invalid"));
			return false;
		} 
	}

	ExePipe.GetLabel() = Format1024(_T("Error: memory address out of instance"));
	return false;

	
}

bool  CCloseMemory::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_MemoryFocus);
	if (MemoryInstance==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: no find the focus memory instance"));
		return true;
	}

	tstring InstanceName = MemoryInstance->GetLabel();

	Dialog->CloseMemoryInstance(Dialog->m_MemoryFocus);
	Dialog->m_NamedMemoryList.UnregisterNameByTask(InstanceName);
	Dialog->m_MemoryFocus = 0;

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_DATA);
	nf.PushInt(CLOSE_INSTANCE);
	nf.PushString(InstanceName);
	nf.Notify(Dialog);


	return true;
}