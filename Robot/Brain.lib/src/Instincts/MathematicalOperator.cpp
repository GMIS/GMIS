// MathematicalOperator.cpp
//
//////////////////////////////////////////////////////////////////////

#include "MathematicalOperator.h"


	bool CAddition::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make addition(+) operator!"),m_ID);
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
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a+b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();		
                float64 b = (float64)t;

				Pipe->PushFloat(a+b);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushFloat(a+b);
				return true;
			}
		}

		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make addition(+) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
    
	bool CSubtraction::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make subtraction(-) operator!"),m_ID);
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
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a-b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();		
                float64 b = (float64)t;
				
				Pipe->PushFloat(a-b);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushFloat(a-b);
				return true;
			}
		}

		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make subtraction(-) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		
		return false;
	}
	
    bool CMultiplication::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make Multiplication(*) operator!"),m_ID);
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
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a*b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();		
                float64 b = (float64)t;
				
				Pipe->PushFloat(a*b);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushFloat(a*b);
				return true;
			}
		}
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make Multiplication(*) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		
		return false;
	}
	
    bool CDivision::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make division(/) operator!"),m_ID);
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
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a/b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();		
                float64 b = (float64)t;
				
				Pipe->PushFloat(a/b);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushFloat(a/b);
				return true;
			}
		}
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make division(/) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
		
	}
	
	
    bool CModulus::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		if (Pipe->Size()<2)
		{
			Pipe->GetLabel() = Format1024(_T("%I64ld Error: no enough numbers to make modulus(%) operator!"),m_ID);
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
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a%b);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			int64 aa = (int64)a;
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64();		
				
				Pipe->PushFloat((float64)(aa%b));
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				int64 bb = (int64)b;
				Pipe->PushFloat((float64)(aa%bb));
				return true;
			}
		}
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make modulus(%) operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	};
	//////////////////////////////////////////////////////////////////////////
