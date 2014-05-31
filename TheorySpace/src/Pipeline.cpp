

#include "AbstractSpacePool.h"
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
		// ��ȫ����
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

		// ResetTypeAB();   ��ϵͳ����
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
        //data.resize(count);  û�б�Ҫ,��ΪPrintString���data��ȡ��ָ����Ŀcount���ַ�
		const char* pdata = data.c_str();
		PrintString(s,TYPE_PIPELINE,m_ID,m_Label,data.size(),pdata);
	};

	//ע�⣺������ַ��ʽΪ��TYPE_PIPELINE@ID@len@Name@LEN@ type@len@data1type@len@data2 ... type@len@dataN
	uint32  ePipeline::FromString(AnsiString& s,uint32 pos/*=0*/){ 
		 Clear();

		 //����
		 if(s.size()-pos<9 || (s[pos]!='9' && s[pos+1]!='@')){
			 	throw _T("Pipe.FromString() fail.");
		 }
		 
		 //��ID
		 uint32 start=pos+2;

		 //�ҵ�������Ƿ�Ϊ����  
		 uint32 slen = FindInt(s,start,'@');
		 if(slen==0 || slen>20){
			 throw _T("Pipe.FromString() fail.");
		 }
		 m_ID = StringToInt(&s[start],slen);

		 //��m_Lable len
		 start += slen+1;
		 slen = FindInt(s,start,'@');
		 if(slen==0 || slen>20){
			 throw _T("Pipe.FromString() fail.");
		 }
		 uint32 len = StringToInt(&s[start],slen);
         
		 //�õ� m_Lable
		 start += slen+1;
		 string temp = s.substr(start,len);
         eSTRING wLable;
		 wLable.FromString(temp,0);
		 m_Label = wLable();

		 //����ݳ���
		 start += len+1;
		 slen = FindInt(s,start,'@');
		 if(slen==0 || slen>20){
			 throw _T("Pipe.FromString() fail.");
		 }
		 len = StringToInt(&s[start],slen);
		 
		 start += slen+1;
		 if(len ==0)return  start - pos;
         if(len > (s.size()-start)){
			 throw _T("Pipe.FromString() fail.");
		 }

		 //���������͵������
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
