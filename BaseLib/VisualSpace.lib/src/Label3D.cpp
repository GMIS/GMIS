// Label3D.cpp: implementation of the CLabel3D class.
//
//////////////////////////////////////////////////////////////////////

#include "Label3D.h"

#pragma warning(disable:4244)

namespace VISUALSPACE{

_glCDefaultFont2D::_glCDefaultFont2D(){
	m_DefaultFontBase = 0;
	m_hFont = NULL;
}
_glCDefaultFont2D::~_glCDefaultFont2D(){
	if (m_DefaultFontBase)
	{
		glDeleteLists(m_DefaultFontBase,255);
	};
	if(m_hFont){
		::DeleteObject(m_hFont);
	}
}

_glCDefaultFont2D   _glAfxDefaultFont2D;
_glCDefaultFont2D&  GetDefaultFont2D(){
	return _glAfxDefaultFont2D;
}

BOOL PASCAL glInitDefaultFont2D(tstring FontName){
 	HFONT	font;										// Windows Font ID
	_glAfxDefaultFont2D.m_DefaultFontBase = glGenLists(256);								// Storage For 256 Characters

	font = CreateFont(	-14,		     				// Height Of Font
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
						FontName.c_str());				// Font Name

	if(font== NULL)return FALSE;
	_glAfxDefaultFont2D.m_hFont = font;

	SelectObject(wglGetCurrentDC(), font);// Selects The Font We Created
   	wglUseFontBitmaps(wglGetCurrentDC(), 0, 255,_glAfxDefaultFont2D.m_DefaultFontBase);
	return TRUE;
}


CText2D::CText2D():m_BmpBits(NULL),m_Width(0),m_Height(0){};
CText2D::~CText2D(){
	if(m_BmpBits){
		delete m_BmpBits;
	    m_BmpBits = NULL;
	}
};


//仅仅适合ASSIC字符组成的字符串
void CText2D::DrawCharText(float x,float y, AnsiString str,float r,float g,float b,HFONT hFont /*= NULL*/) {
	glDisable(GL_BLEND);
	glColor3f(r,g,b);
	unsigned int Base = _glAfxDefaultFont2D.m_DefaultFontBase;
	if (hFont)
	{
		HDC hdc = wglGetCurrentDC();  
		SelectObject(hdc, hFont); 
		Base = glGenLists(255);
		wglUseFontBitmaps(hdc, 0, 255,Base);
	}

	glRasterPos2f(x, y); 
	glListBase(Base); 
	glCallLists(str.size(), GL_UNSIGNED_BYTE, str.c_str()); 

	if(hFont){
		glDeleteLists(Base, 255);
	}
}


//先把文字打印在一个内存BMP上，因此适用中英文和多行文字
void CText2D::DrawBmpText(float x,float y, tstring str,float r/* =1::0f */,float g/* =1::0f */,float b/* =1::0f */,HFONT hFont /* = NULL */){
  glDisable(GL_BLEND);
  glColor3f(r,g,b);
  if(GetBmpTextBit(str,hFont)==NULL)return;	
                                      
  glPixelStorei(GL_UNPACK_ALIGNMENT ,1);
  glRasterPos2f(x,y); 
  glBitmap(m_Width,m_Height,0,0,0,0,m_BmpBits); 
     
}
	
void CText2D::DrawBmpText(float x,float y,float r/* =1::0f */,float g/* =1::0f */,float b/* =1::0f */,HFONT hFont /* = NULL */){
	if(GetBmpTextBit()==NULL)return;
	glDisable(GL_BLEND);
    glColor3f(r,g,b);
 
	glPixelStorei(GL_UNPACK_ALIGNMENT ,1);
    glRasterPos2f(x,y); 
    glBitmap(m_Width,m_Height,0,0,0,0,m_BmpBits); 
}

const UCHAR* CText2D::GetBmpTextBit(tstring str,HFONT hFont /*= NULL*/){
  if(m_BmpBits){
     delete  m_BmpBits;
	 m_BmpBits = NULL;
	 m_Width  = 0;
	 m_Height = 0;
  }

  BITMAP bm;

  HDC MDC = ::CreateCompatibleDC(0);
  HFONT Font = hFont;
  HFONT OldFont;
  if(Font==NULL) Font = _glAfxDefaultFont2D.m_hFont;
  if(Font!=NULL)OldFont = (HFONT)SelectObject(MDC,Font);	

 // ::GetTextExtentPoint32(MDC,str.c_str(),str.size(),&size);
  RECT rc;
  ::SetRect(&rc,0,0,0,0);
  DrawText(MDC,str.c_str(),str.size(),&rc,DT_CALCRECT);
    
  m_Width  = rc.right-rc.left;
  m_Height = rc.bottom-rc.top;

  if(m_Width==0 || m_Height==0)return NULL;

 // size.cx = rc.right; size.cy = rc.bottom;
  HBITMAP bitmap = ::CreateBitmap(m_Width, m_Height, 1, 1, NULL);
  HBITMAP oldBmp=(HBITMAP)SelectObject(MDC,bitmap);
  
  SetBkColor(MDC, RGB(0, 0, 0));  
  SetTextColor(MDC, RGB(255, 255, 255));

  DrawText(MDC,str.c_str(),str.size(),&rc,NULL);
  ::GetObjectW(bitmap, sizeof(BITMAP), &bm);

  m_Width = (bm.bmWidth + 31) & (~31);
  int bufsize =m_Height * m_Width;
  struct {  BITMAPINFOHEADER bih;
			RGBQUAD col[2];
 		 }bic; 
  BITMAPINFO *binf = (BITMAPINFO *)&bic; 
  binf->bmiHeader.biSize     = sizeof(binf->bmiHeader);//
  binf->bmiHeader.biWidth    = bm.bmWidth;
  binf->bmiHeader.biHeight   = bm.bmHeight;
  binf->bmiHeader.biPlanes   = 1;   
  binf->bmiHeader.biBitCount = 1;
  binf->bmiHeader.biCompression = BI_RGB;
  binf->bmiHeader.biSizeImage   = bufsize; 
  m_BmpBits = new UCHAR[bufsize];	
  if(m_BmpBits==NULL)return NULL;

  ::GetDIBits(MDC,bitmap,0,bm.bmHeight,m_BmpBits,binf,DIB_RGB_COLORS); 
    
  ::SelectObject(MDC, oldBmp);
  if(Font != NULL)::SelectObject(MDC,OldFont);
  
  ::DeleteDC(MDC);

//  m_Width  = size.cx;
//  m_Height = size.cy;

  return m_BmpBits;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLabel3::CLabel3(){
    m_z = -0.1;
		 
	m_LeftBottom[0]=0.0f; m_LeftBottom[1]=0.0f;m_LeftBottom[2]=0.0f;
	m_RightTop[0]=0.0f;m_RightTop[1]=0.0f;m_RightTop[2]=0.0f;

	m_State |= SPACE_SHOWBKG;
	m_State |= SPACE_SHOWBORDER;

	m_crBk     = RGB(255,128,128);
	m_crText   = RGB(255,255,255);
	m_crBorder = RGB(255,255,255);

	m_Font = NULL; //使用缺省字体

    m_IsMutilLineText = false;
};

CLabel3::CLabel3(int64 ID, tstring Text)
:CVSpace3(ID),m_Text(Text)
{
    m_LeftBottom[0]=0.0f; m_LeftBottom[1]=0.0f;m_LeftBottom[2]=0.0f;
	m_RightTop[0]=0.0f;m_RightTop[1]=0.0f;m_RightTop[2]=0.0f;

	m_State |= SPACE_SHOWBKG;
	m_State |= SPACE_SHOWBORDER;

	m_crBk     = RGB(0,128,0);
	m_crText   = RGB(255,255,255);
	m_crBorder = RGB(128,128,128);

	m_Font = NULL; //使用缺省字体
    m_Text2D.GetBmpTextBit(Text,m_Font);
    m_IsMutilLineText = false;
};

CLabel3::~CLabel3(){

};

void CLabel3::SetColor(COLORREF crBk, COLORREF crText, COLORREF crBorder){
	m_crBk     = crBk;
	m_crText   = crText;
	m_crBorder = crBorder;
};

void CLabel3::SetText(tstring Text, bool IsMutilLine,HFONT hFont){
	m_Font = hFont;
	m_Text = Text;
	m_IsMutilLineText = IsMutilLine;

	m_Text2D.GetBmpTextBit(Text,m_Font);
}
	
void CLabel3::Layout(bool Redraw){
	
	glPushMatrix();
	glLoadIdentity();

	GLint    viewport[4];

	glGetIntegerv (GL_VIEWPORT, viewport); 

//	if(m_IsMutilLineText){ //多行文本已经确定了文字高宽，此时应该以此为最小高宽
        if(m_Text2D.m_Width>(AreaWidth()-4))m_AreaRight = m_AreaLeft+m_Text2D.m_Width+4;
		if(m_Text2D.m_Height>(AreaHeight()-4))m_AreaBottom = m_AreaTop+m_Text2D.m_Height+4;
//	}
	
	//opengl与window的Y坐标原点相反
	int32 top = viewport[3]-m_AreaTop;
	int32 bottom = viewport[3]-m_AreaBottom;

	m_LeftBottom[0] = m_AreaLeft;
	m_LeftBottom[1] = bottom;
    
	m_RightTop[0] = m_AreaRight;
	m_RightTop[1] = top;
	

	//左小角设置为当前光栅位置
//	ToPlace(left+4,bottom,m_z,0,0,0);
}

void CLabel3::Draw3D(float32* ParentMatrix,ePipeline* Pipe /*= NULL*/){

	glPushMatrix();
	//由于是2D显示在主窗口，父矩阵就是顶级规化矩阵,所以直接调入本地矩阵就ok
	//glLoadMatrixf(m_LocalMatrix);      

	if (m_State & SPACE_SHOWBKG)
	{   
		glColor3ub(GetRValue(m_crBk), GetGValue(m_crBk), GetBValue(m_crBk));
		glBegin(GL_POLYGON);
        glVertex3f(m_LeftBottom[0], m_LeftBottom[1],m_z);
        glVertex3f(m_RightTop[0], m_LeftBottom[1],m_z);
        glVertex3f(m_RightTop[0], m_RightTop[1],m_z);
        glVertex3f(m_LeftBottom[0], m_RightTop[1],m_z);
        glEnd();
	} 
	if(m_State & SPACE_SHOWBORDER)
	{
        glColor3ub(GetRValue(m_crBorder), GetGValue(m_crBorder), GetBValue(m_crBorder));
		glBegin(GL_LINE_LOOP);
        glVertex3f(m_LeftBottom[0], m_LeftBottom[1],m_z);
        glVertex3f(m_RightTop[0], m_LeftBottom[1],m_z);
        glVertex3f(m_RightTop[0], m_RightTop[1],m_z);
        glVertex3f(m_LeftBottom[0], m_RightTop[1],m_z);
        glEnd();
	}

//	glLoadMatrixf(m_LocalMatrix);      

//	if(m_IsMutilLineText){
		m_Text2D.DrawBmpText(m_LeftBottom[0]+2,m_LeftBottom[1]+4, m_Text,(float)GetRValue(m_crText)/255, (float)GetGValue(m_crText)/255, (float)GetBValue(m_crText)/255,m_Font);
//	}else{
//		m_Text2D.DrawCharText(m_LeftBottom[0]+2,m_LeftBottom[1]+4,m_Text,(float)GetRValue(m_crText)/255, (float)GetGValue(m_crText)/255, (float)GetBValue(m_crText)/255,m_Font);
//	}
	glPopMatrix();
}


ButtonItem3::ButtonItem3()
{	
}		
ButtonItem3::ButtonItem3(int64 CmdID,const TCHAR* Name)
:CLabel3(CmdID,Name)
{
};
void ButtonItem3::Draw3D(float32* ParentMatrix,ePipeline* Pipe /*= NULL*/){
		
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);

	glPushMatrix();
	//由于是2D显示在主窗口，父矩阵就是顶级规化矩阵,所以直接调入本地矩阵就ok
	//glLoadMatrixf(m_LocalMatrix);      
    int alpha = 128;
    if(m_State & SPACE_FOCUSED){
	    alpha = 192;
	}
	if (m_State & SPACE_SHOWBKG)
	{   
		glColor4ub(GetRValue(m_crBk), GetGValue(m_crBk), GetBValue(m_crBk),alpha);
		glBegin(GL_POLYGON);
        glVertex3f(m_LeftBottom[0], m_LeftBottom[1],m_z);
        glVertex3f(m_RightTop[0], m_LeftBottom[1],m_z);
        glVertex3f(m_RightTop[0], m_RightTop[1],m_z);
        glVertex3f(m_LeftBottom[0], m_RightTop[1],m_z);
        glEnd();
	} 
	if(m_State & SPACE_SHOWBORDER)
	{
        glColor4ub(GetRValue(m_crBorder), GetGValue(m_crBorder), GetBValue(m_crBorder),alpha);
		glBegin(GL_LINE_LOOP);
        glVertex3f(m_LeftBottom[0], m_LeftBottom[1],m_z);
        glVertex3f(m_RightTop[0], m_LeftBottom[1],m_z);
        glVertex3f(m_RightTop[0], m_RightTop[1],m_z);
        glVertex3f(m_LeftBottom[0], m_RightTop[1],m_z);
        glEnd();
	}

//	glLoadMatrixf(m_LocalMatrix);      

//	if(m_IsMutilLineText){
		m_Text2D.DrawBmpText(m_LeftBottom[0]+2,m_LeftBottom[1]+4, m_Text,(float)GetRValue(m_crText)/255, (float)GetGValue(m_crText)/255, (float)GetBValue(m_crText)/255,m_Font);
//	}else{
//		m_Text2D.DrawCharText(m_LeftBottom[0]+2,m_LeftBottom[1]+4,m_Text,(float)GetRValue(m_crText)/255, (float)GetGValue(m_crText)/255, (float)GetBValue(m_crText)/255,m_Font);
//	}
	glPopMatrix();
}

}//namespace VISUALSPACE
