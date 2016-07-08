// LogicOperator.cpp: implementation of the CLogicOperator class.
//
//////////////////////////////////////////////////////////////////////

#include "LogicOperator.h"

	
	//////////////////////////////////////////////////////////////////////////

	bool CLogical_AND::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		eElectron     DataA;
		eElectron     DataB;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make '&&' operator!"),m_ID);
			return false;
		}
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();

		int64 a, b;

		bool retA,retB;
		if(TypeA == TYPE_INT){
			a = *(int64*)(DataA.Value());
			retA = !!a;
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataA.Value());
			retA = !!f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make '&&' operator!"),m_ID,TYPENAME(TypeA));
			return false;	
		}

		if(TypeB==TYPE_INT){
			b = *(int64*)(DataB.Value());
			retB = !!b;
		}else if (TypeB == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataB.Value());
			retB = !!f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make '&&' operator!"),m_ID,TYPENAME(TypeB));
			return false;	
		}

		
		if( retA && retB){
			Pipe->PushInt(1);
		}else{
			Pipe->PushInt(0);
		}
		return true;
	}
	
	bool CLogical_NOT::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<1)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make '!' operator!"),m_ID);

			return false;
		}
		eElectron     DataA;		
		Pipe->Pop(&DataA);
		
		int64 a;
		bool retA;

		uint32 TypeA = DataA.EnergyType();
		
		if(TypeA == TYPE_INT){
			a = *(int64*)(DataA.Value());
			retA = !!a;
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataA.Value());
			retA = !!f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make '&&' operator!"),m_ID,TYPENAME(TypeA));
			return false;	
		}
		
		
		if( !retA){
			Pipe->PushInt(1);
		}
		else{
			Pipe->PushInt(0);
		}
		return true;	
	}
	bool CLogical_OR::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make '||' operator!"),m_ID);
			return false;
		}
		eElectron     DataA;
		eElectron     DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		int64 a, b;
		bool retA,retB;

		if(TypeA == TYPE_INT){
			a = *(int64*)(DataA.Value());
			retA = !!a;
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataA.Value());
			retA = !!f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make '||' operator!"),m_ID,TYPENAME(TypeA));
			return false;	
		}

		if(TypeB==TYPE_INT){
			b = *(int64*)(DataB.Value());
			retB = !!b;
		}else if (TypeB == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataB.Value());
			retB = !!f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make '||' operator!"),m_ID,TYPENAME(TypeB));
			return false;	
		}


		if( retA || retB){
			Pipe->PushInt(1);
		}
		else {
			Pipe->PushInt(0);
		}
		return true;
		
	}

