#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"
#include "LogicThread.h"

void CLogicDialog::_FindTokenAnd(deque<int64>& DestMeaningList, map<int64,FDeque>& SrcMeaningList,map<int64,FDeque>& ResultMeaningList,bool first){
 
	if (!first && SrcMeaningList.size()==0)
    {
		return;
    }

	if(DestMeaningList.size() ==0){
		return;
	}

	deque<int64>::iterator DestIt = DestMeaningList.begin();
	while (DestIt != DestMeaningList.end())
	{
		int64  DestMeaning = *DestIt;

		//取得每一个目标意义的所有结尾空间的ID
		FDeque EndSpaceIDList;
		char buf[30];
		int64toa(DestMeaning,buf);

		
		CppSQLite3Buffer SQL;
		SQL.format("select %s from  \"%s\"  ;",RB_SPACE_ID,buf);
		CppSQLite3Table t0 = CBrainMemory::BrainDB.getTable(SQL);
		for (int row = 0; row < t0.numRows(); row++)
		{	
			t0.setRow(row);
			int64 SpaceID = t0.getInt64Field(0);
			EndSpaceIDList.push_back(SpaceID);
		}	
		
		//如果缺少参与运算的意义,（第一次）则直接转入到结果列表中
		if (SrcMeaningList.size() == 0) 
		{
			assert(first);
			ResultMeaningList[DestMeaning] = EndSpaceIDList;
			DestIt++;
			continue;
		}
		
		map<int64,FDeque>::iterator SrcIt = SrcMeaningList.begin();
		while(SrcIt != SrcMeaningList.end()){
			
			//找出两个不同意义之间存储时间接近者
			FDeque& SrcEndSpaceIDList = SrcIt->second;
		
			FDeque::iterator DequeIt1 = SrcEndSpaceIDList.begin();
			while (DequeIt1 != SrcEndSpaceIDList.end())
			{
				int64 Time1 = *DequeIt1;
				FDeque::iterator DequeIt2 = EndSpaceIDList.begin();
                
				bool Find = false;
				int64 Interval=0; 
				while (DequeIt2 != EndSpaceIDList.end())
				{
					int64 Time2 = *DequeIt2;
					Interval = Time1-Time2;
					
					if ((Interval>=0&&Interval<m_FindInterval) || (Interval<0&&-Interval<m_FindInterval))
					{  					
						//保留后记忆token的意义ID及时间戳作为结果
						if(Time1>Time2){								
							FDeque& ResultEndSpaceIDList = ResultMeaningList[SrcIt->first];
							ResultEndSpaceIDList.push_back(Time1);
						}else{
							FDeque& ResultEndSpaceIDList = ResultMeaningList[DestMeaning];
							ResultEndSpaceIDList.push_back(Time2);
						}
						Find = true;
					}else{
						Find = false;
					} 

					if (Interval<0 && !Find) //由于Timer2是递增的，所以后面的肯定不符合。
					{
						break;
					}
                     
					DequeIt2++;
				}
									
				if (Interval>0 && !Find) //由于Timer1是递增的，如果最后一个差值>0且不符合则后面的肯定不符合，
				{
					break;
				}
				DequeIt1++;
			};

			SrcIt++;
		}
		DestIt ++;
	}

};

void CLogicDialog::_FindTokenOr(deque<int64>& DestMeaningList, map<int64,FDeque>& SrcMeaningList,map<int64,FDeque>& ResultMeaningList){
 
	deque<int64>::iterator DestIt = DestMeaningList.begin();

	while (DestIt != DestMeaningList.end())
	{
		int64  DestMeaning = *DestIt;

		//取得每一个目标意义的所有结尾空间的ID
		FDeque EndSpaceIDList;
		char buf[30];
		int64toa(DestMeaning,buf);
		
		CppSQLite3Buffer SQL;
		SQL.format("select %s from  \"%s\"  ;",RB_SPACE_ID,buf);
		CppSQLite3Table t0 = CBrainMemory::BrainDB.getTable(SQL);
		for (int row = 0; row < t0.numRows(); row++)
		{	
			t0.setRow(row);
			int64 SpaceID = t0.getInt64Field(0);
			EndSpaceIDList.push_back(SpaceID);
		}	
		
		//如果缺少参与运算的意义,则直接转入到结果列表中
		if (SrcMeaningList.size() == 0) 
		{
			ResultMeaningList[DestMeaning] = EndSpaceIDList;
			DestIt++;
			continue;
		}
		
		map<int64,FDeque>::iterator SrcIt = SrcMeaningList.begin();
		while(SrcIt != SrcMeaningList.end()){
			
			if(SrcIt->first == DestMeaning){ //只保留DestMeaning	
#ifdef _STLP_MAP
				SrcMeaningList.erase(SrcIt);
				SrcIt++;
#else
				SrcIt = SrcMeaningList.erase(SrcIt);
#endif				
				continue;
			}
			
			//找出两个不同意义之间存储时间接近者
			FDeque& SrcEndSpaceIDList = SrcIt->second;
		    
			FDeque::iterator DequeIt1 = SrcEndSpaceIDList.begin();
			while (DequeIt1 != SrcEndSpaceIDList.end())
			{
				int64 Time1 = *DequeIt1;
				FDeque::iterator DequeIt2 = EndSpaceIDList.begin();
                
				bool Find = false;
				int64 Interval=0; 
				while (DequeIt2 != EndSpaceIDList.end())
				{
					int64 Time2 = *DequeIt2;
					Interval = Time1-Time2;
					
					if ((Interval>=0&&Interval<m_FindInterval) || (Interval<0&&-Interval<m_FindInterval))
					{  					
						//如果重复，去掉先记忆token的时间戳
						if(Time1>Time2){								
							DequeIt2 = EndSpaceIDList.erase(DequeIt2);
							DequeIt2--;
						}else{
							DequeIt1 = SrcEndSpaceIDList.erase(DequeIt1);
							DequeIt1--;
						}
						Find = true;
					}else{
						Find = false;
					} 

					if (Interval<0 && !Find) //由于Timer2是递增的，所以后面的肯定不符合。
					{
						break;
					}
                     
					DequeIt2++;
				}
									
				if (Interval>0 && !Find) //由于Timer1是递增的，如果最后一个差值>0且不符合则后面的肯定不符合，
				{
					break;
				}
				DequeIt1++;
			};

			SrcIt++;
		}
		ResultMeaningList[DestMeaning] = EndSpaceIDList;
		DestIt ++;
	}

	//最后合并SrcMeaningList
	//	ResultMeaningList.insert(SrcMeaningList.begin(),SrcMeaningList.end());
  	map<int64,FDeque>::iterator SrcIt = SrcMeaningList.begin();
	while(SrcIt != SrcMeaningList.end()){
	     FDeque& EndSpaceList = SrcIt->second;
		 ResultMeaningList[SrcIt->first] = EndSpaceList;
		 SrcIt++;
	}
};


void CLogicDialog::_FindTokenNot(deque<int64>& DestMeaningList, map<int64,FDeque>& SrcMeaningList,map<int64,FDeque>& ResultMeaningList){
 
	if(SrcMeaningList.size()==0 || DestMeaningList.size()==0)return;

	deque<int64>::iterator DestIt = DestMeaningList.begin();

	while (DestIt != DestMeaningList.end())
	{
		int64  DestMeaning = *DestIt;

		//取得每一个目标意义的所有结尾空间的ID
		FDeque EndSpaceIDList;
		char buf[30];
		int64toa(DestMeaning,buf);
		
		CppSQLite3Buffer SQL;
		SQL.format("select %s from  \"%s\"  ;",RB_SPACE_ID,buf);
		CppSQLite3Table t0 = CBrainMemory::BrainDB.getTable(SQL);
		for (int row = 0; row < t0.numRows(); row++)
		{	
			t0.setRow(row);
			int64 SpaceID = t0.getInt64Field(0);
			EndSpaceIDList.push_back(SpaceID);
		}	
		
		map<int64,FDeque>::iterator SrcIt = SrcMeaningList.begin();
		while(SrcIt != SrcMeaningList.end()){
			
			if(SrcIt->first == DestMeaning){ //只保留DestMeaning	
#ifdef _STLP_MAP
				SrcMeaningList.erase(SrcIt);
				SrcIt++;
#else
				SrcIt = SrcMeaningList.erase(SrcIt);
#endif				
				continue;
			}			
			//找出两个不同意义之间存储时间接近者
			FDeque& SrcEndSpaceIDList = SrcIt->second;
		    
			FDeque::iterator DequeIt1 = SrcEndSpaceIDList.begin();
			while (DequeIt1 != SrcEndSpaceIDList.end())
			{
				int64 Time1 = *DequeIt1;
				FDeque::iterator DequeIt2 = EndSpaceIDList.begin();
                
				bool Find = false;
				int64 Interval=0; 
				while (DequeIt2 != EndSpaceIDList.end())
				{
					int64 Time2 = *DequeIt2;
					Interval = Time1-Time2;
					
					if ((Interval>=0&&Interval<m_FindInterval) || (Interval<0&&-Interval<m_FindInterval))
					{  	
						//只要与目标意义的时间戳相近则删除
						DequeIt1 = SrcEndSpaceIDList.erase(DequeIt1);
						DequeIt1--;						
						Find = true;
					}else{
						Find = false;
					} 

					if (Interval<0 && !Find) //由于Timer2是递增的，所以后面的肯定不符合。
					{
						break;
					}
                     
					DequeIt2++;
				}
									
				if (Interval>0 && !Find) //由于Timer1是递增的，如果最后一个差值>0且不符合则后面的肯定不符合，
				{
					break;
				}
				DequeIt1++;
			};

			SrcIt++;
		}
		DestIt ++;
	}

	// 重新得到经过修剪的SrcMeaningList
	//	ResultMeaningList.insert(SrcMeaningList.begin(),SrcMeaningList.end());
  	map<int64,FDeque>::iterator SrcIt = SrcMeaningList.begin();
	while(SrcIt != SrcMeaningList.end()){
	     FDeque& EndSpaceList = SrcIt->second;
		 ResultMeaningList[SrcIt->first] = EndSpaceList;
		 SrcIt++;
	}
		
};


void CLogicDialog::_FindMemorySpace(CLogicThread* Think,map<int64,FDeque>* DestTokenList, deque<int64>* ResultSpaceList){
	
	deque<int64> SpaceSet;
	char buf[30];
	CppSQLite3Buffer SQL;	
	
	map<int64,FDeque>::iterator TokenIt = DestTokenList->begin();
	while (TokenIt != DestTokenList->end())
	{
		int64 TokenMeaingID = TokenIt->first;
		FDeque& TokenEndSpaceList = TokenIt->second;
		
		//取得使用这个token意义的所有索引信息
	    ToLBrain(TokenMeaingID);
		int64toa(TokenMeaingID,buf);

		SQL.format("select * from  \"%s\"  ;",buf);
		CppSQLite3Table t0 = CBrainMemory::BrainDB.getTable(SQL);
		for (int row = 0; row < t0.numRows(); row++)
		{	
			t0.setRow(row);
			int64 FatherSpaceID = t0.getInt64Field(0);
			int64 FatherMeaningID = t0.getInt64Field(1);
            int64 ChildID   = t0.getInt64Field(2);
           
			if (ChildID == -TokenMeaingID) //前面使用了ToLBrain(TokenMeaingID);
			{
				continue;   //这是对意义空间值为0时的特殊处理造成的结果，忽略
			}
			
			if (FatherSpaceID != ROOT_SPACE)
			{
				//依次取出父空间意义的所有结尾空间
		
				FDeque EndSpaceList;			
				int64toa(FatherMeaningID,buf);
				SQL.format("select %s from  \"%s\"  ;",RB_SPACE_ID,buf);
				CppSQLite3Table t1 = CBrainMemory::BrainDB.getTable(SQL);
				for (int row1 = 0; row1 < t1.numRows(); row1++)
				{	
					t1.setRow(row1);
					int64 SpaceID = t1.getInt64Field(0);
					EndSpaceList.push_back(SpaceID);
				}	
				
				bool Find ;
				
				//比较两个结尾空间列表，如果有对应的时间关联则前面所的
				//的ChildID为最终的记忆路径
				FDeque::iterator DequeIt1 = TokenEndSpaceList.begin();
				while (DequeIt1 != TokenEndSpaceList.end())
				{
					int64 Time1 = *DequeIt1;
					
					Find = false;
					int64 Interval=0; 
					FDeque::iterator DequeIt2 = EndSpaceList.begin();
					while (DequeIt2 != EndSpaceList.end())
					{
						int64 Time2 = *DequeIt2;
						Interval = Time1-Time2;
						
						if ((Interval>=0&&Interval<m_FindInterval) || (Interval<0&&-Interval<m_FindInterval))
						{  					
							//保留后记忆token的意义ID及时间戳作为结果
							//SpaceSet.insert(ChildID);
							SpaceSet.push_back(ChildID);
							Find = true;
							break;
						}else{
							Find = false;
						} 
						
						if (Interval<0) //由于Timer2是递增的，所以后面的肯定不符合。
						{
							break;
						}
						
						DequeIt2++;
					}
					
					if (Find){
						break;
					}else if (Interval>0 ) //由于Timer1是递增的，如果最后一个差值>0且不符合则后面的肯定不符合，
					{
						break;
					}
					DequeIt1++;
				};
				
			}else{ //FatherID == ROOM_ROOT
					
				int64toa(ChildID,buf);
				SQL.format("select * from  \"%s\"  ;",buf);
				CppSQLite3Table t1 = CBrainMemory::BrainDB.getTable(SQL);
				//取得ChildID的所有子空间
				for (int row1 = 0; row1 < t1.numRows(); row1++)
				{	
					t1.setRow(row1);
					int64 SpaceID = t1.getInt64Field(0);
					int64 MeaningID = t1.getInt64Field(1);
					int64 SpaceType = t1.getInt64Field(2);

					if(SpaceType == MEMORY_BODY){
						//得到MeaningID下的所有结尾空间
						int64toa(MeaningID,buf);
					}else if (IsMeaningSpace(SpaceType))
					{   //直接得到所有结尾空间
						int64toa(SpaceID,buf);					
					}
					assert(SpaceType != MEMORY_LOGIC_END || SpaceType != MEMORY_NULL_END);

					FDeque EndSpaceList;		
					SQL.format("select %s from  \"%s\"  ;",RB_SPACE_ID,buf);
					CppSQLite3Table t2 = CBrainMemory::BrainDB.getTable(SQL);
					for (int row2 = 0; row2 < t2.numRows(); row2++)
					{	
						t2.setRow(row2);
						int64 EndID = t2.getInt64Field(0);
						EndSpaceList.push_back(EndID);
					}	
					
					if(EndSpaceList.size()==0)continue;
					bool Find ;
					
					//比较两个结尾空间列表，如果有对应的时间关联则前面所的
					//的ChildID为最终的记忆路径
					FDeque::iterator DequeIt1 = TokenEndSpaceList.begin();
					while (DequeIt1 != TokenEndSpaceList.end())
					{
						int64 Time1 = *DequeIt1;
						
						Find = false;
						int64 Interval=0; 
						FDeque::iterator DequeIt2 = EndSpaceList.begin();
						while (DequeIt2 != EndSpaceList.end())
						{
							int64 Time2 = *DequeIt2;
							Interval = Time1-Time2;
							
							if ((Interval>=0&&Interval<m_FindInterval) || (Interval<0&&-Interval<m_FindInterval))
							{  					
								if(SpaceType == MEMORY_BODY){
									SpaceSet.push_back(SpaceID);
								}else //if (SpaceType == MEMORY_TYPE_MEANING)
								{   //直接得到搜索结果
									ProcessMeaning(Think,SpaceID,MeaningID,SpaceType,m_FindType);
								}
								Find = true;
								break;
							}else{
								Find = false;
							} 
							
							if (Interval<0) //由于Timer2是递增的，所以后面的肯定不符合。
							{
								break;
							}
							
							DequeIt2++;
						}
						
						if (Find){
							break;
						}else if (Interval>0 ) //由于Timer1是递增的，如果最后一个差值>0且不符合则后面的肯定不符合，
						{
							break;
						}
						DequeIt1++;
					};
				}					

			}

		     
		}	
		
		TokenIt++;        
	}
	
    
	//进一步剔除重复的
	/*真的需要吗？如果剔除，集中学习本能都将被排除在结果之外
	  目前干脆不管。
	*/

	ResultSpaceList->swap(SpaceSet);
    return;

	if(SpaceSet.size()==0)return;

	deque<int64>::iterator SetIt1 ,SetIt2 ;
	SetIt1 = SpaceSet.begin();
	SetIt2 = SetIt1;
	SetIt2++;
	
	if(SpaceSet.size()==1){
		ResultSpaceList->push_back(*SetIt1);
		return ;
	}
	
	do{
		int64 Time1 = *SetIt1;
		int64 Time2 = *SetIt2;

		int64 Interval = (Time2-Time1)/10000000L;
		assert(Interval>=0);	
		if (Interval<m_FindInterval)
		{  										
			SetIt2 = SpaceSet.erase(SetIt1);
		}else{
			ResultSpaceList->push_back(Time1);
		}
	    SetIt1 = SetIt2;
		SetIt2++;
	}while(SetIt2 != SpaceSet.end());

	//补上最后一个
	ResultSpaceList->push_back(*SetIt1);
}

void CLogicDialog::FindFirst(tstring& text,FindTypeExpected FindType/*= FIND_ALL*/){
	m_FindType = FindType;
	m_FindSeedList.clear();
	m_FindResultList.clear();
	
	CLogicThread  TempThink; //避免对现有对话产生影响

	for(uint32 j=0; j<text.size(); j++){
		TempThink.m_Text.Analyse(j,text[j]);
		TempThink.ThinkAllAffectedToken(this);
	};
	


	if(TempThink.m_Text.m_TokenList.size()==0)return;
  
    
    map<int64,FDeque> MeaningList1;
  	map<int64,FDeque> MeaningList2;
    map<int64,FDeque>* CurTokenList = &MeaningList1; 
    map<int64,FDeque>* ResultTokenList = &MeaningList2;
	map<int64,FDeque>* temp=NULL; 
    
	
/*	CClause* Clause = JustOneClause();
	if(Clause == NULL){
       RuntimeOutput("Error: Find cmd incorrect, ignore");
	   return;
	};
*/

	tstring flag;
	bool FirstToken = true;
	deque<CToken*>::iterator TokenIt = TempThink.m_Text.m_TokenList.begin();
	//TokenIt += Clause->m_BeginPos;	
	for (uint32 i=0; i<TempThink.m_Text.m_TokenList.size(); i++)
	{
		CToken *Token = *TokenIt;
		TokenIt++;
		
		if (Token->m_Str == _T("or")  || Token->m_Str == _T("not") || Token->m_Str== _T("and"))
		{
			flag = Token->m_Str;
			continue;
		}
		
		deque<int64> MeaningSpaceIDList;
		if (Token->isPunct())
		{
			continue;
		}
		else if(!Token->IsOK()){
//			RuntimeOutput(srcMsg,_T("Warning: token \"%s\" incognizance !"),Token->m_Str.c_str());
//			continue;
		}else{
			TempThink.GetAllMeaningSpaceID(Token->m_MemoryID, MeaningSpaceIDList);
		}

		if(flag == _T("not")){
			_FindTokenNot(MeaningSpaceIDList,*CurTokenList,*ResultTokenList);
			flag = _T("");
		}else if(flag == _T("or")){
			_FindTokenOr(MeaningSpaceIDList,*CurTokenList,*ResultTokenList);
			flag = _T("");
		}else{
			_FindTokenAnd(MeaningSpaceIDList,*CurTokenList,*ResultTokenList,FirstToken);
		}
		//交换地址，把得到的结果继续参与下一个token运算，避免交换值
		CurTokenList->clear();
		temp = CurTokenList;
		CurTokenList = ResultTokenList;
		ResultTokenList = temp;
		FirstToken = false;
	}
	
	//得到最终的搜索路径
	//现在得到的是一个或多个token某个指定意义空间ID及使用这个意义的结尾空间列表
	//需要把它转换成最终搜索路径，并剔除重复路径
	_FindMemorySpace(&TempThink,CurTokenList,&m_FindSeedList);

}
	
void CLogicDialog::FindContinue(CLogicThread* Think,uint32 Index, ePipeline& SearchResult){	
	
	if(Index >= m_FindResultList.size()){  //指定要求的结果超过当前结果的范围，则继续搜索新结果

		int64 SpaceID, SpaceValue,SpaceType;
		char buf[30];
		CppSQLite3Buffer SQL;

		uint32 End = m_FindResultList.size() + m_ItemNumPerPage;
		while(m_FindResultList.size()< End && m_FindSeedList.size())
		{
			
			SpaceID = m_FindSeedList.front();
			m_FindSeedList.pop_front();
			
			ToRBrain(SpaceID);
			int64toa(SpaceID,buf);
			SQL.format("select *  from  \"%s\" ;",buf);
			CppSQLite3Table t = CBrainMemory::BrainDB.getTable(SQL);
			
			for (int row = 0; row < t.numRows(); row++)
			{
				t.setRow(row);
				SpaceID = t.getInt64Field(0);
				SpaceType = t.getInt64Field(2);
				
				if (IsMeaningSpace(SpaceType))
				{
					SpaceValue = t.getInt64Field(1);
					ProcessMeaning(Think,SpaceID,SpaceValue,SpaceType,m_FindType);
				}else{
					m_FindSeedList.push_back(SpaceID);
				}
			}	
		}
	
	}

	Index = max(0,Index);	
	
	uint32 End = min(Index+m_ItemNumPerPage,m_FindResultList.size());
	for (uint32 i = Index; i<End; i++)
	{
		_FindResult& Result = m_FindResultList[i];
		if (Result.m_Type == TEXT_RESULT)
		{
			PrintText(Think,SearchResult,i+1,Result.m_ID);
		}else if (Result.m_Type == LOGIC_RESULT)
		{
			PrintLogic(Think,SearchResult,i+1,Result.m_ID,Result.m_Value);
		}else{
			PrintObject(Think,SearchResult,i+1,Result.m_ID,Result.m_Value);
		}
	}		
}

void CLogicDialog::ProcessMeaning(CLogicThread* Think,int64 SpaceID,int64 SpaceValue,int64 SpaceType,FindTypeExpected FindType /*= FIND_ALL*/){

	char buf[30];
	CppSQLite3Buffer SQL;
	int64 FatherID;
	
	if (SpaceType == MEMORY_PEOPLE ){  //独立表达,到此结束
		if(FindType==FIND_ALL || FindType==FIND_PEOPLE)
			OutputFindResult(OBJECT_RESULT,SpaceID,SpaceValue);
		//m_ObjectList[SpaceID] = SpaceValue;
	}else if (SpaceType == MEMORY_OBJECT)
	{
		if(FindType==FIND_ALL || FindType==FIND_OBJECT)
			OutputFindResult(OBJECT_RESULT,SpaceID,SpaceValue);
	}
	else{ //既可能独立表达也可能参与表达
		if (SpaceType == MEMORY_SERIES || SpaceType == MEMORY_SHUNT){
			if(FindType==FIND_ALL || FindType==FIND_LOGIC)
				OutputFindResult(LOGIC_RESULT,SpaceID,SpaceValue);
		}else if (SpaceType == MEMORY_INSTINCT)
		{
			if(FindType==FIND_ALL || FindType==FIND_COMMAND)
				//m_LogicList[SpaceID] = SpaceValue;
				OutputFindResult(LOGIC_RESULT,SpaceID,SpaceValue);
		}

		int32 OldCount =m_FindSeedList.size();
		if(Think->LBrainHasTable(SpaceID)){
			//索引空间是否有表，有则表示其为其它空间的空间值
			//根据索引表得到其作为空间值时的空间存储ID，存入种子表
			ToLBrain(SpaceID);
			int64toa(SpaceID,buf);
			SQL.format("select %s,%s from  \"%s\"  ;",LB_FATHER_ID,LB_CHILD_ID,buf);

			int64 OldFather = 0;
			CppSQLite3Table t0 = CBrainMemory::BrainDB.getTable(SQL);
			for (int row = 0; row < t0.numRows(); row++)
			{
				t0.setRow(row);
				FatherID = t0.getInt64Field(0);
				int64 ChildID = t0.getInt64Field(1);
				if(ChildID == -SpaceID)continue; //特殊设计，忽略

				int64 ChildType = Think->GetChildType(FatherID,ChildID);
				if(ChildType == MEMORY_BODY){ //参与其它意义
					//如何避免一个token在同一个记忆的不同地方引起的重复呢？
					if((FatherID - OldFather)> m_FindInterval){
						m_FindSeedList.push_back(ChildID);
						OldFather = FatherID;
					}
				}
			}
		}
		//作为文本,忽略参与表达，只返回最终表达(参与表达引起m_FindSeedList增长)
		if( OldCount == m_FindSeedList.size()){
			ToRBrain(SpaceID);
			if(SpaceValue != SpaceID){ //根据规则，如果是NULL_MEANING则SpaceValue == SpaceID
				int64 SpaceID1 = SpaceValue;
				int64 SpaceValue1 =0;
				int64 SpaceType1 = 0;
				if(Think->GetSpaceInfo(SpaceID1,SpaceValue1,SpaceType1)){
					if (SpaceType1 == MEMORY_INSTINCT){
						if(FindType==FIND_ALL || FindType==FIND_COMMAND)
							OutputFindResult(LOGIC_RESULT,SpaceID1,SpaceValue1);
					}
					else if (SpaceType1 == MEMORY_PEOPLE ){  //独立表达,到此结束
						if(FindType==FIND_ALL || FindType==FIND_PEOPLE)
							OutputFindResult(OBJECT_RESULT,SpaceID1,SpaceValue1);
					}else if (SpaceType1 == MEMORY_OBJECT)
					{
						if(FindType==FIND_ALL || FindType==FIND_OBJECT)
							OutputFindResult(OBJECT_RESULT,SpaceID1,SpaceValue1);
					}
					else if (SpaceType1 == MEMORY_SERIES || SpaceType1 == MEMORY_SHUNT){
						if(FindType==FIND_ALL || FindType==FIND_LOGIC)
								OutputFindResult(LOGIC_RESULT,SpaceID1,SpaceValue1);	
					}
				}
			}else if(IsPartOfSpeech(SpaceType)){
				if(FindType==FIND_ALL || FindType==FIND_TEXT){
					//m_TextList[SpaceID] = SpaceValue;
					ToRBrain(SpaceID);
					OutputFindResult(TEXT_RESULT,SpaceID,SpaceValue);
				}	
			}
		}	
	}
	
}

void  CLogicDialog::PrintText(CLogicThread* Think,ePipeline& SearchResult,int32 n,int64 SpaceID){
	tstring Text;
	Think->RetrieveText(SpaceID,Text);
			
    ePipeline Item;
	Item.PushInt(TEXT_RESULT);
	Item.PushInt(n);
	Item.PushInt(SpaceID);
	Item.PushString(Text);
	
	SearchResult.PushPipe(Item);
}

void  CLogicDialog::PrintLogic(CLogicThread* Think,ePipeline& SearchResult,int32 n,int64 SpaceID,int64 SpaceValue){
	tstring Text ;
	deque<tstring> LogicList;
	if (BelongInstinct(SpaceValue))
	{
		Think->RetrieveText(SpaceID,Text);
		Text = _T("Command : ") + Text;
	}else{
		Think->RetrieveLogic(SpaceID,LogicList);		
		deque<tstring>::iterator It = LogicList.begin();
		while (It != LogicList.end())
		{
			Text += *It;
			Text += _T('\n');
			It++;
		}
		if(Text.size())*Text.rbegin() = _T('\0'); 
	}

	//得到逻辑描述
	LogicList.clear();
   
	tstring Memo;
	Think->GetMemo(SpaceID,LogicList);
	deque<tstring>::iterator It = LogicList.begin();
	while (It != LogicList.end())
	{
	    Memo += *It;
		Memo += _T('\n');
		It++;
	}

	if(Memo.size()==0)Memo = _T("(NULL)");
	else *Memo.rbegin() = _T('\0'); 

    ePipeline Item;
	Item.PushInt(LOGIC_RESULT);
	Item.PushInt(n);
	Item.PushInt(SpaceID);
	Item.PushString(Text);
	Item.PushString(Memo); 
	SearchResult.PushPipe(Item);
}

void  CLogicDialog::PrintObject(CLogicThread* Think,ePipeline& SearchResult,int32 n,int64 SpaceID,int64 SpaceValue){

	tstring Text;

	Think->RetrieveObject(SpaceID,Text);
	    
	//得到逻辑描述
	tstring Memo;
	deque<tstring> MemoList;
    Think->GetMemo(SpaceID,MemoList);
	deque<tstring>::iterator It = MemoList.begin();
	while (It != MemoList.end())
	{
		Memo += *It;
		Memo += _T('\n');
		It++;
	}
	if(Memo.size()==0)Memo = _T("(NULL)");

	
	ePipeline Item;
	Item.PushInt(OBJECT_RESULT);
	Item.PushInt(n);
	Item.PushInt(SpaceID);
	Item.PushString(Text);
	Item.PushString(Memo);
	SearchResult.PushPipe(Item);

};

void  CLogicDialog::OutputFindResult(int8 type, int64 SpaceID,int64 SpaceValue){
	//避免结果重复之苯方法
/*
	vector<_FindResult>::iterator It = m_FindResultList.begin();
	while(It < m_FindResultList.end()){
	    if(It->m_ID == SpaceID)return; 
		It++;
	}
*/   
	_FindResult Result(type, SpaceID,SpaceValue);
	m_FindResultList.push_back(Result);
	
	
	return;
/*
	if (Result.m_Type == TEXT_RESULT)
	{
		PrintText(srcMsg,m_FindResultList.size(),Result.m_ID);
	}else if (Result.m_Type == LOGIC_RESULT)
	{
		PrintLogic(srcMsg,m_FindResultList.size(),Result.m_ID,Result.m_Value);
	}else{
		PrintObject(srcMsg,m_FindResultList.size(),Result.m_ID,Result.m_Value);
	}
	
*/	
	return ;
}
