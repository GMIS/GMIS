#include "..\Brain.h"
#include "..\LogicDialog.h"
#include "PipeViewElt.h"

void CPipeViewMass::PrintView(ePipeline& Result,ePipeline& Pipe)
{
	
	TCHAR buffer[1024];
    int i;
	int size = Pipe.Length();

	int Tab=++m_TabNum;

	//限制嵌套数目
	if(Tab>90){
		throw std::exception("WARNING: Nest Pipe too much, end View");
	}
    //Tab = 0;
	for(i=0; i<Tab; i++){
		buffer[i]=_T('\t');
	}
	tstring PipeLabel = Pipe.m_Label;
	if(PipeLabel.size())
	{
		_stprintf(buffer+Tab,_T("Output<%s>:\n"),PipeLabel.c_str());
	}	
	else{
        _stprintf(buffer+Tab,_T("Output:\n"));
	} 
	Result.PushString(buffer);

	for(i=0; i<size; i++){
	   uint32 Type = Pipe.GetDataType(i);
	   switch(Type){
	   case TYPE_NULL:
		   {
		   _stprintf(buffer,_T("%03d NULL:0\n"),i);
		   Result.PushString(buffer);
		   }
		   break;

	   case TYPE_INT:
		   {
		   int64* value=(int64 *)Pipe.GetData(i);
           _stprintf(buffer,_T("%03d INT64: %I64ld\n"),i, *value);
           Result.PushString(buffer);
		   }
		   break;
	   case TYPE_FLOAT:
		   {
		   float64* value=(float64 *)Pipe.GetData(i);
           _stprintf(buffer,_T("%03d FLOAT64: %f\n"),i, *value);
           Result.PushString(buffer);
		   }
		   break;
	   case TYPE_STRING:
		   {
		   tstring* value=(tstring *)Pipe.GetData(i);
           _stprintf(buffer,_T("%03d STRING:"),i);
		   //MBPrintf("test1","%s",value->c_str());
		   tstring s(buffer);
		   s +=(*value);
		   s +=_T('\n');

           Result.PushString(s);
		   //MBPrintf("test","size=%d",Result.Size());
		   }
		   break;
 /*	   case TYPE_STREAM:
		   {
			  
		   eStream* value=(eStream *)Data.Value();
           int size=value->GetDataCount();
		   for(int i=0; i<size; i++){
		      _stprintf(buffer,"%03d STREAM: \n",i);
              eSTRING s(buffer);
		      Result.Push(&s);
			  value->Output(Result);
		   }
		 
		   }
		   break;
  */	   case TYPE_PIPELINE:
		   {
		   ePipeline* value=(ePipeline *)Pipe.GetData(i);
		   //如果不用PipeNum++而用t++,导致t增大不能恢复，中间形成空格，之后打印的数据不可见
		   PrintView(Result,*value);
		   }      
		   break;
	   }
	}
    m_TabNum--;
}	
	
void CPipeViewMass::GetPipeInfo(ePipeline* Pipe,tstring& text){
	m_TabNum = -1;
	ePipeline Result;
	PrintView(Result,*Pipe);
	int len = Result.Length();
    eElectron Data;
	for(int i=0; i<len; i++){
        Result.Pop(&Data);
        text+=*(tstring*)Data.Value();
	}
}

bool CPipeViewMass::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	tstring Text;
	GetPipeInfo(&ExePipe,Text);	
	Dialog->RuntimeOutput(Text);

	return true;
}