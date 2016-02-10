/*
*ePipeline is similar to a piece of wire provides unified data input and output channel for other Mass instances.
*
*The data is like an electrons in ePipeline, deleted and disappeared automatically after use.It avoids C++ garbage problem.
*
*ePipeline itself can also be used as a data type that supports nested itself, so you can use it to express arbitrary data structures.
*
*Through serializing as string, ePipeline can be easily implement cross-domain transfer and storage.

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

	class  ePipeline;
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
		bool   FromString(AnsiString& s,uint32& pos){ 
			assert(m_Pointer != NULL);	
			return m_Pointer->FromString(s,pos);
		};	
	};


class  ePipeline : public Energy  
{
	SUPPORT_ABSTRACT_SAPCE_POOL(ePipeline);
public:
    typedef deque<Energy*>                  EnergyList;
	typedef deque<Energy*>::iterator        EnergyPtr;
 	typedef deque<Energy*>::const_iterator  ConstEnergyPtr;
//    friend class ePipeline;
protected:
	TypeAB          m_TypeAB;     //Data type abbreviation,one can express the first 8 data type
	EnergyList      m_EnergyList;

	bool            m_Alive;      //FALSE for breaking this pipe, generally always TRUE, , only for internal use

public:
    int64           m_ID;       
	tstring         m_Label;      //Users filled it for their needs,default usually is ePipeline name.
     

public:
	ePipeline();
	ePipeline(int64 ID);
	ePipeline(const wchar_t* Text,int64 ID );
	virtual ~ePipeline();
		
	ePipeline(const ePipeline& C) { Clone(C);};
	ePipeline& operator=(const ePipeline& C ){
		if(this != &C)Clone(C);
		return *this;
	};



    //Transfers the data in a pipe to the end of this pipe, the C pipe will lose all data.  
	ePipeline& operator << (ePipeline& C);

	virtual eType EnergyType(){ return TYPE_PIPELINE;};	 	

	void Clear();
	
	void UndeleteClear(){
		m_EnergyList.clear();
	};
    // Full replication
    void Clone(const ePipeline &C);
  
	int  Size(){ return m_EnergyList.size();};  
    int  Length(){ return Size();}
	
	void        SetID(uint64 ID){m_ID = ID;};
	uint64      GetID()const {return m_ID;};
	tstring&    GetLabel(){ return m_Label;};
	void        SetLabel(const wchar_t* Text){ m_Label=Text;};

	//Disable this pipe
	void Break(){ m_Alive = false;};

	bool IsAlive(){ return m_Alive;};
    void Reuse(){ m_Alive = true;};

	virtual void*  Value()const { return (void *)this;};
    virtual Energy* Clone(){ return new ePipeline(*this);};   
    

	/*
	 ePipeline is one of the basic data types, so it can also be converted into the following format:
	  type@IDlen@ID@Lablelen@Lable@len@data

     Note that it has a extra member ID.
     Due to the Pipeline save all other data, the format of the data section is :
 
	     type@id@len@type@len@data1 ... type@len@dataN
		             |-------------DATA--------------|  
		             
	 ePipeline can be nested.

	 This format is only tentative and may be changed in the future.
	*/
	void ToString(AnsiString& s);

	//Note: the inputted string format is: TYPE_PIPELINE@ID@LEN@type@len@data1type@len@data2 ... type@len@dataN
	bool  FromString(AnsiString& s,uint32& pos);		
	
	/*for javascrip, a ePipeline is a JSON Object 
	   "Pipe": {
		   "ID": int64
		   "Label": "string"
		   "Data": [
				"eNULL":"NULL",
				"eINT": int64,
				"eFloat":float,
				"eSTRING": "string",
				Pipe: {
					....
				}
		   ]
	   }
	*/
	void ToJSON(AnsiString& s);


	/*
	Before the pipe flowed into a Mass instance, the system will automatically call this function to check
    the type abbreviation whether is consistent with the instance required,
    which is equal to the parameter checking in C language.
    Unless the parameter is more than eight or have special requirements,
    users no longer have to do same thing by themself	
	*/
	TypeAB GetTypeAB(){return m_TypeAB;};
	void   SetTypeAB(uint32 t){ m_TypeAB = t;};

	/*
	  Reset the type abbreviation of first eight data. 
	  It should be called when the data of ePipeline have been changed 
	  so that it correctly flow into the next Mass instance, 
	  usually this work can be carried out by the system automatically 
	  but it only help you check the first eight, 
	  if you want to use the data more than 8, 
	  then you need to check it by yourself.
	*/
    void AutoTypeAB();

	bool  HasTypeAB(uint32 t){
	    AutoTypeAB();
		uint32 n =t&0xf0000000;
		if(n==0)return true;
		if(n!=(m_TypeAB & 0xf0000000))return false;
	
		n=t&0x0f000000;
		if(n==0)return true;
		if(n!=(m_TypeAB & 0x0f000000))return false;

		n=t&0x00f00000;
		if(n==0)return true;
		if(n!=(m_TypeAB & 0x00f00000))return false;

		n=t&0x000f0000;
		if(n==0)return true;
		if(n!=(m_TypeAB & 0x000f0000))return false;

		n=t&0x0000f000;
		if(n==0)return true;
		if(n!=(m_TypeAB & 0x0000f000))return false;

		n=t&0x00000f00;
		if(n==0)return true;
		if(n!=(m_TypeAB & 0x00000f00))return false;

		n=t&0x000000f0;
		if(n==0)return true;
		if(n!=(m_TypeAB & 0x000000f0))return false;

		n=t&0x0000000f;
		if(n==0)return true;
		if(n!=(m_TypeAB & 0x0000000f))return false;

		return true;
	};

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
		tstring s = UTF8toWS(Value);
		eSTRING* Data = new eSTRING(s);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};

	//Stored binary data in std::string 
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

	void PushPipe(const ePipeline& Data){    
	    ePipeline* Pipe = new ePipeline(Data);
		assert(Pipe);
		m_EnergyList.push_back(Pipe);   
	};
		
	/*
      eElectron is a auto_ptr
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

	//Stored binary data in std::string 
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

		
	/*
	  Here ePipeline is equivalent to a c struct, except that its length and content can be dynamically changed.
	  Sometimes we need to treat it as a fixed structure and view specific data.

	  We can use a enum{DATA1=0, ...} to define the data order, and then access the data like a structure member
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
	void* GetData(uint32 Pos)  
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

	//Inserted pipes will lose data
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

		for (uint32 i=0; i<Len;++i)
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
		for (uint32 i=Pos; i<n;++i)
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
	
	/*
	Get the total number of child pipe, including this pipe
	*/
	int32 GetPipeCount();
	

};


}; //namespace ABSTRACT

#endif //  _PIPELINE_H_ 