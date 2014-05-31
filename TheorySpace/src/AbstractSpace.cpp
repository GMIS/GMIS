
#include "AbstractSpace.h"
#include "BaseEnergy.h"
#include "TimeStamp.h"
#include "AbstractSpacePool.h"


namespace ABSTRACT{
	
	CTimeStamp* AbstractSpace::m_StaticTimer = NULL;
	CAbstractSpacePool*  AbstractSpace::m_StaticSpacePool = NULL;

	void AbstractSpace::InitTimer(CTimeStamp* Timer){
		m_StaticTimer = Timer;
	}

	void AbstractSpace::InitSpacePool(CAbstractSpacePool* Pool){
		m_StaticSpacePool = Pool;
	}

	int64 AbstractSpace::CreateTimeStamp(){
		assert(m_StaticTimer != NULL); 
		return m_StaticTimer->TimeStamp();
	};
	
	CTimeStamp*  AbstractSpace::GetTimer(){
		assert(m_StaticTimer != NULL); 
		return m_StaticTimer;

	}

	CAbstractSpacePool* AbstractSpace::GetSpacePool(){
		assert(m_StaticSpacePool);
		return m_StaticSpacePool;
	};
	

    const char*  Energy::BaseEnergyChar= "0123456";  //��������-�ַ�ת�� 

	const wchar_t*  Energy::BaseEnergyName[] = {
                     _T("TYPE_NULL"),        // = 0,   	                 
		    		 _T("TYPE_INT"),         // = 1,
					 _T("TYPE_FLOAT"),       // = 2,
					 _T("TYPE_STRING"),      // = 3,
					 _T("TYPE_PIPELINE"),    // = 4,
					 _T("TYPE_BLOB"),        // = 5
					 _T("TYPE_USER")         // = 6,		
	};


    const wchar_t* Energy::GetTypeName(uint32 type){
	    assert(type<TYPE_END);
		return BaseEnergyName[type];
	}
/*�ò�?��ʵ���ַ������ֵ�ת������֪���ܼӿ���٣��ӷ�����ȳ˷�Ӧ�ø���Ч��
  inttable[a][b]  aΪʮ���Ƶ�λ��bΪ��λ�ϵ�ֵ       
*/
	const int64 Energy::INTTABLE[21][10]={
		  {0,0,0,0,0,0,0,0,0,0},      //����
		  {0,1,2,3,4,5,6,7,8,9},
		 {-1,10,20,30,40,50,60,70,80,90},
		 {-1,100,200,300,400,500,600,700,800,900},
		 {-1,1000,2000,3000,4000,5000,6000,7000,8000,9000},
		 {-1,10000,20000,30000,40000,50000,60000,70000,80000,90000},
		 {-1,100000,200000,300000,400000,500000,600000,700000,800000,900000},
		 {-1,1000000,2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000},
		 {-1,10000000,20000000,30000000,40000000,50000000,60000000,70000000,80000000,90000000},
		 {-1,100000000,200000000,300000000,400000000,500000000,600000000,700000000,800000000,900000000},
		 {-1,1000000000ULL,2000000000ULL,3000000000ULL,4000000000ULL,5000000000ULL,6000000000ULL,7000000000ULL,8000000000ULL,9000000000ULL},
		 {-1,10000000000ULL,20000000000ULL,30000000000ULL,40000000000ULL,50000000000ULL,60000000000ULL,70000000000ULL,80000000000ULL,90000000000ULL},
		 {-1,100000000000ULL,200000000000ULL,300000000000ULL,400000000000ULL,500000000000ULL,600000000000ULL,700000000000ULL,800000000000ULL,900000000000ULL},
		 {-1,1000000000000ULL,2000000000000ULL,3000000000000ULL,4000000000000ULL,5000000000000ULL,6000000000000ULL,7000000000000ULL,8000000000000ULL,9000000000000ULL},
		 {-1,10000000000000ULL,20000000000000ULL,30000000000000ULL,40000000000000ULL,50000000000000ULL,60000000000000ULL,70000000000000ULL,80000000000000ULL,90000000000000ULL},
		 {-1,100000000000000ULL,200000000000000ULL,300000000000000ULL,400000000000000ULL,500000000000000ULL,600000000000000ULL,700000000000000ULL,800000000000000ULL,900000000000000ULL},
		 {-1,1000000000000000ULL,2000000000000000ULL,3000000000000000ULL,4000000000000000ULL,5000000000000000ULL,6000000000000000ULL,7000000000000000ULL,8000000000000000ULL,9000000000000000ULL},
		 {-1,10000000000000000ULL,20000000000000000ULL,30000000000000000ULL,40000000000000000ULL,50000000000000000ULL,60000000000000000ULL,70000000000000000ULL,80000000000000000ULL,90000000000000000ULL},
		 {-1,100000000000000000ULL,200000000000000000ULL,300000000000000000ULL,400000000000000000ULL,500000000000000000ULL,600000000000000000ULL,700000000000000000ULL,800000000000000000ULL,900000000000000000ULL},
		 {-1,1000000000000000000ULL,2000000000000000000ULL,3000000000000000000ULL,4000000000000000000ULL,5000000000000000000ULL,6000000000000000000ULL,7000000000000000000ULL,8000000000000000000ULL,9000000000000000000ULL},
		 {-1,10000000000000000000ULL,0,0,0,0,0,0,0,0}
	};
  
	const  float64 Energy::FLOATTABLE[][10] = {
		 {0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9},
		 {-2.0,0.01,0.02,0.03,0.04,0.05,0.06,0.07,0.08,0.09},
		 {-2.0,0.001,0.002,0.003,0.004,0.005,0.006,0.007,0.008,0.009},
		 {-2.0,0.0001,0.0002,0.0003,0.0004,0.0005,0.0006,0.0007,0.0008,0.0009},
		 {-2.0,0.00001,0.00002,0.00003,0.00004,0.00005,0.00006,0.00007,0.00008,0.00009},
		 {-2.0,0.000001,0.000002,0.000003,0.000004,0.000005,0.000006,0.000007,0.000008,0.000009},
          //����δ�ã�Ŀǰ��֪���Ƿ��б�ҪΪ������㱣��
		 {-2.0,0.0000001,0.0000002,0.0000003,0.0000004,0.0000005,0.0000006,0.0000007,0.0000008,0.0000009},
		 {-2.0,0.00000001,0.00000002,0.00000003,0.00000004,0.00000005,0.00000006,0.00000007,0.00000008,0.00000009},
		 {-2.0,0.000000001,0.000000002,0.000000003,0.000000004,0.000000005,0.000000006,0.000000007,0.000000008,0.000000009},
		 {-2.0,0.0000000001,0.0000000002,0.0000000003,0.0000000004,0.0000000005,0.0000000006,0.0000000007,0.0000000008,0.0000000009},
		 {-2.0,0.00000000001,0.00000000002,0.00000000003,0.00000000004,0.00000000005,0.00000000006,0.00000000007,0.00000000008,0.00000000009},
		 {-2.0,0.000000000001,0.000000000002,0.000000000003,0.000000000004,0.000000000005,0.000000000006,0.000000000007,0.000000000008,0.000000000009},
		 {-2.0,0.0000000000001,0.0000000000002,0.0000000000003,0.0000000000004,0.0000000000005,0.0000000000006,0.0000000000007,0.0000000000008,0.0000000000009},
		 {-2.0,0.00000000000001,0.00000000000002,0.00000000000003,0.00000000000004,0.00000000000005,0.00000000000006,0.00000000000007,0.00000000000008,0.00000000000009},
		 {-2.0,0.000000000000001,0.000000000000002,0.000000000000003,0.000000000000004,0.000000000000005,0.000000000000006,0.000000000000007,0.000000000000008,0.000000000000009},
		 {-2.0,0.0000000000000001,0.0000000000000002,0.0000000000000003,0.0000000000000004,0.0000000000000005,0.0000000000000006,0.0000000000000007,0.0000000000000008,0.0000000000000009},
		 {-2.0,0.00000000000000001,0.00000000000000002,0.00000000000000003,0.00000000000000004,0.00000000000000005,0.00000000000000006,0.00000000000000007,0.00000000000000008,0.00000000000000009},
		 {-2.0,0.000000000000000001,0.000000000000000002,0.000000000000000003,0.000000000000000004,0.000000000000000005,0.000000000000000006,0.000000000000000007,0.000000000000000008,0.000000000000000009},
		 {-2.0,0.0000000000000000001,0.0000000000000000002,0.0000000000000000003,0.0000000000000000004,0.0000000000000000005,0.0000000000000000006,0.0000000000000000007,0.0000000000000000008,0.0000000000000000009},
		 {-2.0,0.00000000000000000001,0.00000000000000000002,0.00000000000000000003,0.00000000000000000004,0.00000000000000000005,0.00000000000000000006,0.00000000000000000007,0.00000000000000000008,0.00000000000000000009},
	};

	void  Energy::PrintString(AnsiString& s,int32 type,uint32 datalen, const char* data)
	{
		assert(type<TYPE_END);

		char buf[20];  
		::sprintf(buf,"%u",datalen);
		int32 slen = strlen(buf);
			
		char temp[50];
		char *str = &temp[0];
		*str=(char)TYPE_TO_CHAR(type); str++;
		*str='@';  str++;
		::strcpy(str,buf);
		str+=slen;
		*str='@';  str++;
        *str=0;
     
		s+=temp;
        
		//s+=data; ����data������UTF16TO8�������������ַ����ֱ�����ַ�������ܵ�����ݲ���
		//������appendǿ����ĩβ����ָ���������ַ�
		s.append(data,datalen);

	};
	//TYPE_PIPELINE@ID@len@Name@LEN@ type@len@data1
	void Energy::PrintString(AnsiString& s,int32 type,int64 ID,tstring Name,
		uint32 datalen, const char* data)
	{
		char buf1[30], buf2[20],buf3[20];  
		
		int64toa(ID,buf1);
				
		eSTRING temp(Name);
        AnsiString AnsiName;
		temp.ToString(AnsiName); //���Name��unicode��ͨ��˷�ʽת����utf8
		
		::sprintf(buf2,"%d",AnsiName.size());
		::sprintf(buf3,"%u",datalen);
	
		s += TYPE_TO_CHAR(type);  
		s += '@';                  
		
		//ID
		s += buf1;   
		s += '@';                
        //len
		s += buf2;   
		s += '@';                  
		//name
		//s +=AnsiName;  
		s.append(AnsiName.c_str(),AnsiName.size());
		s += '@';                  
		//data len
		s +=buf3;   
		s += '@';                  
		//data
		//s+=data; ����data������UTF16TO8�������������ַ����ֱ�����ַ�������ܵ�����ݲ���
		s.append(data,datalen);
/*
		int32 n = s.size();
		s.resize(n+datalen);
		memcpy(&s[n],data,datalen);
*/	};
	
	int64 Energy::StringToInt(const char* s, uint32 len){
		int64 n=0;
		int64 k;
		const char* ch = s;
		if(*s=='-' || *s=='+'){ --len; ch++;}      //"-1243" or "+1243"
		while(len>0){
            int32 t  = (*ch) -'0';
			k = INTTABLE[len][t];
			if(k != -1)n+=k;
			++ch;
			--len;
		}
		if(*s=='-')n = -n;
		return n;
	};
    
	
	int64 Energy::RawStringToInt(const char* s, uint32 len,bool& correct){
		correct = FALSE;
		int64 n=0,k;
		const char* ch = s;
		if(*s=='-' || *s=='+'){ --len; ch++;}      //"-1243" or "+1243"
		if(len>20)return 0;
		while(len>0){
			if(!isdigit(*ch))break;
            int32 t  = (*ch) -'0';
			k = INTTABLE[len][t];
			if(k != -1)n+=k;
			++ch;
			--len;
		}
		
		if(len == 0 ){
			if(*s=='-')n = -n; 
			correct = TRUE;
			return n;
		}
		return 0;
	};
	
	//uint64
	uint64 Energy::RawStringToUInt(const char* s, uint32 len,bool& correct){
		correct = FALSE;
		uint64 n=0;
		int64 k;
		const char* ch = s;
		if(len>20)return 0;
		while(len>0){
			if(!isdigit(*ch))break;
            int32 t  = (*ch) -'0';
			k = INTTABLE[len][t];
			if(k != -1)n+=k;
			++ch;
			--len;
		}
		
		if(len == 0 ){
			correct = TRUE;
			return n;
		}
		return 0;
	};
	
	
	//ʹ��ǰ�Ѿ�ȷ��s��ʽ��ȷ�����Ҽ��С������len<=6;
	float64 Energy::StringToFloat(const char* s, uint32 len,int32 floatpoint){
		int64 IntPart = StringToInt(s, floatpoint);
		len = len-floatpoint-1;
		float64 n=0.0f,k;    
		const char* ch = s+floatpoint+1;
		for(uint32 i=0; i<len; i++){
            int32 t  = (*ch) -'0';
			k = FLOATTABLE[i][t];
			if(k >-1.0)n+=k;
			++ch;
		}
		n += IntPart;
		return n;
	};
	
	
	//��s��posλ�ÿ�ʼ����ÿһ���ַ�����Ϊ�������ַ�@������󷵻����ֳ���,0Ϊ����
	int32 Energy::FindInt(AnsiString& s,uint32 pos, char ch /*='@'*/)
	{
		int32 len =0;
		if(pos<s.size() && s[pos] == '-'){ //ע�⣺����������ǰ��'+'��
			++pos; 
			++len;
		};
		
		uint32 end = pos+20; //�������20λ��������ν�Ĳ���
		if(end<s.size())end = s.size();
		for(uint32 i=pos; i<end; ++i)
		{ 
			if(s[i] == ch){
				if(i==pos)return 0;
				else return len;
			}
			if(!isdigit(s[i]))return 0;
            ++len;
		}
		return 0;
	}
	
	//ͬ�ϣ��ҵ�����鸡��������ֳ��ȣ����Ѹ���λ�ñ�����floatpos��
	int32 Energy::FindFloat(AnsiString& s, uint32 pos, uint32& floatpos,char ch /*='@'*/)
	{
		int32 len = 0;
		if(s[pos] == '-'){ ++pos; ++len;}; //ע�⣺����������ǰ��'+'��
		
		bool floatpoint = FALSE;
		
		uint32 end = min(pos+30,s.size()); //�������20λ��������ν�Ĳ���
		for(uint32 i=pos; i<end; ++i)
		{ 
			if(isdigit(s[i])){
                ++len;
			}
			else if(s[i] == ch){
				if(i==pos)return 0;
				if(!floatpoint)floatpos = len; //û���ҵ�С������ͬ��С��������
				return len;
			}
			else if(s[i]=='.'){      //1.234
				if(!floatpoint){
					floatpos = i-pos;
					floatpoint = TRUE;
					++len;
				}
				else return 0;
			}
			else return 0;
		}
		if (floatpoint && len>1)
		{
			return len; 
		}
		return 0;
	}
	
	Energy*  Mass::ToEnergy(){
		return new eNULL;
	}; 
	
	bool Mass::FromEnergy(Energy* E){
		assert(E->EnergyType() == TYPE_NULL);
		return true;
	} 
}//namespace ABSTRACT



