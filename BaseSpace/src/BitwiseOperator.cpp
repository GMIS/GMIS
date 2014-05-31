// BitwiseOperator.cpp: implementation of the CBitwiseOperator class.
//
//////////////////////////////////////////////////////////////////////

#include "BitwiseOperator.h"

namespace BASESPACE{
	


    bool CBitwise_AND::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			return false;
		}
		eElectron      DataA;
		eElectron      DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
        
		//ֻ����32λ������޷��������������
		if(TypeA == TYPE_INT &&      
			TypeB == TYPE_INT){
			*(uint64*)(DataA.Value()) =(*(uint64*)(DataA.Value())) & (*(uint64*)(DataB.Value()));
			Pipe->Push_Directly(DataA.Release());
			return true;		
		}	
		
		Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s and %d can't make bitwise AND(|) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;	
	};

    bool CBitwise_NOT::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<1)
		{
			return false;
		}
		eElectron      DataA;
		
		Pipe->Pop(&DataA);
		
		uint32 TypeA = DataA.EnergyType();
		
		if( TypeA == TYPE_INT){
			*(uint64*)(DataA.Value()) = ~(*(uint64*)(DataA.Value()));
			Pipe->Push_Directly(DataA.Release());
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
			return false;
		}
		eElectron      DataA;
		eElectron      DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		//ֻ����32λ������޷��������������,�����ݵ����͵�ͬ���һ�����
		if(TypeA == TYPE_INT  &&      
			TypeB == TYPE_INT ){
			*(uint64*)(DataA.Value()) =(*(uint64*)(DataA.Value())) | (*(uint64*)(DataB.Value()));
			Pipe->Push_Directly(DataA.Release());
			return true;
		}	
		
		Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s and %s can't make bitwise OR(|) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
		
    bool CBitwise_XOR::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			return false;
		}
		eElectron      DataA;
		eElectron      DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		//ֻ����32λ������޷��������������
		if(TypeA==TYPE_INT  &&      
			TypeB == TYPE_INT){
			*(uint64*)(DataA.Value()) =(*(uint64*)(DataA.Value())) ^ (*(uint64*)(DataB.Value()));
			Pipe->Push_Directly(DataA.Release());
			return true;
			
		}	
		
		Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s and %s can't make bitwise XOR(^) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
	
	bool CLeft_Shift::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			return false;
		}
		eElectron     DataA;
		eElectron     DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		//ֻ����32λ������޷��������������
		if(TypeA==TYPE_INT  &&      
			TypeB == TYPE_INT){
			*(uint64*)(DataA.Value()) =(*(uint64*)(DataA.Value())) << (*(uint64*)(DataB.Value()));
			Pipe->Push_Directly(DataA.Release());
			return true;
		}	
		
		Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s and %s can't make left shift(<<) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
	
	bool CRight_Shift::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			return false;
		}
		eElectron      DataA;
		eElectron      DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		//ֻ����32λ������޷��������������
		if(TypeA==TYPE_INT  &&      
			TypeB == TYPE_INT){
			*(uint64*)(DataA.Value()) =(*(uint64*)(DataA.Value())) >> (*(uint64*)(DataB.Value()));
			Pipe->Push_Directly(DataA.Release());
			return true;
		}	
		Pipe->GetLabel() = Format1024(_T("%I64ld Error: %s and %s can't make right shift(>>) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
}
