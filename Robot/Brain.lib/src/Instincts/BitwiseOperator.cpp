// BitwiseOperator.cpp: implementation of the CBitwiseOperator class.
//
//////////////////////////////////////////////////////////////////////

#include "BitwiseOperator.h"
	

	/* 
			Because both of Int32 and UInt32 can do this operation, so it allows to accept any type of data,
			But will check whether the operand is a 32-bit integer, otherwise returns false
			Following similar
	*/
    bool CBitwise_AND::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make bitwise AND(&) operator!"),m_ID);
			return false;
		}
		eElectron      DataA;
		eElectron      DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
        
		int64 a, b;

		if(TypeA==TYPE_INT){
			a = *(int64*)(DataA.Value());
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataA.Value());
			a = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise AND(&) operator!"),m_ID,TYPENAME(TypeA));
			return false;	
		}

		if(TypeB==TYPE_INT){
			b = *(int64*)(DataB.Value());
		}else if (TypeB == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataB.Value());
			b = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise AND(&) operator!"),m_ID,TYPENAME(TypeB));
			return false;	
		}

		a =a & b;
		Pipe->PushInt(a);
		return true;	
	};

    bool CBitwise_NOT::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<1)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make bitwise NOT(~) operator!"),m_ID);
			return false;
		}

		eElectron      DataA;
		Pipe->Pop(&DataA);
		
		uint32 TypeA = DataA.EnergyType();
		
		if( TypeA == TYPE_INT){
			int64  a =  *(int64*)(DataA.Value());
			a = ~a;
			Pipe->PushInt(a);
			return true;
		}else if(TypeA == TYPE_FLOAT){
			float64  a =  *(float64*)(DataA.Value());
			int64 b = a;
			b = ~b;
			Pipe->PushInt(b);
			return true;
		}	
		
		Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise NOT(~) operator!"),m_ID,TYPENAME(TypeA));
		return false;
	}
	
    bool CBitwise_OR::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make bitwise OR(|) operator!"),m_ID);
			return false;
		}
		eElectron      DataA;
		eElectron      DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		int64 a, b;

		if(TypeA==TYPE_INT){
			a = *(int64*)(DataA.Value());
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataA.Value());
			a = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise OR(|) operator!"),m_ID,TYPENAME(TypeA));
			return false;	
		}

		if(TypeB==TYPE_INT){
			b = *(int64*)(DataB.Value());
		}else if (TypeB == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataB.Value());
			b = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise OR(|) operator!"),m_ID,TYPENAME(TypeB));
			return false;	
		}

		a =a | b;
		Pipe->PushInt(a);
		return true;
	}
		
    bool CBitwise_XOR::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make bitwise XOR(^) operator!"),m_ID);
			return false;
		}
		eElectron      DataA;
		eElectron      DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		int64 a, b;

		if(TypeA==TYPE_INT){
			a = *(int64*)(DataA.Value());
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataA.Value());
			a = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise XOR(^) operator!"),m_ID,TYPENAME(TypeA));
			return false;	
		}

		if(TypeB==TYPE_INT){
			b = *(int64*)(DataB.Value());
		}else if (TypeB == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataB.Value());
			b = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise XOR(^) operator!"),m_ID,TYPENAME(TypeB));
			return false;	
		}
		
		a = a^b;
		Pipe->PushInt(a);

		return true;
	}
	
	bool CLeft_Shift::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make left shift(<<) operator!"),m_ID);
			return false;
		}
		eElectron     DataA;
		eElectron     DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		int64 a, b;

		if(TypeA==TYPE_INT){
			a = *(int64*)(DataA.Value());
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataA.Value());
			a = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise left shift(<<) operator!"),m_ID,TYPENAME(TypeA));
			return false;	
		}

		if(TypeB==TYPE_INT){
			b = *(int64*)(DataB.Value());
		}else if (TypeB == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataB.Value());
			b = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise left shift(<<) operator!"),m_ID,TYPENAME(TypeB));
			return false;	
		}

		a = a<<b;
		Pipe->PushInt(a);
		return true;
	}
	
	bool CRight_Shift::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make right shift(>>) operator!"),m_ID);
			return false;
		}
		eElectron      DataA;
		eElectron      DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		int64 a, b;

		if(TypeA==TYPE_INT){
			a = *(int64*)(DataA.Value());
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataA.Value());
			a = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise right shift(>>) operator!"),m_ID,TYPENAME(TypeA));
			return false;	
		}

		if(TypeB==TYPE_INT){
			b = *(int64*)(DataB.Value());
		}else if (TypeB == TYPE_FLOAT)
		{
			float64 f = *(float64*)(DataB.Value());
			b = f;
		}else{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s can't make bitwise right shift(>>) operator!"),m_ID,TYPENAME(TypeB));
			return false;	
		}

		a = a>>b;
		Pipe->PushInt(a);
		return true;
	}

