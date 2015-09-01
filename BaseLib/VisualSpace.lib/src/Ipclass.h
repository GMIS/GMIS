// Ipclass.h: interface for the _IP_ class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IPCLASS_H__
#define _IPCLASS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
using namespace std;

class IP
{
private:
public:
	unsigned char seg1;
	unsigned char seg2;
	unsigned char seg3;
	unsigned char seg4;
	std::string Get();
	void Set(char *);
	void Set(string ip);
	bool operator == (IP &);
	unsigned int GetUint32();
	IP(IP &);
	IP(unsigned int p);
	IP(char *);
	IP(std::string);
	IP::IP(unsigned char, unsigned char, unsigned char, unsigned char);
	IP();
	virtual ~IP();
};


#endif // _IPCLASS_H__
