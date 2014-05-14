#pragma warning(disable:4786)

#include "AbstractSpacePool.h"
#include "Pipeline.h"
#include <stdio.h>

namespace ABSTRACT{

	ePipeline::ePipeline()
		: m_Label(),
		  m_EnergyList(),
		  m_TypeAB(0),
		  m_Alive(1),
		  m_ID(0)
	{
	};

	ePipeline::ePipeline(uint64 ID)
		: m_Label(),
		  m_EnergyList(),
		  m_TypeAB(0),
		  m_Alive(1),
		  m_ID(ID)
	{
	};

	ePipeline::ePipeline(const TCHAR* Text,uint64 ID /*= 0*/)
		: m_Label(Text),
		  m_EnergyList(),
		  m_TypeAB(0),
		  m_Alive(1),
		  m_ID(ID)
	{
	};

	ePipeline::~ePipeline(){
		Clear();
	};
	void ePipeline::Clone(const ePipeline &C){
		// 完全复制
		Clear();
		ConstEnergyPtr It=C.m_EnergyList.begin();
		while(It !=C.m_EnergyList.end()){
			m_EnergyList.push_back((*It)->Clone());
			It++;
		}
		m_TypeAB = C.m_TypeAB;
		m_Alive  = C.m_Alive;
		m_Label  = C.m_Label;
		m_ID     = C.m_ID;
	}     
	
	ePipeline& ePipeline::operator << (ePipeline& C)
	{
		if(this == &C)return *this;
		EnergyPtr p = C.m_EnergyList.begin();
		while(p != C.m_EnergyList.end()){
			m_EnergyList.push_back(*p);
			p++;
		}
		C.m_EnergyList.clear();

		// ResetTypeAB();   由系统来做
		return *this;
	}


	void ePipeline::Clear(){
		EnergyPtr It = m_EnergyList.begin();
		while(It != m_EnergyList.end()){
		   delete *It;
		   *It = NULL;
		   It ++;
		}
		m_EnergyList.clear();
	}

    void ePipeline::AutoTypeAB()
	{    
		m_TypeAB = 0;
		EnergyPtr p = m_EnergyList.begin();
		int32 s = m_EnergyList.size();
		s=(s<8)?s:8;
		for(int i=0; i<s; i++)
		{
			m_TypeAB |= (((*p)->EnergyType())<<4*(7-i));
			p++;
		}
	}

	void ePipeline::PrintLabel(TCHAR* Format, ...){
		TCHAR Buffer [512] ;
		va_list ArgList ;		
		va_start (ArgList, Format) ;
		_vsntprintf(Buffer, 512, Format, ArgList) ;		
		va_end (ArgList) ;

		m_Label = Buffer;
	};
	int32 ePipeline::GetPipeCount(){
		EnergyPtr It = m_EnergyList.begin();
		int32 Count = 1;
		while(It != m_EnergyList.end()){
			if((*It)->EnergyType() == TYPE_PIPELINE){
				Count += ((ePipeline*)(*It))->GetPipeCount();
			}
		    It ++;
		}
		return Count;
	};
	void ePipeline::ToString(AnsiString& s){		
		AnsiString data;
		EnergyPtr It = m_EnergyList.begin();		
        while(It != m_EnergyList.end()){
			Energy* e = (*It);
			e->ToString(data); 
			It++;
		}
        //data.resize(count);  没有必要,因为PrintString会从data中取出指定数目count的字符
		const char* pdata = data.c_str();
		PrintString(s,TYPE_PIPELINE,m_ID,m_Label,data.size(),pdata);
	};

	//注意：输入的字符串格式为：TYPE_PIPELINE@ID@len@Name@LEN@ type@len@data1type@len@data2 ... type@len@dataN
	uint32  ePipeline::FromString(AnsiString& s,uint32 pos/*=0*/){ 
		 Clear();

		 //基本检查
		 if(s.size()-pos<9 || (s[pos]!='9' && s[pos+1]!='@')){
			 	throw std::exception("Pipe.FromString() fail.");
		 }
		 
		 //找ID
		 uint32 start=pos+2;

		 //找到并检查是否为整数；  
		 uint32 slen = FindInt(s,start,'@');
		 if(slen==0 || slen>20){
			 throw std::exception("Pipe.FromString() fail.");
		 }
		 m_ID = StringToInt(&s[start],slen);

		 //找m_Lable len
		 start += slen+1;
		 slen = FindInt(s,start,'@');
		 if(slen==0 || slen>20){
			 throw std::exception("Pipe.FromString() fail.");
		 }
		 uint32 len = StringToInt(&s[start],slen);
         
		 //得到 m_Lable
		 start += slen+1;
		 string temp = s.substr(start,len);
         eSTRING wLable;
		 wLable.FromString(temp,0);
		 m_Label = wLable();

		 //找数据长度
		 start += len+1;
		 slen = FindInt(s,start,'@');
		 if(slen==0 || slen>20){
			 throw std::exception("Pipe.FromString() fail."); 
		 }
		 len = StringToInt(&s[start],slen);
		 
		 start += slen+1;
		 if(len ==0)return  start - pos;
         if(len > (s.size()-start)){
			 throw std::exception("Pipe.FromString() fail.");
		 }

		 //解析各类型的子数据
		 uint32 i=start;
		 len += start;
		 uint32 n;
		 while(i<len)
		 {	
			 char Type = s[i];
			 switch(Type){        //01234567891011
			 case '0':
				 {
			         //0@0@
					 if(s[i+1] != '@')return 0;
					 eNULL* v = new eNULL();
					 Push_Directly(v);
					 n = v->FromString(s,i);
				 }
				 break;
			 case '1':
				 {
					 eINT* v = new eINT();
					 Push_Directly(v);
					 n = v->FromString(s,i);
				 }
				 break;
			 case '2':
				 {
					 eFLOAT* v = new eFLOAT();
					 Push_Directly(v);
					 n = v->FromString(s,i);
				 }
				 break;
			 case '3':
				 {
					 eSTRING* v = new eSTRING();
					 Push_Directly(v);
					 n = v->FromString(s,i);
				 }
				 break;

			 case '4':
				 {
					 ePipeline* v = new ePipeline();
					 Push_Directly(v);
					 n = v->FromString(s,i);					 
				 }
				 break;
			 case '5':
				 {
					eBLOB* b = new eBLOB;
                    Push_Directly(b);
					n = b->FromString(s,i);
				 }
				 break;
			 default:
				 {
					assert(0);
					return 0;
				 }
			}//switch				
			if(n==0)return 0;					 
			i += n;	
		} // for
        n =  len - pos;
		return n;
	};	
}//namespace ABSTRACT