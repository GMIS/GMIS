#ifndef _THIN_DATA_TRANS_PROTOCOL_H
#define _THIN_DATA_TRANS_PROTOCOL_H

/*
Version : 0.9
Create Data: 2014/9/24
Author:  ZHB (hongbing75@gmail.com)
License: BSD

*/


/*
�����ݴ���Э����ҪĿ�����ڲ�ͬӲ��ƽ̨֮��ʵ��Ӳ���޹ص������������ݴ��䣬��Ҫ��Ե�����
Ƕ��ʽӲ���Ľ�����

����ԭ���ǰѶ��������ݱ���ı����ݴ��䡣���磬����һ������n=158�������ƴ�����������·�ʽ��
 char buf[2];
 buf[0]=(n&0xff)>>8
 buf[1]=(n&0x00ff)

����Ӳ��ƽ̨���ܴ����ڴ��С�����⣬����㲻��ȷ�Է����ն˵�ģʽ����ô���ݾͿ��ܱ�����Ĵ���
��������ʱ�������ڻ��İ���豸������ͨ������������ܻ�Ƚϳ�����

��Э��ֱ�Ӱ�n=158���ַ�����158�����䣬��������������⡣

Ŀǰ֧�����ֻ������ݣ�int,double,string���Ժ���ܻ�֧��int64

ÿ�����ݴ����ͳһ��ʽΪ��type@len@data

typeΪ���ݵ�����ID��lenΪ���ݵĳ��ȣ�data��Ϊ����Ϊlen���ַ���

��n=158Ϊ���������ʽΪ��1@3@158

��Э�鶨λΪ������Ϣ���䣬Ϊ��������򵥻���Ҫ�����˫����ÿ֡�̶����ȵķ�ʽ�������ݡ�
����˵����ÿ֡����Ϊchar buf[100],��ô��ʹֻ����һ��������ҲӦ�ô�����100���ֽڣ�
ͬ���Է�ֻ�н�����100���ֽں󣬲ſ�ʼ���ͺ�ʹ�����ݡ�


���ͨ��һ����֪���Է�����Ϣ֡����Ϊ��Э�̳�һ�µ�֡���ȣ��״η��ŵ�һ�����Է���
��ʽΪ��@len@@����Ϣ�����ʾ��˵���ҵ�֡��Ϊlen��Ȼ��ȴ��Է�������ͬ����Ϣ����ʾЭ����ɡ�

����������������ʱ����Ƿ��յ�����@len@@��ʽ����Ϣ��Ȼ��ı��Լ���֡������������ͬ��Ϣ���Է�


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

//��Bufд��ָ��������,��������Pos����������������򷵻�0
int  TDTP_INT_ToString(char* Buf, int BufSize,int WritePos,int   Data);
int  TDTP_FLOAT_ToString(char* Buf, int BufSize,int WritePos,double Data);
int  TDTP_STR_ToString(char* Buf, int BufSize,int WritePos,const char* Str,int StrLen);


//����״ζԷ����������ݣ�������֪���Է������֡���ȣ���ôֻҪ���ν��ղ�С��30�������Ի��
//�Է�֡Э�鳤�ȣ��Ӷ������Լ��Ľ��ճ���,����0��ζ�Ŵ���
int  TDTP_GetFrameSize(char* Buf,int BufSize);

//��Buf�ж�һ���ض��������ݣ��ɹ��򷵻���һ��Pos,���򷵻�0
int  TDTP_INT_FromString(char* Buf,int BufSize,int ReadPos,int&  Data);
int  TDTP_FLOAT_FromString(char* Buf,int BufSize,int ReadPos,double& Data);
int  TDTP_STR_FromString(char* Buf,int BufSize,int ReadPos,char* str,int strlen);

#endif //_THIN_DATA_TRANS_PROTOCOL_H