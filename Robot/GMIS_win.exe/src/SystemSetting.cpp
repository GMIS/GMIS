#include "SystemSetting.h"
#include <tchar.h>
#include "resource.h"


SystemSetting::SystemSetting(){
	
	//default font
	m_SystemFont = NULL;
	m_Font8   = NULL;
	m_Font10  = NULL;
	m_Font12  = NULL;
	m_Font12B = NULL;
	m_Font24  = NULL;
	
	//default
	m_crViewBkg = RGB(0,0,0);
	m_crViewBorder = RGB(128,128,128);
	
	//Main Title
	m_imgMainTitle  = NULL;
	m_crMainTitle = RGB(255,0,0);
	
	//Title text
	m_crActTitleText = RGB(192,192,192);
	m_crFocusedTitleText= RGB(00,255,00);
	
	//Title bnt
	m_crActTitleBnt= RGB(255,255,255);
	m_crTitleBntBorder= RGB(192,192,192);
	m_crFocusedTitleBnt= RGB(00,255,00);
	m_crSelectedTitleBnt= RGB(00,44,255);
	
	//skin
	m_imgMainSkin   = NULL;
	m_crMainSkin  = RGB(128,181,136);
	
	//default border
	m_imgBorder = NULL;
	m_crNormalBorder= RGB(192,192,192);
	m_crInactiveBorder= RGB(255,255,255);
	m_crFocusedBorder= RGB(00,255,00);
	
	//main bnt
	m_imgMainBntP  = NULL;
	m_imgMainBntA  = NULL;
	
	m_crBntBorder = RGB(128,128,128);
	m_crBntFg= RGB(192,192,192); 
	m_crBntBg= RGB(0,192,0); 
	m_crBntFocused= RGB(4,11,99); 
	
	//conv title
	m_imgConvTitle = NULL;
	
	//conv skin
	m_imgConvSkin = NULL;
	m_crConvSkin = RGB(128,128,128);   
	
	//NewConv bnt
	m_imgNewConvBntA   = NULL;
	m_imgNewConvBntP = NULL;
	
	//sendbnt
	m_imgSendBnt = NULL;
	m_imgEnterBnt = NULL;
	
	//conv toolbar
	m_imgConvToolSkin  = NULL;
	
	//conv view lock bnt
	m_imgViewLockOpen = NULL;
	m_imgViewLockClose = NULL;
	//conv icon
	m_DefaultConvIcon = NULL; 
	
	//brain view
	crBrainViewItemText = RGB(0x17,0x35,0x7b);
	crBrainViewBorder = RGB(0,255,0);
	crBrainViewToolbarBk = RGB(160,160,160);
	crBrainViewItemBorder = RGB(0,0,160);
	crBrainViewConnectionLine = RGB(128,128,128);
	crBrainViewItemTextBk= RGB(128,128,128);
	
	//task view
	crTaskMassBorder= RGB(255,128,255);  
	crTaskMassText= RGB(0,0,0);
	crTaskMassBk = RGB(255,255,255);   
	crTaskMassPause = RGB(200,128,128);
	crTaskMassBreak = RGB(128,128,200);
	crTaskMassDeleted = RGB(100,100,100);

	m_imgRun = NULL; 
	m_imgPause = NULL; 
	m_imgStop = NULL; 
	m_imgStep = NULL; 
	m_imgGoto = NULL; 
	
	m_imgNavibar = NULL;
	
	//SceneView
	m_crSceneNormal= RGB(0,0,0);  
	m_crSceneNew= RGB(0,180,0);
	m_crSceneDel = RGB(180,180,180);   

	HPEN   m_hVectorPen = NULL;
};

SystemSetting::~SystemSetting(){
	ReleaseRes();
};

bool SystemSetting::Init(const char* di /*r=NULL*/){
	ReleaseRes();
	   
	try
	{
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
//font
		m_SystemFont    = (HFONT)::GetStockObject(ANSI_VAR_FONT);
		
		m_Font8         = CreateFont(	-8,		     				// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_NORMAL,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						_T("宋体"));

		m_Font10         = CreateFont(	-10,		     				// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_NORMAL,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						_T("Fixedsys"));
	
		m_Font12         = CreateFont(	-16,		     				// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_NORMAL,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						_T("宋体"));

		m_Font12B         = CreateFont(	-12,		     				// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						_T("Times New Roman"));	
		
		m_Font24         = CreateFont(	-24,		     				// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						_T("Fixedsys"));	
/*
//Main Title
		m_imgMainTitle  = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_MAINTITLE),IMAGE_BITMAP,0,0,0);

//skin
		m_imgMainSkin   = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_MATERIAL),IMAGE_BITMAP,0,0,0);

//default border
		m_imgBorder = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_BORDER),IMAGE_BITMAP,0,0,0);

//default bnt
		m_imgMainBntP    = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_MAINBNTP),IMAGE_BITMAP,0,0,0);
		m_imgMainBntA    = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_MAINBNTA),IMAGE_BITMAP,0,0,0);

//conv title
		m_imgConvTitle    = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_CONVTITLE),IMAGE_BITMAP,0,0,0);

//conv skin
        m_imgConvSkin    = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_CONVSKIN),IMAGE_BITMAP,0,0,0);

//NewConv bnt
		m_imgNewConvBntA    = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_NEWCONVBNTA),IMAGE_BITMAP,0,0,0);
		m_imgNewConvBntP  = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_NEWCONVBNTP),IMAGE_BITMAP,0,0,0);
//send bnt
		m_imgSendBnt = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_SENDBNT),IMAGE_BITMAP,0,0,0);
		m_imgEnterBnt = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_ENTERBNT),IMAGE_BITMAP,0,0,0);

//conv toolbar
		m_imgConvToolSkin  = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_CONVTOOLSKIN),IMAGE_BITMAP,0,0,0);
//conv view lock bnt
        m_imgViewLockOpen = ::LoadIcon(hInstance,MAKEINTRESOURCE(IDI_UNLOCKVIEW));
        m_imgViewLockClose =::LoadIcon(hInstance,MAKEINTRESOURCE(IDI_LOCKVIEW));		
//conv icon
        m_DefaultConvIcon = ::LoadIcon(hInstance,MAKEINTRESOURCE(IDI_CONVVIEW));

//task icon
		m_imgRun=  ::LoadImage(hInstance,MAKEINTRESOURCE(IDI_RUN),IMAGE_ICON,16,16,0);
        m_imgPause=::LoadImage(hInstance,MAKEINTRESOURCE(IDI_PAUSE),IMAGE_ICON,16,16,0);
		m_imgStop= ::LoadImage(hInstance,MAKEINTRESOURCE(IDI_STOP),IMAGE_ICON,16,16,0);
		m_imgStep= ::LoadImage(hInstance,MAKEINTRESOURCE(IDI_STEP),IMAGE_ICON,16,16,0);
		m_imgGoto= ::LoadImage(hInstance,MAKEINTRESOURCE(IDI_GOTO),IMAGE_ICON,16,16,0);
*/

//Navibar
		m_imgNavibar = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_NAVIBAR),IMAGE_BITMAP,0,0,0);

//SceneView
	    m_hVectorPen = ::CreatePen(PS_SOLID, 3, RGB(0,200,0));
	}
	catch (...)
	{
		return false;
		::MessageBox(NULL,_T("Resourse Init Error!"),_T("FAIL"),MB_OK);
	}

	return true;
}

void SystemSetting::ReleaseRes(){

//  FONT
	if(m_Font8)::DeleteObject(m_Font8);     m_Font8 = NULL;
	if(m_Font10)::DeleteObject(m_Font10);   m_Font10 = NULL;
	if(m_Font12)::DeleteObject(m_Font12);   m_Font12 = NULL;
	if(m_Font12B)::DeleteObject(m_Font12B); m_Font12B = NULL;
	if(m_Font24)::DeleteObject(m_Font24);   m_Font24 = NULL;

//Main Title
	if(m_imgMainTitle)::DeleteObject(m_imgMainTitle); m_imgMainTitle = NULL;

//skin
	if(m_imgMainSkin)::DeleteObject(m_imgMainSkin); m_imgMainSkin = NULL;
//default border
	if(m_imgBorder)::DeleteObject(m_imgBorder); m_imgBorder = NULL;

//default bnt
	if(m_imgMainBntP)::DeleteObject(m_imgMainBntP); m_imgMainBntP = NULL;
	if(m_imgMainBntA)::DeleteObject(m_imgMainBntA); m_imgMainBntA = NULL;

//conv title
	if(m_imgConvTitle)::DeleteObject(m_imgConvTitle); m_imgConvTitle = NULL;

//conv skin
	if(m_imgConvSkin)::DeleteObject(m_imgConvSkin); m_imgConvSkin = NULL;

//NewConv bnt
	if(m_imgNewConvBntA)::DeleteObject(m_imgNewConvBntA); m_imgNewConvBntA = NULL;
	if(m_imgNewConvBntP)::DeleteObject(m_imgNewConvBntP); m_imgNewConvBntP = NULL;
//sendbnt
	if(m_imgSendBnt)::DeleteObject(m_imgSendBnt);   m_imgSendBnt = NULL;
	if(m_imgEnterBnt)::DeleteObject(m_imgEnterBnt); m_imgEnterBnt = NULL;

//conv toolbar
	if(m_imgConvToolSkin)::DeleteObject(m_imgConvToolSkin); m_imgConvToolSkin = NULL;
//conv view lock bnt  
	if(m_imgViewLockOpen) ::DestroyIcon(m_imgViewLockOpen); m_imgViewLockOpen = NULL;
	if(m_imgViewLockClose)::DestroyIcon(m_imgViewLockClose); m_imgViewLockClose = NULL;
//conv icon
	if(m_DefaultConvIcon)::DestroyIcon(m_DefaultConvIcon); m_DefaultConvIcon = NULL;
    
	if(m_imgRun)::DeleteObject(m_imgRun);     m_imgRun = NULL;
	if(m_imgPause)::DeleteObject(m_imgPause); m_imgPause = NULL;
	if(m_imgStop)::DeleteObject(m_imgStop);   m_imgStop = NULL;
	if(m_imgStep)::DeleteObject(m_imgStep);   m_imgStep = NULL;
	if(m_imgGoto)::DeleteObject(m_imgGoto);   m_imgGoto = NULL;

	if(m_imgNavibar)::DeleteObject(m_imgNavibar); m_imgNavibar = NULL;

	if(m_hVectorPen)::DeleteObject(m_hVectorPen); m_hVectorPen = NULL;

};
