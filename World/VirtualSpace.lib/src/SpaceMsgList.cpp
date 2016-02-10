#pragma warning (disable: 4786)

#include "SpaceMsgList.h"


tstring LinkerCode2Str(int32 CodeID){
	static map<int32,tstring>  LinkerCodeList;
	if (LinkerCodeList.size()==0)
	{
		int n = ( sizeof( (_LinkerCodeStrList) ) / sizeof( (_LinkerCodeStrList[0]) ) ) ;
		for (int k=0; k<n; k++)
		{
			LinkerCodeList[(int32)_LinkerCodeStrList[k].msg_id] =_LinkerCodeStrList[k].msg_text;
		}
		
	}
	tstring MsgStr;
	map<int32,tstring>::iterator it = LinkerCodeList.find(CodeID);
	if(it!=LinkerCodeList.end()){
		MsgStr = it->second;		
	}
	
	return MsgStr;
}

void CPipeView::PrintPipe(ePipeline& Pipe,int32 TabNum/*=0*/)
{
	
	TCHAR buffer[200];
    int i;
	int size = Pipe.Length();

	int Tab=TabNum;

	for(i=0; i<Tab; i++){
		buffer[i]=_T('\t');
	}

	//限制嵌套数目
	if(Tab>90){
		throw std::exception("WARNING: Nest Pipe too much, end View");
	}

	tstring PipeLabel = Pipe.m_Label;
	_stprintf(buffer+Tab,_T("Pipe<%s>: %d Data\n"),PipeLabel.c_str(), Pipe.Size());
	m_Result.PushString(buffer);

	Tab = ++TabNum;
	for(i=0; i<Tab; i++){
		buffer[i]=_T('\t');
	}

	for(i=0; i<size; i++){
	   uint32 Type = Pipe.GetDataType(i);
	   switch(Type){
	   case TYPE_NULL:
		   {
		   _stprintf(buffer+Tab,_T("%03d null\n"),i);
		   m_Result.PushString(buffer);
		   }
		   break;

	   case TYPE_INT:
		   {
		   int64* value=(int64 *)Pipe.GetData(i);
           _stprintf(buffer+Tab,_T("%03d int64: %I64ld\n"),i, *value);
           m_Result.PushString(buffer);
		   }
		   break;
	   case TYPE_FLOAT:
		   {
		   float64* value=(float64 *)Pipe.GetData(i);
           _stprintf(buffer+Tab,_T("%03d float64: %f\n"),i, *value);
           m_Result.PushString(buffer);
		   }
		   break;
	   case TYPE_STRING:
		   {
		   tstring* value=(tstring *)Pipe.GetData(i);
           _stprintf(buffer+Tab,_T("%03d string:"),i);
		   //MBPrintf("test1","%s",value->c_str());
		   tstring s(buffer);
		   s +=(*value);
		   s +=_T('\n');

           m_Result.PushString(s);
		   //MBPrintf("test","size=%d",Result.Size());
		   }
		   break;
	   case TYPE_PIPELINE:
		   {
		   ePipeline* value=(ePipeline *)Pipe.GetData(i);
    
		   //如果不用PipeNum++而用t++,导致t增大不能恢复，中间形成空格，之后打印的数据不可见
		   PrintPipe(*value,TabNum);
		   }      
		   break;
	   }
	}
    TabNum--;
}

tstring TriToken(tstring& Token){
	tstring::size_type begin = Token.find_first_not_of(_T(' '));
    if(begin == string::npos)return Token;
    tstring::size_type end = Token.find_last_not_of(_T(' '));
	return Token.substr(begin,end-begin+1);
}