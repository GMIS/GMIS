/*
* ePipeline用来给其他空间实例提供统一的数据输入输出管道。
*
* 可以理解为一根电线，由于所有数据都被约束在ePipeline里，每一个数据使用后就象电流里
* 的电子会自动消失，这可以解决C语言所面临的内存回收难题。
*
* author: ZhangHongBing(hongbing75@gmail.com)   
*/

#ifndef _PIPELINE_H_ 
#define _PIPELINE_H_ 

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "AbstractSpace.h"
#include "BaseEnergy.h"
#include <deque>
#include <string>
#include "Msg.h"
#include <stdarg.h>

using namespace std;

namespace ABSTRACT {


class ePipeline;

/*

*/

class  eElectron: public Energy{

	friend  ePipeline;
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
  ePipeline的数据原则上是先进先出，不鼓励随机读写
*/


class  ePipeline : public Energy  
{
	SUPPORT_ABSTRACT_SAPCE_POOL(ePipeline);
public:
    typedef deque<Energy*>                  EnergyList;
	typedef deque<Energy*>::iterator        EnergyPtr;
 	typedef deque<Energy*>::const_iterator  ConstEnergyPtr;
    friend class ePipeline;
protected:
	TypeAB          m_TypeAB;     //能量类型缩写，一次可以同时处理前8个数据
	EnergyList      m_EnergyList;

	bool            m_Alive;      //FALSE表示断开此管道,一般总是为TRUE，一般只作为内部使用

public:
    uint64          m_ID;       
	tstring         m_Label;      //用户自己根据需要填充和解释m_Label,default通常为ePipeline name     

public:
	ePipeline();
	ePipeline(uint64 ID);
	ePipeline(const TCHAR* Text,uint64 ID = 0);
	virtual ~ePipeline();
		
	ePipeline(const ePipeline& C) { Clone(C);};
	ePipeline& operator=(const ePipeline& C ){
		if(this != &C)Clone(C);
		return *this;
	}



	//把一个管道里的数据转移到本管道末尾，原管道将失去所有的数据。  
	ePipeline& operator << (ePipeline& C);

	virtual eType EnergyType(){ return TYPE_PIPELINE;};	 	

	void Clear();
	
	void UndeleteClear(){
		m_EnergyList.clear();
	};
	// 完全复制
    void Clone(const ePipeline &C);
  
	int  Size(){ return m_EnergyList.size();};  
    int  Length(){ return Size();}
	
	void        SetID(uint64 ID){m_ID = ID;};
	uint64      GetID()const {return m_ID;};
	tstring&    GetLabel(){ return m_Label;};
	void        SetLabel(const TCHAR* Text){ m_Label=Text;}; 

	//本管道失效，目前只用于系统内部
	void Break(){ m_Alive = FALSE;}

	bool IsAlive(){ return m_Alive;};
    void Reuse(){ m_Alive = TRUE;}

	//这表明ePipeline本身也被看作是一种复合数据类型，同样可以被传递。
	void*  Value()const { return (void *)this;}  
    Energy* Clone(){ return new ePipeline(*this);};   
    

	/*Pipeline也是一种基本数据类型，因此它也将被转换成基本的 type@IDlen@ID@Lablelen@Lable@len@data格式，
	 注意，它别其他数据类型多一个ID项目，其他数据成员将被忽略。
	  
	 由于Pipeline装载的都是其他数据，因此在data部分的格式为：
	     type@id@len@type@len@data1 ... type@len@dataN
		             |-------------DATA--------------|  
	 另外管道是可以嵌套的。

	 注意：本格式只是暂定，将来有可能改变。
	*/
	void ToString(AnsiString& s);

	//注意：输入的字符串格式为：TYPE_PIPELINE@ID@LEN@type@len@data1type@len@data2 ... type@len@dataN
	uint32  FromString(AnsiString& s,uint32 pos=0);		
	
	/*
	  通常在管道流入某一个空间实例之前，系统自动调用此函数来检查
	  是否和空间实例的TypeAB()接口一致，相当于完成C语言的参数检查
	  除非参数超过8个或有特殊要求，否则用户不必再自己检查ePipeline里
	  的数据是否合法。
	*/
	TypeAB GetTypeAB(){return m_TypeAB;};

	void  SetTypeAB(uint32 t){ m_TypeAB = t;}

	/*
	  重新设置前8个数据的类型缩写,管道保存的数据改变以后都应该调用
	  这个函数以确保它正确流如入下一个空间实例，这项工作也是由系统自动
	  完成的，但它只能帮你检查前8个，你要取出的数据超过8个，那你就需要
	  自己调用此函数然后检查随后的8个数据是否合法。
	*/
    void AutoTypeAB();

	bool  HasTypeAB(uint32 t){
	    AutoTypeAB();
		return (t&m_TypeAB) == t;
	}

	//往管道里写入一个数据，它克隆目标数据,而不破坏原数据
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
	}
	void PushFloat(float64 Value){
		eFLOAT* Data = new eFLOAT(Value);
		assert(Data);
        m_EnergyList.push_back(Data);   
	}
	void PushString(const TCHAR* Value){
		eSTRING* Data = new eSTRING(Value);
		assert(Data);
        m_EnergyList.push_back(Data);   
	}
	void PushString(const tstring& Value){
		eSTRING* Data = new eSTRING(Value);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};
#ifdef _UNICODE
	void PushString(const AnsiString& Value){
		tstring s = eSTRING::UTF8TO16(Value);
		eSTRING* Data = new eSTRING(s);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};
#endif

    void Push256String(TCHAR* Format, ...){
		TCHAR Buffer [256] ;
		va_list ArgList ;		
		va_start (ArgList, Format) ;
		_vsntprintf(Buffer, 256, Format, ArgList) ;		
		va_end (ArgList) ;
				
		eSTRING* Data = new eSTRING(Buffer);
		assert(Data);
        m_EnergyList.push_back(Data);   
	};

	//以std::string为载体处理二进制数据
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
      它让一个auto_ptr来取出数据，你可以使用eElectron对象来泛指任何数据
	  也可以使用它的派生类eData<class T>,从而避免处理类型转换
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

	//以std::string为载体存储二进制数据
	void PopBlob(AnsiString& Blob){
		assert(m_EnergyList.size() != 0);  
		Energy* e = m_EnergyList.front();
		m_EnergyList.pop_front();
		assert(e->EnergyType() == TYPE_BLOB);
		Blob =  *(AnsiString*)e->Value();
		delete e;
		e = NULL;
	}
	CMsg PopMsg(){
		ePipeline* p = NULL;
		if(m_EnergyList.size() != 0){
			CMsg m(p);
			return m;
		};  
		Energy* e = m_EnergyList.front();
		assert(e->EnergyType() == TYPE_PIPELINE);
		m_EnergyList.pop_front();
		p = (ePipeline*)e;
		CMsg m(p);
		return m;
	}

	/*ePipeline其实相当于一个C语言中的struct，只不过其数据长度和内容可以动态改变
	  有时我们需要把它当作一个固定结构而查看起特定的数据。

      我们可以通过一个enum{DATA1=0,...}定义数据保存次序,然后象结构一样访问其数据成员
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
	void* GetData(uint32 Pos)  //曾考虑使用模板，不过还是程序员显式的转换更好，至少他不得不思考一下自己将要操作的是什么类型的数据
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

	//插入管道将失去数据
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
		for (int i=0; i<Len;++i)
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
		for (int i=Pos; i<n;++i)
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
	
	//如果一个空间实例在执行时发生错误，可以由此写入必要的信息到m_Label
	void PrintLabel(TCHAR* Format, ...);

	/*得到所含有PIPE的总数目，包括本管道*/
	int32 GetPipeCount();
	

};


}; //namespace ABSTRACT

#endif //  _PIPELINE_H_ 