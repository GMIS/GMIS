// LogicOperator.cpp: implementation of the CLogicOperator class.
//
//////////////////////////////////////////////////////////////////////

#include "LogicOperator.h"

	
	//////////////////////////////////////////////////////////////////////////
	bool Logical_Error(eElectron&   A){
		throw std::string("fatal error: use unkown data type making logical operator!");
		return false;
	};

	bool Logical_Int(eElectron&   A){
		if(*(int64*)(A.Value()) != 0 )return true;
		return false;
	};

	bool Logical_Float(eElectron&   A){
		if(*(float64*)(A.Value()) != 0.0f )return true;
		return false;
	};

	bool (*CLogical_AND::LOGICAL_OPERATOR[])(eElectron&  A)={
	    	Logical_Error,
			Logical_Int,
			Logical_Float
	};
	bool (*CLogical_NOT::LOGICAL_OPERATOR[])(eElectron&  A)={
	    	Logical_Error,
			Logical_Int,
			Logical_Float
	};	bool (*CLogical_OR::LOGICAL_OPERATOR[])(eElectron&  A)={
			Logical_Error,
			Logical_Int,
			Logical_Float
	};	

	bool CLogical_AND::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		eElectron     DataA;
		eElectron     DataB;
		if (Pipe->Size()<2)
		{
			return false;
		}
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		if(BASETYPE(TypeA) && BASETYPE(TypeB)){
			if( (*LOGICAL_OPERATOR[TypeA])(DataA) && (*LOGICAL_OPERATOR[TypeB])(DataB))
				Pipe->Push_Directly(new eINT(1));
			else Pipe->Push_Directly(new eINT(0));
			return true;
		}
		
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make '&&' operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
	
	bool CLogical_NOT::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<1)
		{
			return false;
		}
		eElectron     DataA;		
		Pipe->Pop(&DataA);

		uint32 TypeA = DataA.EnergyType();
		if(BASETYPE(TypeA)){
			if( !(*LOGICAL_OPERATOR[TypeA])(DataA))
				Pipe->Push_Directly(new eINT(1));
			else Pipe->Push_Directly(new eINT(0));
			return true;
		}

		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s can't make '!' operator!"),m_ID,TYPENAME(TypeA));
		return false;
	}
	bool CLogical_OR::Do(Energy* E)
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
		
		if(BASETYPE(TypeA) && BASETYPE(TypeB)){
			if( (*LOGICAL_OPERATOR[TypeA])(DataA) || (*LOGICAL_OPERATOR[TypeB])(DataB))
				Pipe->Push_Directly(new eINT(1));
			else Pipe->Push_Directly(new eINT(0));
			return true;
		}
		
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make '||' operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}

