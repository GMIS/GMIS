#pragma warning (disable: 4786)

#include "BrainInitElt.h"
#include "..\InstinctDefine.h"
#include "..\LogicDialog.h"


CTestExpectation::CTestExpectation(int64 ID,bool bExpectation,tstring Name)
:CElement(ID,Name.c_str()),m_bExpectation(bExpectation)
{

}
CTestExpectation::~CTestExpectation(){

}

bool CTestExpectation::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	if(m_bExpectation==true){
		if(ExePipe.GetID()==RETURN_ERROR){
			tstring& error = ExePipe.GetLabel();
			Dialog->RuntimeOutput(m_ID--,_T("Got not expectation result--> %s"),error.c_str());
			error = _T("");
			ExePipe.SetID(RETURN_NORMAL);
			return true;
		}else{
			return true;
		}
	}else{
		if(ExePipe.GetID()==RETURN_ERROR){
			tstring& error = ExePipe.GetLabel();
			error = _T("");
			ExePipe.SetID(RETURN_NORMAL);
			return true;
		}else{
			Dialog->RuntimeOutput(m_ID--,_T("Got not expectation result-->it should be failed"));
			ExePipe.SetID(RETURN_NORMAL);
			return true;
		}
	}
}


CBrainTestElt::CBrainTestElt(int64 ID,tstring Name)
:CElement(ID,Name.c_str())
{
	        
};

CBrainTestElt::~CBrainTestElt(){
	
}
	
bool CBrainTestElt::Do(CLogicDialog* Dialog,ePipeline& ExePipe, ePipeline& LocalAddress,CMsg& Msg){

	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);
    
	int32 ChildIndex =0 ;

	ActomList::iterator It = m_ActomList.begin();

	if (!Msg.IsReaded()) //先处理信息
	{		
		ePipeline& ObjectAddress = Msg.GetReceiver();
		if (ObjectAddress.Size()==0) //该此Element处理信息,在处理自己的Child之前
		{
			ChildIndex = IT_SELF;  //表示自身,而不是某个child
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress); 
			if (ret==RETURN_DIRECTLY)
			{
				return true;
			}

			It = m_ActomList.begin();
		} 
		else //根据地址在自己的子物体里寻找
		{
			int64 ID = ObjectAddress.PopInt();			
			while (It != m_ActomList.end())
			{
				Mass* Child = *It;
				if (Child->m_ID == ID)
				{
					break;
				}
				It++;
			}
			
			if (It == m_ActomList.end())
			{
				//先交给系统缺省处理(有时需要关闭无效事件)
			    int64 MsgID = Msg.GetMsgID();
				tstring MsgStr = Dialog->m_Brain->MsgID2Str(MsgID);

				ChildIndex = IT_SELF;
				CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
				
				ExePipe.SetID(RETURN_ERROR);
				ExePipe.GetLabel() = Format1024(_T("Error: Msg(%s) Address(%I64ld) Invalid"),MsgStr.c_str(),ID);
				return false;
			}else{
				
				Mass* Child = *It;
				if(Child->MassType() != MASS_ELEMENT){  //对于非MASS_ELEMENT，由父Element代为处理信息
					if (ObjectAddress.Size()!=0) //对于非MASS_ELEMENT,则肯定是最后一个目标地址
					{
						int64 MsgID = Msg.GetMsgID();
						tstring MsgStr = Dialog->m_Brain->MsgID2Str(MsgID);

						ExePipe.SetID(RETURN_ERROR);
						ExePipe.GetLabel() = Format1024(_T("Error: Msg(%s) Address(%I64ld) Invalid"),MsgStr.c_str(),ID);
						return false;
					};
					
					ChildIndex = It-m_ActomList.begin();
					MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
					if (ret == RETURN_DIRECTLY)
					{
						return false;
					}
				}
			}
		}
	}

	//然后依据漫游产生的迭代器位置，继续正常的逻辑执行。
	while(ExePipe.IsAlive() && It!=m_ActomList.end()){ 

		Mass* Child = *It++;
	
		if(Child->MassType() == MASS_ELEMENT){
			CElement* Elt = (CElement*)Child;
			if(!Elt->Do(Dialog,ExePipe,LocalAddress,Msg)){
				ExePipe.SetID(RETURN_ERROR);
				if (ExePipe.GetLabel().size()==0)
				{
					ExePipe.GetLabel() = Format1024(_T("%I64ld: %s execute failure."),Child->m_ID,Child->GetName().c_str());
				}
			};
		}
		else {					
			ExePipe.AutoTypeAB();   
			uint32 Type = Child->GetTypeAB();   

			if( !ExePipe.HasTypeAB(Type))  
			{
				ExePipe.SetID(RETURN_ERROR);
				ExePipe.GetLabel() = Format1024(_T("%I64ld: %s input data type checking failure:(%x,%x)"),Child->m_ID,Child->GetName().c_str(),Type,ExePipe.GetTypeAB());
			}else{								
				if(!Child->Do(&ExePipe)){
					ExePipe.SetID(RETURN_ERROR);
					if (ExePipe.GetLabel().size()==0){
						ExePipe.GetLabel() = Format1024(_T("%I64ld: %s execute failure."),Child->m_ID,Child->GetName().c_str());
					}
				};     
			}			
		};
	
	 	
		int64 State = ExePipe.GetID();

		//错误视为正常，不影响执行下一个
		/*if (State == RETURN_ERROR)
		{
		tstring& error = ExePipe.GetLabel();
		Dialog->RuntimeOutput(error.c_str());
		error = _T("");
		}
		ExePipe.SetID(RETURN_NORMAL);*/

	}
	Dialog->SetWorkMode(WORK_TASK);
	return true;
}