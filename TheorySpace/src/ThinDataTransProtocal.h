#ifndef _THIN_DATA_TRANS_PROTOCOL_H
#define _THIN_DATA_TRANS_PROTOCOL_H

/*
Version : 0.9
Create Data: 2014/9/24
Author:  ZHB (hongbing75@gmail.com)
License: BSD

*/


/*
此数据传输协议主要目的是在不同硬件平台之间实现硬件无关的少量基本数据传输，主要针对的是与
嵌入式硬件的交互。

基本原理是把二进制数据变成文本数据传输。比如，对于一个整数n=158，二进制传输可能用如下方式：
 char buf[2];
 buf[0]=(n&0xff)>>8
 buf[1]=(n&0x00ff)

由于硬件平台可能存在内存大小端问题，如果你不明确对方接收端的模式，那么数据就可能被错误的处理，
在物联网时代，由于会和陌生设备互联互通，这种事情可能会比较常见。

本协议直接把n=158按字符串“158”传输，则避免了上述问题。

目前支持三种基本数据：int,double,string，稍后可能会支持int64

每种数据传输的统一格式为：type@len@data

type为数据的类型ID，len为数据的长度，data则为长度为len的字符串

以n=158为例，传输格式为：1@3@158

本协议定位为少量信息传输，为了让问题简单化，要求接收双方以每帧固定长度的方式传输数据。
就是说假如每帧数据为char buf[100],那么即使只传输一个整数，也应该传输完100个字节，
同样对方只有接收完100个字节后，才开始解释和使用数据。


如果通信一方不知道对方的信息帧长，为了协商出一致的帧长度，首次发信的一方可以发出
格式为：@len@@的信息，则表示在说，我的帧长为len，然后等待对方返回相同此信息，表示协商完成。

而收信者有义务随时检查是否收到类似@len@@格式的信息，然后改变自己的帧长，并返回相同信息给对方


test code:


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

//向Buf写入指定的数据,返回最新Pos，如果缓冲区不够则返回0
int  TDTP_INT_ToString(char* Buf, int BufSize,int WritePos,int   Data);
int  TDTP_FLOAT_ToString(char* Buf, int BufSize,int WritePos,double Data);
int  TDTP_STR_ToString(char* Buf, int BufSize,int WritePos,const char* Str,int StrLen);


//如果首次对方传来的数据，但并不知道对方传输的帧长度，那么只要本次接收不小于30，都可以获得
//对方帧协议长度，从而调整自己的接收长度,返回0意味着错误
int  TDTP_GetFrameSize(char* Buf,int BufSize);

//从Buf中读一个特定类型数据，成功则返回下一个Pos,否则返回0
int  TDTP_INT_FromString(char* Buf,int BufSize,int ReadPos,int&  Data);
int  TDTP_FLOAT_FromString(char* Buf,int BufSize,int ReadPos,double& Data);
int  TDTP_STR_FromString(char* Buf,int BufSize,int ReadPos,char* str,int strlen);

#endif //_THIN_DATA_TRANS_PROTOCOL_H