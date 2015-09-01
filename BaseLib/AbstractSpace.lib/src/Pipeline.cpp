

#include "ABSpacePool.h"
#include "Pipeline.h"
#include <stdio.h>

namespace ABSTRACT{

	ePipeline::ePipeline()
		: m_TypeAB(0),
		  m_EnergyList(),
		  m_Alive(1),
		  m_ID(0),
		  m_Label()
	{
	};

	ePipeline::ePipeline(uint64 ID)
	: m_TypeAB(0),
	  m_EnergyList(),
	  m_Alive(1),
	  m_ID(ID),
	  m_Label()
	{
	};

	ePipeline::ePipeline(const wchar_t* Text,uint64 ID /*= 0*/)
	: m_TypeAB(0),
	  m_EnergyList(),
	  m_Alive(1),
	  m_ID(ID),
	  m_Label(Text)
	{
	};

	ePipeline::~ePipeline(){
		Clear();
	};

	void ePipeline::Clone(const ePipeline &C){
		//完全复制
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
        //data.resize(count); 没有必要,因为PrintString会从data中取出指定数目count的字符
		const char* pdata = data.c_str();
		PrintString(s,TYPE_PIPELINE,m_ID,m_Label,data.size(),pdata);
	};

	//注意：输入的字符串格式为：TYPE_PIPELINE@ID@len@Name@LEN@ type@len@data1type@len@data2 ... type@len@dataN
	bool  ePipeline::FromString(AnsiString& s,uint32& pos){ 
		 Clear();

		 if(s.size()-pos<9 || (s[pos]!='9' && s[pos+1]!='@')){
			 	return false;
		 }
		 
		 //get m_ID
		 uint32 start=pos+2;
 
		 uint32 slen = FindInt(s,start,'@');
		 if(slen==0 || slen>20){
			 return false;
		 }
		 m_ID = StringToInt(&s[start],slen);

		 //Get m_Lable len
		 start += slen+1;
		 slen = FindInt(s,start,'@');
		 if(slen==0 || slen>20){
			 return false;
		 }
		 uint32 len = (uint32)StringToInt(&s[start],slen);
         
		 //Get m_Lable
		 start += slen+1;
		 string temp = s.substr(start,len);
         eSTRING wLable;

		 uint32 p=0;
		 bool ret = wLable.FromString(temp,p);
		 if(!ret){
			 return false;
		 }
		 m_Label = wLable();

		 //Get Data length
		 start += len+1;
		 slen = FindInt(s,start,'@');
		 if(slen==0 || slen>20){
			 return false;
		 }
		 len = (uint32)StringToInt(&s[start],slen);
		 
		 start += slen+1;
		 if(len ==0){
			 pos = start;
			 return true;
		 }
         if(len > (s.size()-start)){
			 return false;
		 }

		 //Get Child Data
		 pos =start;
		 uint32 end = len + start;
		 
		 while(pos<end)
		 {	
			 char Type = s[pos];
			 switch(Type){        //01234567891011
			 case '0':
				 {
			         //0@0@
					 eNULL* v = new eNULL();
					 Push_Directly(v);
					 ret = v->FromString(s,pos);
					 if(!ret){
						 return false;
					 }
				 }
				 break;
			 case '1':
				 {
					 eINT* v = new eINT();
					 Push_Directly(v);
					 ret = v->FromString(s,pos);
					 if(!ret){
						 return false;
					 }
				 }
				 break;
			 case '2':
				 {
					 eFLOAT* v = new eFLOAT();
					 Push_Directly(v);
					 ret = v->FromString(s,pos);
					 if(!ret){
						 return false;
					 }
				 }
				 break;
			 case '3':
				 {
					 eSTRING* v = new eSTRING();
					 Push_Directly(v);
					 ret = v->FromString(s,pos);
					 if(!ret){
						 return false;
					 }
				 }
				 break;

			 case '4':
				 {
					 ePipeline* v = new ePipeline();
					 Push_Directly(v);
					 ret = v->FromString(s,pos);
					 if(!ret){
						 return false;
					 }
				 }
				 break;
			 case '5':
				 {
					eBLOB* b = new eBLOB;
                    Push_Directly(b);
					ret = b->FromString(s,pos);
					if(!ret){
						return false;
					}
				 }
				 break;
			 default:
				 {
					assert(0);
					return false;
				 }
			}//switch								 
		} // for
        assert(pos ==  end);
		return true;
	};	
}//namespace ABSTRACT
