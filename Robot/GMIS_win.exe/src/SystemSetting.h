/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef   _SYSTEMSET_H_
#define   _SYSTEMSET_H_


#include <windows.h>
/*统一配制颜色

*/
class SystemSetting{
public: 
	//default font
	HFONT              m_SystemFont;
	HFONT              m_Font8;
	HFONT              m_Font10;  
	HFONT              m_Font12;   
	HFONT              m_Font12B;  
	HFONT              m_Font24;
	
	//default system
	COLORREF           m_crViewBkg;
    COLORREF           m_crViewBorder;

	//Main Title
	HANDLE             m_imgMainTitle;
    COLORREF           m_crMainTitle;   //如果没有贴图则使用此缺省颜色
    
	//Main Title text
	COLORREF           m_crActTitleText;
    COLORREF           m_crFocusedTitleText;

	//Main Title bnt
	COLORREF           m_crActTitleBnt;
	COLORREF           m_crTitleBntBorder;
	COLORREF           m_crFocusedTitleBnt;
	COLORREF           m_crSelectedTitleBnt;

	//main skin
	HANDLE             m_imgMainSkin;
	COLORREF           m_crMainSkin;

	//default border
	HANDLE             m_imgBorder;
    COLORREF           m_crNormalBorder;
	COLORREF           m_crInactiveBorder;
	COLORREF           m_crFocusedBorder;

	//Main bnt
	HANDLE             m_imgMainBntP;
	HANDLE             m_imgMainBntA;

	COLORREF           m_crBntBorder;
	COLORREF           m_crBntFg;
	COLORREF           m_crBntBg;
	COLORREF           m_crBntFocused;

	//conv title
	HANDLE             m_imgConvTitle;
    COLORREF           m_crConvTitle;   
    
	//conv skin
	HANDLE             m_imgConvSkin;
    COLORREF           m_crConvSkin;   
	
	//NewConv bnt
	HANDLE             m_imgNewConvBntA;
	HANDLE             m_imgNewConvBntP;

	//SendBnt
	 HANDLE            m_imgSendBnt;
	 HANDLE            m_imgEnterBnt;
    //conv toolbar
	HANDLE             m_imgConvToolSkin;
	//conv view lock bnt
    HICON              m_imgViewLockOpen;
	HICON              m_imgViewLockClose;
    //conv icon
	HICON              m_DefaultConvIcon;
	
	//brain view
	COLORREF crBrainViewItemText;
 	COLORREF crBrainViewBorder;
  	COLORREF crBrainViewToolbarBk;
	COLORREF crBrainViewItemBorder;
	COLORREF crBrainViewConnectionLine;
	COLORREF crBrainViewItemTextBk;
	
	//task view
    COLORREF crTaskMassBorder;   
	COLORREF crTaskMassText;
	COLORREF crTaskMassBk;     
	COLORREF crTaskMassPause;
    COLORREF crTaskMassBreak;
	COLORREF crTaskMassDeleted;

	HANDLE     m_imgRun;
	HANDLE     m_imgPause;
	HANDLE     m_imgStop;
	HANDLE     m_imgStep;
	HANDLE     m_imgGoto;
   
	//Navibar
	HANDLE    m_imgNavibar;

	//Scene View
    COLORREF  m_crSceneNormal;   
	COLORREF  m_crSceneNew;
	COLORREF  m_crSceneDel;   
	HPEN      m_hVectorPen;
public:	
	bool Init(const char* dir=NULL);
    void ReleaseRes();

	SystemSetting();
	~SystemSetting();
};

static SystemSetting SS;

#endif //_SYSTEMSET_H_