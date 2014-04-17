// MathematicalOperator.cpp
//
//////////////////////////////////////////////////////////////////////

#include "MathematicalOperator.h"

namespace BASESPACE{

	bool CAddition::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		
		if (Pipe->Size()<2)
		{
			return false;
		}
		eElectron    DataA;
		eElectron    DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
	
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a+b);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = f;
				
				Pipe->PushInt(a+b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();		
                float64 b = t;

				Pipe->PushFloat(a+b);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushFloat(a+b);
				return true;
			}
		}

		Pipe->PrintLabel(_T("%I64ld Error: %s and %s can't make addition(+) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
    
	bool CSubtraction::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			return false;
		}
		eElectron    DataA;
		eElectron    DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a-b);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = f;
				
				Pipe->PushInt(a-b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();		
                float64 b = t;
				
				Pipe->PushFloat(a-b);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushFloat(a-b);
				return true;
			}
		}

		Pipe->PrintLabel(_T("%I64ld Error: %s and %s can't make subtraction(-) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		
		return false;
	}
	
    bool CMultiplication::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			return false;
		}
		eElectron    DataA;
		eElectron    DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a*b);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = f;
				
				Pipe->PushInt(a*b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();		
                float64 b = t;
				
				Pipe->PushFloat(a*b);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushFloat(a*b);
				return true;
			}
		}
		Pipe->PrintLabel(_T("%I64ld Error: %s and %s can't make Multiplication(*) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		
		return false;
	}
	
    bool CDivision::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			return false;
		}
		eElectron    DataA;
		eElectron    DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a/b);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = f;
				
				Pipe->PushInt(a/b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();		
                float64 b = t;
				
				Pipe->PushFloat(a/b);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushFloat(a/b);
				return true;
			}
		}
		Pipe->PrintLabel(_T("%I64ld Error: %s and %s can't make division(/) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
		
	}
	
	
    bool CModulus::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			return false;
		}
		eElectron    DataA;
		eElectron    DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a%b);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = f;
				
				Pipe->PushInt(a%b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			int64 aa = a;
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64();		
				
				Pipe->PushFloat(aa%b);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				int64 bb = b;
				Pipe->PushFloat(aa%bb);
				return true;
			}
		}
		Pipe->PrintLabel(_T("%I64ld Error: %s and %s can't make modulus(%) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	};
	//////////////////////////////////////////////////////////////////////////


}// namespace ZHB