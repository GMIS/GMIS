// TexasPoker.cpp: implementation of the CTexasPokerEvaluate class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4786)

#include "TexasPokerEvaluate.h"
//#include "FORMAT.H"
#include "crc32.h"
#include <iostream>
#include <fstream>
#include<string>


tstring GetSuitStr(int Suit)
{
	switch(Suit)
	{
	case 0:		
		return _T("����");//����
		break;
	case 1:		
		return _T("÷��"); //÷��
		break;
	case 2:		
		return _T("��Ƭ"); //��Ƭ
		break;
	case 3:		
		return _T("����"); //����
		break;
	default:	return _T("Error");
	}
};

tstring GetFaceStr(int Face)
{
	switch(Face)
	{
	case 2:		return _T("2");
		break;
	case 3:		return _T("3");
		break;
	case 4:		return _T("4");
		break;
	case 5:		return _T("5");
		break;
	case 6:		return _T("6");
		break;
	case 7:		return _T("7");
		break;
	case 8:		return _T("8");
		break;
	case 9:		return _T("9");
		break;
	case 10:	return _T("10");
		break;
	case 11:	return _T("J");
		break;
	case 12:	return _T("Q");
		break;
	case 13:	return _T("K");
		break;
	case 14:	return _T("A");
		break;
	default:
		return _T("Error");
	}
}

tstring GetCardStr(int Card)
{
	tstring s = GetSuitStr(Card/100) + GetFaceStr(Card%100);
	return s;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTexasPokerEvaluate::CTexasPokerEvaluate()
//:m_PokerDB("Poker.DB")
{

}

CTexasPokerEvaluate::~CTexasPokerEvaluate()
{

}


int32  CTexasPokerEvaluate::Str2Num(tstring& str){
	tstring numStr;
	for(int i=0; i<str.size(); i++)
	{
		TCHAR ch = str[i];
		if (_istdigit(ch))
		{
			numStr+=ch;
		}else if(ch ==_T(',')){
			continue;
		}else if(!_istspace(ch) && _istprint(ch))
		{
			return 0;
		}
	}

	int32  n =_ttoi(numStr.c_str());
	return n;
}


//��������������5��������Ϊ˳�ӣ�����з�����Сλ�ã�����0
int32 CTexasPokerEvaluate::FindStraight(int32 Card[]){
	int i=14;
	int n=0;
	for (i; i>1; i--)
	{
		int32 v = Card[i];
		if (v)
		{
			n++;
			if (n==5)
			{
				return i;
			}
		}else
		{
			n=0;
		}
	}
	return 0;
}


float32 CTexasPokerEvaluate::StraightChance(int32 MyCard[],int32 Hand[]){

	int32 MaxStraight = 0;
	int32 n=0, p = 0;
    for(int i=0; i<15; i++)
	{
		int32 v = Hand[i]%100;
     
        if (v != 0)
        {
			n ++;
        } 
        else
        {
			if(MaxStraight<n){
				MaxStraight = n;
				p = i; //���ټ�¼���������λ��
			}
			n =0;
        }	
	}

	int32  Card1 = MyCard[0]%100;
    int32  Card2 = MyCard[1]%100;


	if (MaxStraight>2) //50%�Ļ���
	{
		for (int i=0; i<MaxStraight; i++)
		{
			int v = Hand[--p]%100;
			if (v == Card1 || v == Card2)
			{
				 if (MaxStraight==3) //�����Ʋ�������
				 {
					 return 0.5f;
				 }else{
					 return 1.0f;
				 }
			}
		}
		if (MaxStraight==4)//�����Ʋ�����
		{
			return -2.0f;
		}
	}
	return 0.0f;
}

float32 CTexasPokerEvaluate::FlushFChance(int32 MyCard[],int32 SuitMaxRepeat,int32 Suit){
	int32  Card1 = MyCard[0]%100;
    int32  Card2 = MyCard[1]%100;
	int32  Suit1 = MyCard[0]/100;
	int32  Suit2 = MyCard[1]/100;

	if (SuitMaxRepeat <3)
	{
		return 0.0f;
	}

    if (Suit1 != Suit && Suit2 != Suit) //���������ǻ�
    {
		if(SuitMaxRepeat==4){
			return -2.0f;
		}
		return 0.0f;
    }

	if(Suit == Suit1 && Suit == Suit2){ //��������
		if (SuitMaxRepeat == 4)
		{
			if (Card1==14)
			{
				return 2.5f;
			}
			return 1.5f;
		}else {
			return 0.5f;
		}
	}
    //ֻ������һ����
    if (Card1==14)
    {
		return 1.0f;
    }

	if (Card1>12 || Card2>12)
	{
		return 0.5f;
	}
	return 0.0f;
}



float32 CTexasPokerEvaluate::GetPocketPairFactor(int32 HiCard, BOOL IsFlush) //���� 
{
	int32 f=6.0f;
	HiCard %= 100;
	
	switch(HiCard){
	case 14:	f = 10.0f; //_T("A");
		break;
	case 13:	f = 10.0f; //_T("K");
		break;
	case 12:	f = 10.0f;  //_T("Q");
		break;
	case 11:	f = 9.0f;  //_T("J");
		break;
	case 10:	f = 8.0f;  //_T("10");
		break;
	case 9:		f = 7.0f;  //_T("9");
		break;
	default:	
		break;
	}
	
	if (IsFlush)
	{
		f+=0.5;
	}
	return f;
};

float32 CTexasPokerEvaluate::GetPocketKickerFactor(int32 HiCard, int32 LowCard,BOOL IsFlush) 
{
	float f = 5.0f;
	
	HiCard %= 100;
	LowCard %= 100;
	
	if (HiCard==14)     //A      
	{
		switch(LowCard){ 
		case 13:	f = 10.0f; //_T("K");
			break;
		case 12:	f = 10.0f;  //_T("Q");
			break;
		case 11:	f = 9.0f;  //_T("J");
			break;
		case 10:	f = 7.0f;  //_T("10");
			break;
		case 9:		f = 6.0f;  //_T("9");
			break;
		default:
			f +=0.5f;
			break;
		}
		return f;
	}
	else if (HiCard==13)  //k  
	{
		switch(LowCard){   
		case 12:	f = 5.8f;  //_T("Q");
			break;
		case 11:	f = 5.7f;  //_T("J");
			break;
		case 10:	f = 5.6f;  //_T("10");
			break;
		case 9:		f = 5.5f;  //_T("9");
			break;
		default:
			f +=0.1f;
			break;
		}	
		return f;
	}else if (HiCard==12)  //q
	{
		switch(LowCard){
		case 11:	f = 5.7f;  //_T("J");
			break;
		case 10:	f = 5.6f;  //_T("10");
			break;
		case 9:	    f = 5.5f;  //_T("9");
			break;
		default:
			f +=0.1f;
			break;
		}
		return f;
	}else if (HiCard==11)  //J
	{
		switch(LowCard){
		case 10:	f = 5.6f;  //_T("10");
			break;
		case 9:		f = 5.5f;  //_T("9");
			break;
		default:
			f +=0.2f;
			break;
		}
		return f;
	}
	
	int d=HiCard-LowCard;
	if (IsFlush ) //ͬ��
    {
		if (LowCard>10)
		{
			f += 0.5f;
		}else{
			f+=0.1f;
		}
		if (d==1)
		{
			f+=0.1f;
		}
	}
	else if (d>4)
	{
		f-=0.1f;

	}else if (d==1)
	{
		f+=0.2f;
	}

	if(HiCard<9)
	{
		f -=0.1f;
	}

	return f;
};

float32 CTexasPokerEvaluate::GetStraightFactor(int32 MyPoker[],int32 Hand[],int32 StraightStartPos,BOOL IsFlush,int32 CardNum){
	float32 f=5.0f;
	
	int32 Card; 
	for (int i=0; i<5; i++)
	{
		Card = Hand[i+StraightStartPos];
		
		if (Card == MyPoker[1]) //���Ƶ�С�Ʋ������˳��
		{
			if (IsFlush)  //ͬ��˳
			{
				return 10.f;
			}else if (i==0) //С�Ʋ���˳�ӵĵײ�
			{
				f = 9.0f;
			}else if (i<4) //С�Ʋ���˳�ӵ��в�
			{
				f = 10.0f;
			}else{
				f = 10.0f;
			}
		}else if (Card == MyPoker[0]) //�����еĴ��Ʋ������˳��
		{
			if (IsFlush)  //ͬ��˳
			{
				return 10.f;
			}else if (i==0) //С�Ʋ���˳�ӵĵײ�
			{
				f = 9.5f;
			}else if (i<4) //С�Ʋ���˳�ӵ��в�
			{
				f = 10.0f;
			}else{
				f = 10.5f;
			}			
		}
	}
	
	if (f > 5.0f)
	{
		Card = Card%100;
		
		if (Card == 14) //A,����˳��
		{
			f = 10.0f;
		}else if (CardNum==5) //��˳
		{
			f += 3.0f;
		}
	}
	return f;	
}

float32 CTexasPokerEvaluate::GetFlushFactor(int32 MyPoker[],int32 FlushSuit,int32 HighPos,int32 Hand[],int32 CardNum){
	int32  Suit1 = MyPoker[0]/100;
	int32  Card1 = MyPoker[0]%100;
	int32  Suit2 = MyPoker[1]/100;
    int32  Card2 = MyPoker[1]%100;
	
	float f=5.0f;
	
	//ͬ���ɲ������Լ�������ɣ�ֻ�ܴ���
	if (Suit1 != FlushSuit && Suit2 !=FlushSuit)
	{
		return f;
	}
	
	int n=0;
    for(int i=HighPos; i>1; i--){
		int32 v = Hand[i];
		if(v==0)continue;
		
		int32 Suit = v/100;
		int32 Card = v%100;
		
		if (Suit != FlushSuit)
		{
			continue;
		}
		
		n++;
		if (Card == Card1 || Card == Card2)
		{
			if (Card == 14)
			{
				f = 10.0f;
			} 
			else if(Card ==13)
			{
				f = 9.0f;
				
				if(n==2){ //��Ȼ������󣬵�ǰ���Ѿ���һ��A�ˣ��Լ���K��Ȼ�����  
					f=10.0f;
				}
			}else if (Card == 12)
			{
				f = 9.0f;
				if (n==3)
				{
					f = 10.0f;
				}
			}else if (Card == 11)
			{
				f = 9.0f;
				if (n==4)
				{
					f = 10.0f;
				}
			}else if (Card == 10)
			{
				if (n==5)
				{
					f = 10.0f;
				}
			}
		}
		
		if (n==5)
		{
			break;
		}
	}
	
	//�Լ������ƶ�����
	if (Suit1 == FlushSuit && Suit2 ==FlushSuit)
	{
		f +=3.0f;
	}
	return f;
}

float32 CTexasPokerEvaluate::GetFourKindFactor(int32 MyPoker[],int32 KindPos, int32 Hand[],int32 CardNum){
	int32  Card1 = MyPoker[0]%100;
    int32  Card2 = MyPoker[1]%100;
	
	int32  Card = Hand[KindPos]%100;
	
    float32 f = 5.0f;
	if (Card1 == Card || Card2 == Card)
	{
		f = 10.0f;
	}
	return f;
}

float32 CTexasPokerEvaluate::GetFullHouseFactor(int32 MyPoker[], int32 ThreeCard, int32 TwoCard,int32 CardNum){
	int32  Card1 = MyPoker[0]%100;
    int32  Card2 = MyPoker[1]%100;
	
	ThreeCard = ThreeCard%100;
	TwoCard = TwoCard%100;
	
	float32 f=5.0f;
	if (ThreeCard == Card1 || ThreeCard == Card2)
	{
		f = 10.0f;
		if (Card1==Card2)
		{
			f = 10.0f;
		}else if (ThreeCard>10)
		{
			f += 2.0f;
		}
	}else if (TwoCard == Card1 || TwoCard == Card2)
	{
		f = 6.0f;
		if (Card1 == Card2)
		{
			f = 8.0f;
            if (TwoCard > 12)
            {
				f=10.0f;
            }
		}
	}
	return f;
}

float32 CTexasPokerEvaluate::GetThreeKindFactor(int32 MyPoker[], int32 ThreeCard, int32 OtherCard1,int32 OtherCard2,int32 CardNum){
	int32  Card1 = MyPoker[0]%100;
    int32  Card2 = MyPoker[1]%100;
	
	ThreeCard  = ThreeCard%100;
	OtherCard1 = OtherCard1%100;
	OtherCard2 = OtherCard2%100;
	
    float32 f = 5.0f;
	
	if (Card1 == ThreeCard || Card2 == ThreeCard)
	{
		f = 7.0f;
		if (Card1 == Card2)
		{
			f = 9.0f;
			
		}else if (ThreeCard>12)
		{
			f = 9.0f;
		}
		
		if (CardNum == 5) //����
		{
			f = 10.0f;
		}
	}
	return f;
}

float32 CTexasPokerEvaluate::GetTwoPairFactor(int32 MyPoker[], int32 FirstPair,int32 SecondPair,int32 OtherCard,int32 CardNum){
	int32  Card1 = MyPoker[0]%100;
    int32  Card2 = MyPoker[1]%100;

	FirstPair  %=100;
	SecondPair %=100;
	
	float f = 5.0f;

	if (Card1 == FirstPair && Card2 == SecondPair)
	{
		f = 10.0f;
	}else if (Card1 == Card2){
		if(Card1 == FirstPair)
		{
			f  = 10.0f;
		}
		else if (Card1 == SecondPair)
		{
			f = 6.0f;
		}
	}
	else if (Card1 == FirstPair )
	{
		
		if (FirstPair ==14)
		{
			f = 10.0f;
		}else if (FirstPair == 13)
		{
			f = 10.0f;
		}else if (FirstPair == 12)
		{
			f = 10.0f;
		}else if (FirstPair == 11)
		{
			f = 10.0f;
		}else{
			f += 5.8f;
		}

		if(Card2>10){
			f += 0.5f;
		}
				
		if (Card2-Card1<3)
		{
			f+= 0.5f;
		};
		if (CardNum == 5) //����
		{
			f+=3.0f;
		}		
				
	}else if (Card2 == FirstPair)
	{
		if (FirstPair == 13)
		{
			f = 10.0f;
		}else if (FirstPair == 12)
		{
			f = 10.0f;
		}else if (FirstPair == 11)
		{
			f = 10.0f;
		}else if (FirstPair >8 && Card1 == 14)
		{
			f = 10.0f;					
		}
		else {
			f = 5.8f;
		}
		
		if (Card2-Card1<3)
		{
			f+= 0.5f;
		};
		if (CardNum == 5) //����
		{
			f+=3.0f;
		}	
	}
	else if (Card1 == SecondPair || Card2 == SecondPair )
	{
		f += 0.8f;
	}
	return f;
}

float32 CTexasPokerEvaluate::GetOnePairFactor(int32 MyPoker[], int32 Pair,int32 OtherCard1,int32 OtherCard2,int32 OtherCard3,int32 CardNum){
	int32  Card1 = MyPoker[0]%100;
    int32  Card2 = MyPoker[1]%100;


	Pair %=100;
	OtherCard1 %=100;
    OtherCard2 %=100;
	OtherCard3 %=100;
	
	float f = 5.0f;
	
    if (Card1 == Pair || Card2 == Pair)
	{
		if (Pair < OtherCard1 && (OtherCard1 != Card1 || OtherCard1 != Card2))  //���Ӳ���������,���������Ʋ���������һ��
		{
		    if (Pair>11)
			{
				f = 6.8f;
			}else if (Pair>9)
			{
				f = 6.0f;
			}
			else{
				f = 5.5f;
			}
			return f;
		}

		if (Pair > 11)
		{
			f = 10.0f;
		}else if(Pair >9){
			f = 7.5f;
		}else {
			f = 6.0f;
		}
	

		if (CardNum == 5) //����
		{
			f+=2.5f;
		}
	
		int32 SecondCard = Card1 == Pair? Card2 : Card1;
		if (SecondCard ==14)
		{
			f += 1.0f;
		}else if (SecondCard == 13)
		{
			f += 1.0f;
		}else if (SecondCard >10)
		{
			f += 0.5f;
            //���ֲ���
		}
		else if (SecondCard > 5)
		{
			f -= 1; 
		}else
		{
			f -= 2.0f;
		}		
	}
	return f;	
}

float32 CTexasPokerEvaluate::GetKickerFactor(int32 MyPoker[], int32 Card1,int32 Card2,int32 Card3,int32 Card4,int32 Card5,int32 CardNum){
	int32  C1 = MyPoker[0]%100;
    int32  C2 = MyPoker[1]%100;
	int32  Suit1 = MyPoker[0]/100;
	int32  Suit2 = MyPoker[1]/100;
	
	Card1 %=100;

	float f = 4.9f;
	
	if (C1 == Card1 || C2 == Card1 )
	{
		if (Card1==14)
		{
			if (C2>10)
			{
				f += 0.8f;
			}else if (C2>8)
			{
				f += 0.4f;	
			}
		}else if (Card1==13)
		{
			if (C2>10)
			{
				f += 0.5f;
			}else if (C2>9)
			{
				f += 0.2f;	
			}
		}
	}

	if (C1-C2>4)
	{
		f -=0.1f;
	}
	if (C1-C2==1)
	{
		f += 0.2f;
	}

	if (CardNum==6)
	{
		f -= 0.2f;
	}
	if(CardNum==7){
		f -= 0.4;
	}

	return f;
}

float32 CTexasPokerEvaluate::CheckHand(vector<int32>& MyPorker, vector<int32>& OtherPorker,tstring& Memo){
	
	int32 MyCard[2] ={0};   //�Լ����ϵ���
	int32 Hand[15]  ={0};   //����������
	int32 Card[15]  ={0};   //��¼ÿ���Ƶ��ظ�����
	int32 Suit[4]   ={0};   //��¼ÿ�ֻ�ɫ���ظ�����

	//��ʼ��
	int i;
	for (i=0; i<MyPorker.size(); i++)
	{
		int32 v = MyPorker[i];
		int32 SuitValue = v/100; //��ɫ
		int32 FaceValue = v%100; //��ֵ
		Suit[SuitValue]++;
		Hand[FaceValue]=v;
		Card[FaceValue]++;
		MyCard[i]=v;
	};

	//����,������ǰ
	if(MyCard[0]%100 < MyCard[1]%100)
	{
		int32 temp = MyCard[0];
		MyCard[0] = MyCard[1];
		MyCard[1] = temp;
	};

	for (i=0; i<OtherPorker.size();i++)
	{
		int32 v = OtherPorker[i];
		int32 SuitValue = v/100; //��ɫ
		int32 FaceValue = v%100; //��ֵ
		Suit[SuitValue]++;
		Hand[FaceValue]=v;
		Card[FaceValue]++;
	};

	int32 CardNum = 0;       //����
	int32 CardMaxRepeat = 0; //����ظ�������

	for(i=2; i<15; i++)
	{
		int32 v = Card[i];
		if (v>0)
		{
			CardNum += v;
			CardMaxRepeat = max(v,CardMaxRepeat);
		}
	}

	assert(CardNum == MyPorker.size()+OtherPorker.size());

	int32 SuitNum = 0;       //�ܵĻ�ɫ�� 
	int32 SuitMaxRepeat = 0; //����ظ��Ļ�ɫ��
	int32 SuitMaxCard = 0;   //����ظ��Ļ�ɫ
	for (i=0; i<4; i++)
	{
		int v = Suit[i];
		if (v>0)
		{
			SuitNum ++;
			if(SuitMaxRepeat < v){
				SuitMaxRepeat = v;
				SuitMaxCard = i;
			}
		}
	}
	assert(SuitNum != 0);
	assert(SuitMaxRepeat<=5);

	float f =5.0f;

	if (CardNum==2) //ֻ�е���
	{
		assert(OtherPorker.size()==0);

		if (SuitNum==1) //ͬ��
		{
			//assert(CardMaxRepeat != 2); //һ��,������
			if(CardMaxRepeat == 2){
				Memo =_T("�Ƽ���������ͬ������һ��");
				return 0.0f; //�Ƽ�����
			}
			//����ɢ��            
			int32 HighestCard = MyCard[0];
			int32 SecondCard  = MyCard[1];             
			f = GetPocketKickerFactor(HighestCard,SecondCard,TRUE); 

			tstring Card1Str=GetCardStr(HighestCard);
			tstring Card2Str=GetCardStr(SecondCard);
			Memo = tformat(_T("���� f=%.1f ͬ��=%s %s "),f,Card1Str.c_str(),Card2Str.c_str());
		}else{
			//assert(SuitNum==2); //��ͬ��ɫ
	
			if (CardMaxRepeat == 2) //һ��
			{
				int32 HighestCard = MyCard[0];
				int32 SecondCard  = MyCard[1]; 

				f = GetPocketPairFactor(HighestCard,FALSE);

				tstring Card1Str= GetCardStr(HighestCard);
				tstring Card2Str= GetCardStr(HighestCard);
				Memo = tformat(_T("���� f=%.1f ����=%s %s"),f,Card1Str.c_str(),Card2Str.c_str());

				return f;
			}else{ //����ɢ��
				int32 HighestCard = MyCard[0];
				int32 SecondCard  = MyCard[1];
				f =  GetPocketKickerFactor(HighestCard,SecondCard,FALSE);

				tstring Card1Str= GetCardStr(HighestCard);
				tstring Card2Str= GetCardStr(SecondCard);
				Memo = tformat(_T("���� f=%.1f ɢ��=%s %s"),f,Card1Str.c_str(),Card2Str.c_str());

			}            
		}
	}else{
		assert(CardNum>=5);
        if (CardNum == 5)
        {
			Memo=_T("���� ");
        }else if (CardNum == 6)
        {
			Memo =_T("�ڶ��� ");
        }else {
			Memo =_T("������ ");
		};

		//����
		if (CardMaxRepeat == 4)
		{
			int p=1;
			while (Card[++p]<4);
			f = GetFourKindFactor(MyCard,p,Hand,CardNum);

			tstring CardStr= GetCardStr(Hand[p]);
			Memo += tformat(_T(" f=%.1f ����=%s %s %s %s"),f,CardStr.c_str(),CardStr.c_str(),CardStr.c_str(),CardStr.c_str());
			return f;
		}
		
		//��«
		if (CardMaxRepeat == 3)
		{
			int p=1;
			while(Card[++p]!=3);
			int32 ThreeCard = Hand[p];
			assert(ThreeCard!=0);

			p=1;
			while (++p<15 && Card[p]!=2);

			if(p!=15){  //�õ���«
				int32 TwoCard = Hand[p];
				f = GetFullHouseFactor(MyCard,ThreeCard,TwoCard,CardNum);

				tstring Card1Str=GetCardStr(ThreeCard);
				tstring Card2Str=GetCardStr(TwoCard);
				Memo += tformat(_T(" f=%.1f ��«=%s %s %s %s %s"),f,Card1Str.c_str(),Card1Str.c_str(),Card1Str.c_str(),Card2Str.c_str(),Card2Str.c_str());
				return f;
			}
		}
		
		//ͬ��˳
		if (SuitMaxRepeat > 4)//ͬ��
		{
			int32 CardSuit=0;       //�ҵ���ͬ���Ļ�ɫ
			int32 i=0;
			for (i; i<4;i++)
			{
				int32 v=Suit[i];
				if (v>4)
				{
					CardSuit = i;
					break;
				}
			};

			assert(CardSuit!=0);
			
			if (CardNum-CardMaxRepeat>3)//��˳�Ŀ���
			{		
				int p = FindStraight(Card);
				if (p) //��˳��
				{
					BOOL bFind=TRUE;
					for(int j=p; j<p+5; j++){ //���˳�ӵĻ�ɫ�Ƿ���ͬ����ͬ
						if (Hand[j]/100 != CardSuit)
						{
							bFind = FALSE;
							break;
						}
					}

					if(bFind){					
						f = GetStraightFactor(MyCard,Hand,p,TRUE,CardNum);					
						Memo += tformat(_T(" f=%.1f ͬ��˳="),f);

						for (int k=0; k<5; k++)
						{
							tstring CardStr=GetCardStr(Hand[p+k]);
							Memo += CardStr;
						}
						
						return f;
					}
				}
			}
		
			//������ͬ��
			
			//�ҵ���ͬ������Ƶ�λ��				
			for (i=14; i>1; i--)
			{
				int32 v = Hand[i];
				if (v)
				{
					v = v/100;
					if (v==CardSuit)
					{
						break;
					}
				}
			}		
			f = GetFlushFactor(MyCard,CardSuit,i,Hand,CardNum);
				
			Memo += _T(" f=%.1f ͬ��=",f);
		
			for (int k=0; k<5; k++)
			{
				tstring CardStr=GetCardStr(Hand[i+k]);
				Memo += CardStr;
			}
			
			return f;			
		}
		
		if (CardNum-CardMaxRepeat>3)  //����˳
		{	
			int p = FindStraight(Card);
			if (p) 
			{
				f = GetStraightFactor(MyCard,Hand,p,FALSE,CardNum);
				Memo += _T(" f=%.1f ˳��=",f);
				
				for (int k=0; k<5; k++)
				{
					tstring CardStr=GetSuitStr(Hand[p+k]/100)+GetCardStr(Hand[p+k]);
					Memo += CardStr;
				}
				return f;
			}
		}

		if (CardMaxRepeat==3)//�õ�����,֮ǰ�Ѿ��ų��˺�«
		{
			int p=1;
			while(Card[++p]!=3);
			int32 ThreeCard = Hand[p];
			assert(ThreeCard!=0);

			//ȡ����������������
			int j=15;
			while (--j>1 && Card[j]!=1);
			assert(j!=1);
			int32 HighestCard = Hand[j];
			while (--j>1 && Card[j]!=1); 
			assert(j!=1);
			int32 SecondCard = Hand[j];
			f = GetThreeKindFactor(MyCard, ThreeCard, HighestCard,SecondCard,CardNum);
			
			tstring Card0Str=GetCardStr(ThreeCard);
			tstring Card1Str=GetCardStr(HighestCard);
			tstring Card2Str=GetCardStr(SecondCard);
			Memo += tformat(_T(" f=%.1f ����=%s %s %s %s %s"),f,Card0Str.c_str(),Card0Str.c_str(),Card0Str.c_str(),Card1Str.c_str(),Card2Str.c_str());
			return f;
		}

		//���� �� һ��,��Ҫ��ͬ����˳�ӵ����
        
		if ( CardMaxRepeat == 2 )
		{
			int j=15;
			while (Card[--j]!=2); //��һ������
			int32 PairCard1 = Hand[j];
			assert(PairCard1!=0);

			while (--j>1 && Card[j]!=2);
			if(j!=1){ //�ҵ�����
				int32 PairCard2 = Hand[j];

				int j=15;  //������һ��������
				while (--j>1 && Card[j]!=1);
				assert(j!=1); 
				int32 OtherCard = Hand[j];

	
				f = GetTwoPairFactor(MyCard, PairCard1,PairCard2,OtherCard,CardNum);
                f += FlushFChance(MyCard,SuitMaxRepeat,SuitMaxCard);
			    f += StraightChance(MyCard,Hand);

				tstring Card0Str=GetCardStr(PairCard1);
				tstring Card1Str=GetCardStr(PairCard2);
				tstring Card2Str=GetCardStr(OtherCard);
				Memo += tformat(_T(" f=%.1f ����=%s %s %s %s %s"),f,Card0Str.c_str(),Card0Str.c_str(),Card1Str.c_str(),Card1Str.c_str(),Card2Str.c_str());

                return f;
			}else{  //ֻ��һ�ԣ���������ɢ��
				int j=15;
				while (--j>1  && Card[j]!=1);
				int32 HighestCard = Hand[j];
				while (--j>1  && Card[j]!=1); 
				int32 SecondCard = Hand[j];
				while (--j>1  && Card[j]!=1); 
				int32 ThreeCard = Hand[j];
				assert(ThreeCard!=0);

	
				f = GetOnePairFactor(MyCard, PairCard1,HighestCard,SecondCard,ThreeCard,CardNum);
                f += FlushFChance(MyCard,SuitMaxRepeat,SuitMaxCard);
				f += StraightChance(MyCard,Hand);

				tstring Card0Str=GetCardStr(PairCard1);
				tstring Card1Str=GetCardStr(HighestCard);
				tstring Card2Str=GetCardStr(SecondCard);
				tstring Card3Str=GetCardStr(ThreeCard);

				Memo += tformat(_T(" f=%.1f һ��=%s %s %s %s %s"),f,Card0Str.c_str(),Card0Str.c_str(),Card1Str.c_str(),Card2Str.c_str(),Card3Str.c_str());
                return f;
			}
		} 
		
		//ɢ��,ͬ��Ҫ��ͬ����˳��		
		int j=15;
		while (Card[--j]!=1);
		int32 HighestCard = Hand[j];
		while (Card[--j]!=1); 
		int32 SecondCard = Hand[j];
		while (Card[--j]!=1); 
		int32 ThreeCard = Hand[j];
		while (Card[--j]!=1); 
		int32 FourCard = Hand[j];
		while (Card[--j]!=1); 
		int32 FiveCard = Hand[j];
		assert(FiveCard!=0);
	
		int32 CardSuit=0;       //�ҵ���ͬ���Ļ�ɫ
		int32 i=0;
		for (i; i<4;i++)
		{
			int32 v=Suit[i];
			if (v>4)
			{
				CardSuit = i;
				break;
			}
		};
		f = GetKickerFactor(MyCard, HighestCard,SecondCard,ThreeCard,FourCard,FiveCard,CardNum);
		f += FlushFChance(MyCard,SuitMaxRepeat,SuitMaxCard);
		f += StraightChance(MyCard,Hand);

		
		tstring Card0Str=GetCardStr(HighestCard);
		tstring Card1Str=GetCardStr(SecondCard);
		tstring Card2Str=GetCardStr(ThreeCard);
		tstring Card3Str=GetCardStr(FourCard);
		tstring Card4Str=GetCardStr(FiveCard);
		
		Memo += tformat(_T(" f=%.1f ɢ��=%s %s %s %s %s"),f,Card0Str.c_str(),Card1Str.c_str(),Card2Str.c_str(),Card3Str.c_str(),Card4Str.c_str());
		
		return f;
		
	}

	return f;
}



