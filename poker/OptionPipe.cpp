// OptionPipe.cpp: implementation of the COptionPipe class.
//
//////////////////////////////////////////////////////////////////////

#include "OptionPipe.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionPipe::COptionPipe(CABMutex* m)
:m_Mutex(m)
{

}

COptionPipe::~COptionPipe()
{

}

void COptionPipe::ToPipe(ePipeline& Pipe){
	CLock lk(m_Mutex);
	Pipe.Clone(*this);
}
void COptionPipe::FromPipe(ePipeline& Pipe){
	CLock lk(m_Mutex);
	Clone(Pipe);
};


BOOL  COptionPipe::SetValue(int64 ID,int32 Value){
	CLock lk(m_Mutex);
	for(int i=0; i<Size(); i++){
		Energy* e = GetEnergy(i);
		if (e->EnergyType() != TYPE_PIPELINE)
		{
			return FALSE;
		}
		ePipeline* Pipe = (ePipeline*)e;
		if (Pipe->GetID() == ID)
		{
			Pipe->Clear();
			Pipe->PushInt(Value);
			return TRUE;
		}
	}
	return FALSE;
};

BOOL  COptionPipe::SetValue(int64 ID,tstring Value){
	CLock lk(m_Mutex);

	for(int i=0; i<Size(); i++){
		Energy* e = GetEnergy(i);
		if (e->EnergyType() != TYPE_PIPELINE)
		{
			return FALSE;
		}
		ePipeline* Pipe = (ePipeline*)e;
		if (Pipe->GetID() == ID)
		{
			Pipe->Clear();
			Pipe->PushString(Value);
			return TRUE;
		}
	}
	return FALSE;
};

BOOL  COptionPipe::GetValue(int64 ID, int32& Value){
	CLock lk(m_Mutex);

	for(int i=0; i<Size(); i++){
		Energy* e = GetEnergy(i);
		if (e->EnergyType() != TYPE_PIPELINE)
		{
			return FALSE;
		}
		ePipeline* Pipe = (ePipeline*)e;
		if (Pipe->GetID() == ID)
		{
            Energy* e1 = Pipe->GetEnergy(0);
			if(e1->EnergyType() != TYPE_INT){
				return FALSE;
			}
			Value = *(int32*)e1->Value();
			return TRUE;
		}
	}
	return FALSE;
};

BOOL  COptionPipe::GetValue(int64 ID, tstring& Value){
	CLock lk(m_Mutex);

	for(int i=0; i<Size(); i++){
		Energy* e = GetEnergy(i);
		if (e->EnergyType() != TYPE_PIPELINE)
		{
			return FALSE;
		}
		ePipeline* Pipe = (ePipeline*)e;
		if (Pipe->GetID() == ID)
		{
            Energy* e1 = Pipe->GetEnergy(0);
			if(e1->EnergyType() != TYPE_STRING){
				return FALSE;
			}
			Value = *(tstring*)e1->Value();
			return TRUE;
		}
	}
	return FALSE;
};
