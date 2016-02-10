#ifndef _THIN_DATA_TRANS_PROTOCOL_H
#define _THIN_DATA_TRANS_PROTOCOL_H

/*
Version : 0.9
Create Data: 2014/9/24
Author:  ZHB (hongbing75@gmail.com)
License: BSD

*/


/*
This data transfer protocol is primarily aimed at different hardware platforms 
to achieve hardware-independent a few basic data transfer.Mainly used for embedded
hardware interaction.

Basic principle is converting the binary data into a textual data. For example,
integer n=158 , binary transmission may be as follows: char buf[2];
 
 buf[0]=(n&0xff)>>8
 buf[1]=(n&0x00ff)

Because of hardware platforms has Big-Endian and Little-Endian difference,
if we don't know the hardware's endian mode, then data may be incorrectly handled.

In the internet of things age, when strange devices interconnected, 
this kind of thing may be more common.

This protocol directly use string "158" as n=158 to transmit, it avoids that problem.

Currently supports three types of basic data: int,double,String, may later supports for Int64

The unified transmission format for each data type: type@Len@data

Type for the data type,len for the length of the data and data is the string whose length is len

Take n=158 for example, the format is: 1@3@158

This protocol is aimed for a small amount of data transmission,to make this simple, 
it requires receiver to transmit data in fixed-length mode of each frame.

Which means if the per-frame data was char buf[100], even if only an integer 
that should transmit 100 bytes.

Also only received 100 bytes, the receiver began to interpret and use data.

If communication participants don't know each other's frame length , 
in order to negotiate the same frame length, the first time letter format 
should is:  @Len@@, which means: my frame length is Len, then waits 
for the other to return the same information to complete the consultation.

The recipient has obligation at any time to check whether has received 
similar information in @Len@@ format, and then changes the frame length 
and returns the same information to others

Test code:

char buf[100];
int pos=0;
pos=Float2Str(112.1235,buf,100);
pos=Float2Str(-112.1234,buf,100);
pos=Float2Str(0.2233,buf,100);
pos=Float2Str(-0.22343,buf,100);

int t=1024,n1;
double f=345.346,f1;
const char* s="this is test";

pos=0;
pos =TDTP_FLOAT_ToString(buf,100,pos,f);
pos =TDTP_STR_ToString(buf,100,pos,s,strlen(s));
pos =TDTP_INT_ToString(buf,100,pos,t);

pos=0;
pos = TDTP_FLOAT_FromString(buf,100,pos,f1);
char s1[100];
pos = TDTP_STR_FromString(buf,100,pos,s1,100);
string  s2 = s1;
pos = TDTP_INT_FromString(buf,100,pos,n1);



*/

#define TDTP_TYPE_INT  		1
#define TDTP_TYPE_FLOAT		2
#define TDTP_TYPE_STRING    3

#define TDTP_TYPE_INT_C		  '1'
#define TDTP_TYPE_FLOAT_C	  '2'
#define TDTP_TYPE_STRING_C    '3'

int Float2Str(double Data,char* Buf,int Bufsize);

//write the specified data to buf, return the latest pos, if the buffer is not enough then return 0
int  TDTP_INT_ToString(char* Buf, int BufSize,int WritePos,int   Data);
int  TDTP_FLOAT_ToString(char* Buf, int BufSize,int WritePos,double Data);
int  TDTP_STR_ToString(char* Buf, int BufSize,int WritePos,const char* Str,int StrLen);



//Get the frame ength from others, adjust the receiving length, return 0 that means error
int  TDTP_GetFrameSize(char* Buf,int BufSize);

//read a particular type of data from the Buf, if success that returns the next Pos, otherwise return 0
int  TDTP_INT_FromString(char* Buf,int BufSize,int ReadPos,int&  Data);
int  TDTP_FLOAT_FromString(char* Buf,int BufSize,int ReadPos,double& Data);
int  TDTP_STR_FromString(char* Buf,int BufSize,int ReadPos,char* str,int strlen);

#endif //_THIN_DATA_TRANS_PROTOCOL_H