// Ipclass.cpp: implementation of the __IP__ class.
//
//////////////////////////////////////////////////////////////////////

#include "Ipclass.h"
#include <stdio.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//The class I use for transfering IP addresses around

void IP::Set(char *ip)
{
	char ipbuf[16];
	if (strlen(ip) > 15 ||
	!strchr(strchr(strchr(ip, '.') + 1, '.') + 1, '.'))
	{
		seg1 = 0;
		seg2 = 0; 
		seg3 = 0; 
		seg4 = 0; 
	}
	else
	{
		strcpy(ipbuf, ip);
		seg1 = atoi(strtok(ipbuf, "."));
		seg2 = atoi(strtok(NULL, "."));
		seg3 = atoi(strtok(NULL, "."));
		seg4 = atoi(strtok(NULL, ""));
	}
	if (seg1 > 0xFF || seg2 > 0xFF || seg3 > 0xFF || seg4 > 0xFF)
	{
		seg1 = 0; 
		seg2 = 0; 
		seg3 = 0; 
		seg4 = 0;
	}
}
	
void IP::Set(std::string ip){
	if (ip.size() > 16) return;
	char Ip[16];
	strcpy(Ip, ip.c_str());
	Set(Ip);
}

std::string IP::Get()
{
	char buf[50];
	sprintf(buf,"%d.%d.%d.%d", seg1, seg2, seg3, seg4);
	return buf;
}

unsigned int IP::GetUint32(){
	return ((unsigned int)(((unsigned int)(seg1)<<24)+((unsigned int)(seg2)<<16)+((unsigned int)(seg3)<<8)+((unsigned int)(seg4))));
}

bool IP:: operator == (IP &ip)
{
	if (seg1 == ip.seg1 && seg2 == ip.seg2 &&
		seg3 == ip.seg3 && seg4 == ip.seg4)
		return true;
	return false;
}

IP::IP(unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4)
{
	seg1 = s1;
	seg2 = s2;
	seg3 = s3;
	seg4 = s4;
}

IP::IP(IP &ip)
{
	seg1 = ip.seg1;
	seg2 = ip.seg2;
	seg3 = ip.seg3;
	seg4 = ip.seg4;
}
	
IP::IP(unsigned int p){
	seg1 = ((p>>24) & 0xff);
	seg2 = ((p>>16) & 0xff);
	seg3 = ((p>>16) & 0xff);
	seg4 = (p & 0xff);
}

IP::IP(char *ip)
{
	Set(ip);
}

IP::IP(std::string ip)
{
	if (ip.size() > 16) return;
	char Ip[16];
	strcpy(Ip, ip.c_str());
	Set(Ip);
}

IP::IP()
{
	seg1 = 0;
	seg2 = 0;
	seg3 = 0;
	seg4 = 0;
}

IP::~IP()
{

}