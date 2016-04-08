// TexasPoker.h: interface for the CTexasPokerEvaluate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXASPOKEREVALUATE__A3CEF3B9_3225_4573_93F4_845B17915A9E__INCLUDED_)
#define AFX_TEXASPOKEREVALUATE__A3CEF3B9_3225_4573_93F4_845B17915A9E__INCLUDED_


#include "TheorySpace.h"
#include <map>
#include "UserTimeStamp.h"
#include "UserMutex.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "PokerDatabase.h"

using namespace std;

tstring GetSuitStr(int Suit);
tstring GetFaceStr(int Face);

tstring GetCardStr(int Card); // GetSuitStr(int Suit)+GetFaceStr(int Face);


class CTexasPokerEvaluate
{
//public:
//	CPokerDatabase  m_PokerDB;
public:
	CTexasPokerEvaluate();
	virtual ~CTexasPokerEvaluate();

    int32  Str2Num(tstring& str);

	float32 Test();

	float32 StraightChance(int32 MyCard[],int32 Hand[]);
	float32 FlushFChance(int32 MyCard[],int32 SuitMaxRepeat,int32 Suit);

	float32 CheckHand(vector<int32>& MyPorker,vector<int32>& OtherPorker,tstring& Memo);

	float32 GetPocketPairFactor(int32 HiCard, BOOL IsFlush); //起手牌对子
	float32 GetPocketKickerFactor(int32 HiCard, int32 LowCard,BOOL IsFlush); //起手散牌
    float32 GetStraightFactor(int32 MyPoker[],int32 Hand[],int32 StraightStartPos,BOOL IsFlush,int32 CardNum);
	float32 GetFlushFactor(int32 MyPoker[],int32 FlushSuit,int32 HighPos,int32 Hand[],int32 CardNum);
    float32 GetFourKindFactor(int32 MyPoker[],int32 KindPos, int32 Hand[],int32 CardNum);
    float32 GetFullHouseFactor(int32 MyPoker[], int32 ThreeCard, int32 TwoCard,int32 CardNum);
    float32 GetThreeKindFactor(int32 MyPoker[], int32 ThreeCard, int32 OtherCard1,int32 OtherCard2,int32 CardNum);
    float32 GetTwoPairFactor(int32 MyPoker[], int32 FirstPair,int32 SecondPair,int32 OtherCard,int32 CardNum);
    float32 GetOnePairFactor(int32 MyPoker[], int32 Pair,int32 OtherCard1,int32 OtherCard2,int32 OtherCard3,int32 CardNum);
    float32 GetKickerFactor(int32 MyPoker[], int32 Card1,int32 Card2,int32 Card3,int32 Card4,int32 Card5,int32 CardNum);
	int32   FindStraight(int32 Card[]);
};

#endif // !defined(AFX_TEXASPOKEREVALUATE__A3CEF3B9_3225_4573_93F4_845B17915A9E__INCLUDED_)