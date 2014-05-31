/*
* ePipeline����������ռ�ʵ���ṩͳһ�������������ܵ���
*
* �������Ϊһ����ߣ�����������ݶ���Լ����ePipeline�ÿһ�����ʹ�ú���������
* �ĵ��ӻ��Զ���ʧ������Խ��C���������ٵ��ڴ�������⡣
*
* author: ZhangHongBing(hongbing75@gmail.com)   
*/

#ifndef _PIPELINE_H_ 
#define _PIPELINE_H_ 


#include "AbstractSpace.h"
#include "BaseEnergy.h"
#include <deque>
#include <string>
#include "Msg.h"

using namespace std;

namespace ABSTRACT {


class ePipeline;

/*

*/

class  eElectron: public Energy{

	friend  class ePipeline;
    Energy* m_Pointer;

public:			
	void Reset(Energy* p = 0)
	{
		if(p!= m_Pointer){
			delete m_Pointer;
			m_Pointer = p;
		}
	}

	Energy* Get() const { return m_Pointer; }

 	Energy* Release()
	{
		Energy* old = m_Pointer;
		m_Pointer = NULL;
		return old;
	}   
public:
	eElectron(Energy* P = NULL):m_Pointer(P){};		
	eElectron(eElectron& e): m_Pointer(e.Release()){};
    virtual ~eElectron() { 
		if(m_Pointer)delete m_Pointer; 
		m_Pointer = NULL;
	}

    eElectron& operator=(eElectron& e)
	{
		if(this != &e) Reset(e.Release());
		return *this;
	}
public:
	
    eType EnergyType(){
		assert(m_Pointer !=NULL);
		return m_Pointer->EnergyType();
	}

	virtual tstring      GetName(){
		assert(m_Pointer !=NULL);
		return m_Pointer->GetName();
	};

	Energy*  Clone(){
		assert(m_Pointer !=NULL);
		return m_Pointer->Clone();
	}
    
    void*  Value()const { 
		assert(m_Pointer !=NULL);		
		return m_Pointer->Value(); 
	}
    
    int64     Int64(){ return *(int64*)Value();};
    float64   Float64(){ return *(float64*)Value();};
    tstring&  String(){ return *(tstring*)Value();};
    

	void ToString(AnsiString& s){
		assert(m_Pointer != NULL);
	    m_Pointer->ToString(s);
	};
	uint32   FromString(AnsiString& s,uint32 pos){ 
	    assert(m_Pointer != NULL);	
	  	return m_Pointer->FromString(s,pos);
	};	
};


/*
  ePipeline�����ԭ�������Ƚ��ȳ�������������д
*/


class  ePipeline : public Energy  
{
	SUPPORT_ABSTRACT_SAPCE_POOL(ePipeline);
public:
    typedef deque<Energy*>                  EnergyList;
	typedef deque<Energy*>::iterator        EnergyPtr;
 	typedef deque<Energy*>::const_iterator  ConstEnergyPtr;
//    friend class ePipeline;
protected:
	TypeAB          m_TypeAB;     //����������д��һ�ο���ͬʱ����ǰ8�����
	EnergyList      m_EnergyList;

	bool            m_Alive;      //FALSE��ʾ�Ͽ��˹ܵ�,һ������ΪTRUE��һ��ֻ��Ϊ�ڲ�ʹ��

public:
    uint64          m_ID;       
	tstring         m_Label;      //�û��Լ������Ҫ���ͽ���m_Label,defaultͨ��ΪePipeline name     

public:
	ePipeline();
	ePipeline(uint64 ID);
	ePipeline(const wchar_t* Text,uint64 ID = 0);
	virtual ~ePipeline();
		
	ePipeline(const ePipeline& C) { Clone(C);};
	ePipeline& operator=(const ePipeline& C ){
		if(this != &C)Clone(C);
		return *this;
	};



	//��һ���ܵ�������ת�Ƶ����ܵ�ĩβ��ԭ�ܵ���ʧȥ���е���ݡ�  
	ePipeline& operator << (ePipeline& C);

	virtual eType EnergyType(){ return TYPE_PIPELINE;};	 	

	void Clear();
	
	void UndeleteClear(){
		m_EnergyList.clear();
	};
	// ��ȫ����
    void Clone(const ePipeline &C);
  
	int  Size(){ return m_EnergyList.size();};  
    int  Length(){ return Size();}
	
	void        SetID(uint64 ID){m_ID = ID;};
	uint64      GetID()const {return m_ID;};
	tstring&    GetLabel(){ return m_Label;};
	void        SetLabel(const wchar_t* Text){ m_Label=Text;};

	//���ܵ�ʧЧ��Ŀǰֻ����ϵͳ�ڲ�
	void Break(){ m_Alive = FALSE;};

	bool IsAlive(){ return m_Alive;};
    void Reuse(){ m_Alive = TRUE;};

	//�����ePipeline����Ҳ��������һ�ָ���������ͣ�ͬ����Ա����ݡ�
	void*  Value()const { return (void *)this;};
    Energy* Clone(){ return new ePipeline(*this);};   
    

	/*PipelineҲ��һ�ֻ�������ͣ������Ҳ����ת���ɻ�� type@IDlen@ID@Lablelen@Lable@len@data��ʽ��
	 ע�⣬�������������Ͷ�һ��ID��Ŀ��������ݳ�Ա�������ԡ�
	  
	 ����Pipelineװ�صĶ���������ݣ������data���ֵĸ�ʽΪ��
	     type@id@len@type@len@data1 ... type@len@dataN
		             |-------------DATA--------------|  
	 ����ܵ��ǿ���Ƕ�׵ġ�

	 ע�⣺����ʽֻ���ݶ��������п��ܸı䡣
	*/
	void ToString(AnsiString& s);

	//ע�⣺������ַ��ʽΪ��TYPE_PIPELINE@ID@LEN@type@len@data1type@len@data2 ... type@len@dataN
	uint32  FromString(AnsiString& s,uint32 pos=0);		
	
	/*
	  ͨ���ڹܵ�����ĳһ���ռ�ʵ��֮ǰ��ϵͳ�Զ����ô˺��������
	  �Ƿ�Ϳռ�ʵ���TypeAB()�ӿ�һ�£��൱�����C���ԵĲ�����
	  ��ǲ����8����������Ҫ�󣬷����û��������Լ����ePipeline��
	  ������Ƿ�Ϸ���
	*/
	TypeAB GetTypeAB(){return m_TypeAB;};

	void  SetTypeAB(uint32 t){ m_TypeAB = t;};

	/*
	  ��������ǰ8����ݵ�������д,�ܵ��������ݸı��Ժ�Ӧ�õ���
	  ���������ȷ������ȷ��������һ���ռ�ʵ�������Ҳ����ϵͳ�Զ�
	  ��ɵģ�����ֻ�ܰ�����ǰ8������Ҫȡ������ݳ���8�����������Ҫ
	  �Լ����ô˺���Ȼ��������8������Ƿ�Ϸ���
	*/
    void AutoTypeAB();

	bool  HasTypeAB(uint32 t){
	    AutoTypeAB();
		return (t&m_TypeAB) == t;
	};

	//��ܵ���д��һ����ݣ����¡Ŀ�����,���ƻ�ԭ���
	void Push_Copy(Energy* Data){
		Energy* New = Data->Clone();
		m_EnergyList.push_back(New);
	};
  	

	void Push_Directly(Energy* Data){     
		assert(Data);
		m_EnergyList.push_back(Data);   
	};

    void Push_Front(Energy* Data){
	    m_EnergyList.push_front(Data);
	};
	void PushNull(){
		eNULL* Data = new eNULL;
		assert(Data);
        m_EnergyList.push_back(Data);   
	};

	void PushInt(int64 Value){
		eINT* Data = new eINT(Value);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};
	void PushFloat(float64 Value){
		eFLOAT* Data = new eFLOAT(Value);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};
	void PushString(const wchar_t* Value){
		eSTRING* Data = new eSTRING(Value);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};
	void PushString(const tstring& Value){
		eSTRING* Data = new eSTRING(Value);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};

	void PushString(const AnsiString& Value){
		tstring s = eSTRING::UTF8toWS(Value);
		eSTRING* Data = new eSTRING(s);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};

    void Push256String(wchar_t* Format, ...){
		wchar_t Buffer [256] ;
		va_list ArgList ;		
		va_start (ArgList, Format) ;
#ifdef _WIN32
		_vsntprintf(Buffer, 256, Format, ArgList) ;
#else
		vswprintf(Buffer,256,Format,ArgList);
#endif
		va_end (ArgList) ;
				
		eSTRING* Data = new eSTRING(Buffer);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};

	//��std::stringΪ���崦����������
	void PushBlob(const AnsiString& Value){
		eBLOB* Data = new eBLOB(Value.c_str(),Value.size());
		assert(Data);
        m_EnergyList.push_back(Data);   
	}
	void PushBlob(const char* Value,int32 Len){
		eBLOB* Data = new eBLOB(Value,Len);
		assert(Data);
        m_EnergyList.push_back(Data);   
	}

	void PushPipe(ePipeline& Data){    
		ePipeline* Pipe = new ePipeline;
		*Pipe<<Data;
		Pipe->m_ID = Data.m_ID;
		Pipe->m_Label = Data.m_Label;
		Pipe->m_Alive = Data.m_Alive;
		Pipe->m_TypeAB = Data.m_TypeAB;
		m_EnergyList.push_back(Pipe);   
	};
	/*
      ����һ��auto_ptr��ȡ����ݣ������ʹ��eElectron��������ָ�κ����
	  Ҳ����ʹ�����������eData<class T>,�Ӷ���⴦������ת��
    */
	void Pop(eElectron* Data){	
		assert(m_EnergyList.size() != 0);  
		Energy* e = m_EnergyList.front();
		m_EnergyList.pop_front();
		Data->Reset(e);
	}
	
	void PopBack(eElectron* Data){	
		assert(m_EnergyList.size() != 0);  
		Energy* e = m_EnergyList.back();
		m_EnergyList.pop_back();
		Data->Reset(e);
	}

	int64 PopInt(){
		assert(m_EnergyList.size() != 0);  
		Energy* e = m_EnergyList.front();
		m_EnergyList.pop_front();
		assert(e->EnergyType() == TYPE_INT);
		int64 v =  *(int64*)e->Value();
		delete e;
		e = NULL;
		return v;
	}

	float64 PopFloat(){
		assert(m_EnergyList.size() != 0);  
		Energy* e = m_EnergyList.front();
		m_EnergyList.pop_front();
		assert(e->EnergyType() == TYPE_FLOAT);
		float64 v =  *(float64*)e->Value();
		delete e;
		e = NULL;
		return v;
	}
	tstring PopString(){
		assert(m_EnergyList.size() != 0);  
		Energy* e = m_EnergyList.front();
		m_EnergyList.pop_front();
		assert(e->EnergyType() == TYPE_STRING);
		tstring v =  *(tstring*)e->Value();
		delete e;
		e = NULL;
		return v;
	}
	void PopString(tstring& Value){
		assert(m_EnergyList.size() != 0);  
		Energy* e = m_EnergyList.front();
		m_EnergyList.pop_front();
		assert(e->EnergyType() == TYPE_STRING);
		Value =  *(tstring*)e->Value();
		delete e;
		e = NULL;
	}

	//��std::stringΪ����洢���������
	void PopBlob(AnsiString& Blob){
		assert(m_EnergyList.size() != 0);  
		Energy* e = m_EnergyList.front();
		m_EnergyList.pop_front();
		assert(e->EnergyType() == TYPE_BLOB);
		Blob =  *(AnsiString*)e->Value();
		delete e;
		e = NULL;
	}

	void PopMsg(CMsg& Msg ){
		ePipeline* p = NULL;
		assert(m_EnergyList.size() != 0);
		  
		Energy* e = m_EnergyList.front();
		assert(e->EnergyType() == TYPE_PIPELINE);
		m_EnergyList.pop_front();
		p = (ePipeline*)e;
		Msg.Reset(p);
	}

	/*ePipeline��ʵ�൱��һ��C�����е�struct��ֻ��������ݳ��Ⱥ����ݿ��Զ�̬�ı�
	  ��ʱ������Ҫ������һ���̶��ṹ��鿴���ض�����ݡ�

      ���ǿ���ͨ��һ��enum{DATA1=0,...}������ݱ������,Ȼ����ṹһ���������ݳ�Ա
	*/
	
	Energy* GetEnergy(uint32 Pos)
	{
		assert(Pos<m_EnergyList.size());
		Energy* Data = m_EnergyList[Pos];
		return Data;
	};
    Energy* GetLastEnergy(){
		if(m_EnergyList.size()==0)return NULL;
		return m_EnergyList.back();
	}
	void* GetData(uint32 Pos)  //����ʹ��ģ�壬�����ǳ���Ա��ʽ��ת����ã�������ò�˼��һ���Լ���Ҫ��������ʲô���͵����
	{
		assert(Pos<m_EnergyList.size());
		Energy* Data = m_EnergyList[Pos];
		return Data->Value();
	}

	void* GetLastData(){
		if(m_EnergyList.size()==0)return NULL;
		return ((Energy*)m_EnergyList.back())->Value();
	}
	ENERGY_TYPE GetDataType(uint32 Pos)
	{
		assert(Pos<m_EnergyList.size());
		Energy* Data = m_EnergyList[Pos];
		return Data->EnergyType();
	}
	
	void InsertEnergy(uint32 Pos,Energy* E)
	{
		assert(Pos<=m_EnergyList.size());
		assert(E);
		m_EnergyList.insert(m_EnergyList.begin()+Pos,1,E);
	}

	//����ܵ���ʧȥ���
	void InsertEnergy(uint32 Pos,ePipeline& Pipe)
	{
		assert(Pos<=m_EnergyList.size());
		assert(Pipe.Size());
		m_EnergyList.insert(m_EnergyList.begin()+Pos,Pipe.m_EnergyList.begin(),Pipe.m_EnergyList.end());
		Pipe.m_EnergyList.clear();
	}

	void EraseEnergy(uint32 Pos,uint32 Len){
		assert(Pos+Len<=m_EnergyList.size());
		EnergyPtr It = m_EnergyList.begin()+Pos;

		for (uint i=0; i<Len;++i)
		{
			Energy* E = *It++;
			delete E;
		}
		It = m_EnergyList.begin()+Pos;
		m_EnergyList.erase(It,It+Len);
	}
	void EraseBackEnergy(){
		assert(m_EnergyList.size());
		Energy* E = m_EnergyList.back();
		delete E;
		m_EnergyList.pop_back();
	}
	void TransEnergy(uint32 Pos,uint32 Len,ePipeline* Pipe){
		uint32 n = Pos+Len;
		assert(n<=m_EnergyList.size());
		assert(Pipe);
		for (uint i=Pos; i<n;++i)
		{
			Energy* E = m_EnergyList[i];
			Pipe->Push_Directly(E);
		}
		EnergyPtr It = m_EnergyList.begin()+Pos;
		m_EnergyList.erase(It,It+Len);
	}	
	
	Energy* ReplaceEnergy(uint32 Pos, Energy* New){
		assert(Pos<m_EnergyList.size());
		Energy* Old =  m_EnergyList[Pos];
		m_EnergyList[Pos] = New;
		return Old;
	}
	//no delete
	Energy* MoveEnergy(uint32 Pos){
		assert(Pos<m_EnergyList.size());
		EnergyPtr It = m_EnergyList.begin()+Pos;
		Energy* e = *It;
		m_EnergyList.erase(It);
		return e;
	}
	
	/*�õ�����PIPE������Ŀ���������ܵ�*/
	int32 GetPipeCount();
	

};


}; //namespace ABSTRACT

#endif //  _PIPELINE_H_                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
