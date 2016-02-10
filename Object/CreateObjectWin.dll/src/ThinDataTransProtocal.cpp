#include "ThinDataTransProtocal.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <string.h>

int  TDTP_INT_ToString(char* Buf, int BufSize,int WritePos,int   Data){
	
	if(WritePos==0){ //Note: msg is a fixed-length, first write the length value in the front
		memset(Buf,0,BufSize);
		Buf[WritePos++] = '@';
		int n = sprintf(Buf+1,"%d@",BufSize);
		WritePos+=n; 
	}

	char tmp1[80];
	int len = sprintf(tmp1,"%d",Data);
	
	char tmp[80];
	len = sprintf(tmp,"%d@%d@%s",TDTP_TYPE_INT,len,tmp1);

	if(WritePos+len>BufSize){
		return 0;
	}

	::memcpy(Buf+WritePos,tmp,len);
	WritePos+=len;
	
	return WritePos;
};

int Float2Str(double Data,char* Buf,int Bufsize)
{

	char buffer[80];

	int  p=0;

	long long a = (long long)Data;

	if(a != 0){
		char tmp[80];
		int len = sprintf(tmp,"%I64ld",a);

		memcpy(buffer,tmp,len);
	    p=len;
		buffer[p++]='.';
	}else
	{
		if (Data<0)
		{
			buffer[p++]='-';
			
		} 
		buffer[p++]='0';
		buffer[p++]='.';	
	}

	char tmp[80];
	long b= (int)((Data-(double)a)*10000.0f);
	if(b<0)b=-b;

	int len = sprintf(tmp,"%d",b);
	memcpy(buffer+p,tmp,len);
	p+=len;
	
	if (p<Bufsize)
	{
		memcpy(Buf,buffer,p);
		Buf[p]='\0';	
		return p;
	}
	return 0;
}
int  TDTP_FLOAT_ToString(char* Buf, int BufSize,int WritePos,double Data){
	if(WritePos==0){ 
		memset(Buf,0,BufSize);
		Buf[WritePos++] = '@';
		int n = sprintf(Buf+1,"%d@",BufSize);
		WritePos+=n; 
	}

	char tmp1[80];
	//int len = sprintf(tmp1,"%.6f",Data); Embedded is not supported
	int len = Float2Str(Data,tmp1,80);

	char tmp[80];
	len = sprintf(tmp,"%d@%d@%s",TDTP_TYPE_FLOAT,len,tmp1);

	if(WritePos+len>BufSize){
		return 0;
	}

	::memcpy(Buf+WritePos,tmp,len);
	WritePos+=len;

	return WritePos;
};

int  TDTP_STR_ToString(char* Buf, int BufSize,int WritePos,const char* Str,int StrLen){
	if(WritePos==0){ 
		::memset(Buf,0,BufSize);
		Buf[WritePos++] = '@';
		int n = sprintf(Buf+1,"%d@",BufSize);
		WritePos+=n; 
	}

	if(WritePos+2+StrLen>BufSize){
		return 0;
	}
    Buf[WritePos++]='3'; //string type id
	Buf[WritePos++]='@';
	
	char tmp[80];
	int len = sprintf(tmp,"%d@",StrLen);
	
	memcpy(Buf+WritePos,tmp,len);
	WritePos+=len;

	memcpy(Buf+WritePos,Str,StrLen);
	WritePos+=StrLen;

	return WritePos;
};


int  TDTP_GetFrameSize(char* Buf,int BufSize){
	assert(Buf[0]=='@');
	
	int i=0;
	Buf++;
	while (i<BufSize)
	{
		if (Buf[i]=='@')
		{
			Buf[i]='\0';
			int size = atoi(Buf);
			Buf[i]='@';
			return size;
		}
		i++;
	}
	return 0;
}

int  TDTP_INT_FromString(char* Buf,int BufSize,int ReadPos,int&  Data){
	assert(ReadPos<BufSize);

	if (ReadPos==0) //Ignore frame header
	{
		int i=1;
		while (i<BufSize)
		{
			if (Buf[i]=='@')
			{
				ReadPos=i+1;
				break;
			}
			i++;
		}
	}

	int len=0;
	
	//First compare type
	int i=ReadPos+1;
	if (Buf[ReadPos] != TDTP_TYPE_INT_C || Buf[i] !='@')
	{
		return false;
	}

	
	//Gets the length of the data
	ReadPos = i+1;
	i = ReadPos;
	while (i<BufSize)
	{
		if (Buf[i] == '@')
		{
			Buf[i]='\0';
			len = atoi(Buf+ReadPos);
			Buf[i]='@';
			if(len == 0){
				return 0;
			}
			break;
		}
		i++;
	}

	//Gets the actual integer value
	ReadPos = i+1;
	len = ReadPos+len;
	if (len > BufSize){
		return 0;
    }
	char ch = Buf[len];
	Buf[len]='\0';
	Data = atoi(Buf+ReadPos);
	Buf[len]=ch;

	return len;
};
int  TDTP_FLOAT_FromString(char* Buf,int BufSize,int ReadPos,double& Data){
	assert(ReadPos<BufSize);

	if (ReadPos==0) //Ignore frame header
	{
		int i=1;
		while (i<BufSize)
		{
			if (Buf[i]=='@')
			{
				ReadPos=i+1;
				break;
			}
			i++;
		}
	}

	int len  =0;

	//First compare type
	int i=ReadPos+1;

	if (Buf[ReadPos] != TDTP_TYPE_FLOAT_C || Buf[i] !='@')
	{
		return 0;
	}

	//Gets the length of the data
	ReadPos = i+1;
	i = ReadPos;
	while (i<BufSize)
	{
		if (Buf[i] == '@')
		{
			Buf[i]='\0';
			len = atoi(Buf+ReadPos);
			Buf[i]='@';
			if(len == 0){
				return 0;
			}
			break;
		}
		i++;
	}

	//Gets the actual float value
	ReadPos = i+1;
	len = ReadPos+len;
	if (len > BufSize){
		return 0;
	}
	char ch = Buf[len];
	Buf[len]='\0';
	Data = atof(Buf+ReadPos);
	Buf[len]=ch;
	
	return len;
};

int  TDTP_STR_FromString(char* Buf,int BufSize,int ReadPos,char* str,int strlen){
	assert(ReadPos<BufSize);
	if (ReadPos==0) 
	{
		int i=1;
		while (i<BufSize)
		{
			if (Buf[i]=='@')
			{
				ReadPos=i+1;
				break;
			}
			i++;
		}
	}
	int len  =0;

	//First compare type
	int i=ReadPos+1;

	if (Buf[ReadPos] != TDTP_TYPE_STRING_C || Buf[i] !='@')
	{
		return 0;
	}


	//Gets the length of the data
	ReadPos = i+1;
	i = ReadPos;
	while (i<BufSize)
	{
		if (Buf[i] == '@')
		{
			Buf[i]='\0';
			len = atoi(Buf+ReadPos);
			Buf[i]='@';
			if(len == 0){
				return 0;
			}
			break;
		}
		i++;
	}

	if(len>strlen-1){
		return 0;	
	};

	ReadPos = i+1;
	memcpy(str,Buf+ReadPos,len);
	str[len+1] = '\0';

	return ReadPos+len;
};