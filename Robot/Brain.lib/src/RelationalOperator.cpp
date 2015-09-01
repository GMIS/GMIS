// RelationalOperator.cpp: implementation of the RelationalOperator class.
//
//////////////////////////////////////////////////////////////////////

#include "RelationalOperator.h"

	

	/////////////////////////////////////////////////////////////
	
	bool CEquivalent::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		
		if(Pipe->Size()<2){
			return false;
		}

		eElectron     DataA;
		eElectron     DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		//NOTE: Comparing the two numbers, and as a result whose type will be changed  to INT
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();

		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a==b?1:0);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = (int64)f;

				Pipe->PushInt(a==b?1:0);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();
				
                float64 b = (float64)t;
				a = a-b;
				Pipe->PushInt((a >= - EPSINON) && (a <= EPSINON)?1:0);
				return true;

			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();

				a = a-b;
				Pipe->PushInt((a >= - EPSINON) && (a <= EPSINON)?1:0);
				return true;
			}
		}else if (TypeA == TYPE_STRING)
		{
			tstring& s1 = DataA.String();

			if (TypeB == TYPE_STRING)
			{
				tstring& s2 = DataB.String();

				Pipe->PushInt(s1==s2?1:0);
				return true;
			}	
		}
		
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make '==' operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
	
	bool CGreaterThan::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		
		if(Pipe->Size()<2){
			return false;
		}
		
		eElectron     DataA;
		eElectron     DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		//NOTE: Comparing the two numbers, and as a result whose type will be changed  to INT
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a>b?1:0);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a>b?1:0);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();
				
                float64 b = (float64)t;

				Pipe->PushInt(a>b?1:0);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushInt(a>b?1:0);
				return true;
			}
		}
		
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make '>' operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
	
	bool CGreaterThanOrEqual::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		
		if(Pipe->Size()<2){
			return false;
		}
		
		eElectron     DataA;
		eElectron     DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		//NOTE:Comparing the two numbers, and as a result whose type will be changed  to INT
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a>=b?1:0);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a>=b?1:0);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();
				
                float64 b = (float64)t;
				
				Pipe->PushInt(a>=b?1:0);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushInt(a>=b?1:0);
				return true;
			}
		}
		
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make '>=' operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
	
	bool CLessThan::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		
		if(Pipe->Size()<2){
			return false;
		}
		
		eElectron     DataA;
		eElectron     DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		//NOTE:Comparing the two numbers, and as a result whose type will be changed  to INT
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a<b?1:0);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a<b?1:0);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();
				
                float64 b = (float64)t;
				
				Pipe->PushInt(a<b?1:0);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushInt(a<b?1:0);
				return true;
			}
		}
		
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make '<' operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
	
	bool CLessThanOrEqual::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		
		if(Pipe->Size()<2){
			return false;
		}
		
		eElectron     DataA;
		eElectron     DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		//NOTE:Comparing the two numbers, and as a result whose type will be changed  to INT
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a<=b?1:0);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a<=b?1:0);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();
				
                float64 b = (float64)t;
				
				Pipe->PushInt(a<=b?1:0);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				Pipe->PushInt(a<=b?1:0);
				return true;
			}
		}
		
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make '<=' operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}
	
	bool CNotEquivalent::Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		
		if(Pipe->Size()<2){
			return false;
		}
		
		eElectron     DataA;
		eElectron     DataB;
		
		Pipe->Pop(&DataA);
		Pipe->Pop(&DataB);
		
		//NOTE:Comparing the two numbers, and as a result whose type will be changed  to INT
		uint32 TypeA = DataA.EnergyType();
		uint32 TypeB = DataB.EnergyType();
		
		if(TypeA == TYPE_INT)
		{
			int64 a = DataA.Int64();
			if (TypeB == TYPE_INT)
			{
				int64 b = DataB.Int64(); 
				
				Pipe->PushInt(a!=b?1:0);
				return true;
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 f = DataB.Float64();
                
				int64 b = (int64)f;
				
				Pipe->PushInt(a!=b?1:0);
				return true;
			}
		}else if (TypeA == TYPE_FLOAT)
		{
			float64 a = DataA.Float64();
			if (TypeB == TYPE_INT)
			{
				int64 t = DataB.Int64();
				
                float64 b = (float64)t;
				a = a-b;
				Pipe->PushInt((a >= - EPSINON) && (a <= EPSINON)?0:1);
				return true;
				
			}else if (TypeB == TYPE_FLOAT)
			{
				float64 b = DataB.Float64();
				
				a = a-b;
				Pipe->PushInt((a >= - EPSINON) && (a <= EPSINON)?0:1);
				return true;
			}
		}else if (TypeA == TYPE_STRING)
		{
			tstring& s1 = DataA.String();
			
			if (TypeB == TYPE_STRING)
			{
				tstring& s2 = DataB.String();
				
				Pipe->PushInt(s1==s2?0:1);
				return true;
			}	
		}
		
		Pipe->GetLabel()=Format1024(_T("%I64ld Error: %s and %s can't make '==' operator!"),m_ID,TYPENAME(TypeA),TYPENAME(TypeB));
		return false;
	}