// Text3D.cpp: implementation of the Font3D class.
//
//////////////////////////////////////////////////////////////////////
//#include "StdAfx.h"
#include "Text3D.h"

namespace VISUALSPACE{

_glCDefaultFont3D::_glCDefaultFont3D(){
	m_DefaultFontBase = 0;
}

_glCDefaultFont3D::~_glCDefaultFont3D(){
	if (m_DefaultFontBase)
	{
		glDeleteLists(m_DefaultFontBase,255);
	}
}

_glCDefaultFont3D   _glAfxDefaultFont3D;

BOOL PASCAL glInitDefaultFont3D(tstring FontName){
	_glAfxDefaultFont3D.m_DefaultFontBase = glGenLists(256);								// Storage For 256 Characters

	HFONT font = CreateFont(	-12,		     				// Height Of Font
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
						FontName.c_str());				// Font Name

	if(font== NULL)return FALSE;
	HFONT OldFont = (HFONT)SelectObject(wglGetCurrentDC(), font);// Selects The Font We Created

	wglUseFontOutlines(	wglGetCurrentDC(),							// Select The Current DC
						0,								// Starting Character
						255,							// Number Of Display Lists To Build
						_glAfxDefaultFont3D.m_DefaultFontBase,							// Starting Display Lists
						0.0f,							// Deviation From The True Outlines
						0.1f,							// Font Thickness In The Z Direction
						WGL_FONT_POLYGONS,				// Use Polygons, Not Lines
						_glAfxDefaultFont3D.m_Gmf);	                        // Address Of Buffer To Recieve Data
	::SelectObject(wglGetCurrentDC(),OldFont);
	::DeleteObject(font);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////

CText3DEN::CText3DEN(){
     m_crDefault = RGB(200,128,0);	
	 m_Width  = 0.5f;
	 m_Height = 0.5f;
};

CText3DEN::~CText3DEN(){

};

void CText3DEN::SetText(tstring Text){
     m_Text = Text;
}



void CText3DEN::SetTextRect(float dx,float dy){
	m_Width  = dx;
	m_Height = dy;

}

void CText3DEN::ToPlace(float transx, float transy, float transz,float rotx, float roty, float rotz){
	glPushMatrix();
	LocalIdentity();
	glLoadMatrixf(m_LocalMatrix);

	//注意次序，必须是先Trans后rot
    glTranslatef(transx, transy,transz);

	if(rotx){
		glRotatef(rotx, 1.0f,0.0f,0.0f);
	}
	if(roty){
		glRotatef(roty, 0.0f,1.0f,0.0f);
	}
	if(rotz){
		glRotatef(rotz, 0.0f,0.0f,1.0f);
	}
	if(m_Text.size() && m_Width!=0 && m_Height !=0){
        GLYPHMETRICSFLOAT& g = _glAfxDefaultFont3D.m_Gmf[m_Text[0]]; 
		float h = g.gmfBlackBoxY ;
		float length=g.gmfBlackBoxX*m_Text.size();

		glScalef(m_Width/length,m_Height/h,0);
		glTranslatef(-length/2, 0,0);

	}
	//暂存运算结果
	glGetFloatv(GL_MODELVIEW_MATRIX,m_LocalMatrix);
    glPopMatrix();
}
	
void CText3DEN::Draw(GLfloat* ParentMatrix){

	glLoadMatrixf(ParentMatrix);  //引入父空间的矩阵变换，
	glMultMatrixf(m_LocalMatrix);     //得到本物体基于世界坐标的矩阵变换
	
	if (m_Text.size())
	{
		glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
		glColor3ub(GetRValue(m_crDefault),GetGValue(m_crDefault),GetBValue(m_crDefault));
		glListBase(_glAfxDefaultFont3D.m_DefaultFontBase);									// Sets The Base Character to 0
		glCallLists(m_Text.size(), GL_UNSIGNED_BYTE, m_Text.c_str());	// Draws The Display List Text
		glPopAttrib();					
	}
}

////////////////////////////////////////////////////////////////////////

CText3D::CText3D(){
     m_crDefault = RGB(200,128,0);	
	 m_Width  = 0.5f;
	 m_Height = 0.5f;
	 m_TextOutline = NULL;
	 m_TextSize = 0;
};

CText3D::~CText3D(){
	Clear();
};
	
void CText3D::Clear(){
	if(m_TextOutline){
		for (int i=0; i<m_TextSize; i++)
		{
			_FontOutline& fo = m_TextOutline[i];
	         glDeleteLists(fo.ListBase, 1);   
		}
		delete m_TextOutline;
		m_TextOutline = NULL;
	}
}

void CText3D::SetText(const TCHAR* lpszText,HFONT hFont){
	Clear();

    m_Text = lpszText;
	int len = m_Text.size();
    m_TextSize = len;

	HDC hdc = wglGetCurrentDC();

	HFONT font = hFont;
    if (hFont==NULL)
    {
		font = CreateFont( -20,		     				// Height Of Font
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
			_T("Fixedsys"));				// Font Name
	}
	HFONT hOldFont=(HFONT)::SelectObject(hdc,font);
	UCHAR * pChar=(UCHAR*)lpszText;
	
	if(m_TextOutline)delete m_TextOutline;
    m_TextOutline = new _FontOutline[len];

	int   nListNum;  
	DWORD dwChar;  

	for(int i = 0; i < len; i++)
	{
	  _FontOutline& fo=m_TextOutline[i];

	  if(IsDBCSLeadByte((BYTE)pChar[i]))
		{ dwChar=(DWORD)((pChar[i]<<8)|pChar[i+1]);
		  i++;
		}
	  else	dwChar = pChar[i];
	  nListNum = glGenLists(1);  

      fo.ListBase = nListNum;
	  wglUseFontOutlines( hdc,	
						  dwChar,	
						  1,
						  nListNum,	
						  0.0f, 
						  0.0f,
						  WGL_FONT_POLYGONS,
						  &fo.gmf	
						);

	}
	::SelectObject(hdc,hOldFont);
	if(hFont==NULL && font){
		::DeleteObject(font);
	}
}



void CText3D::SetTextRect(float dx,float dy){
	m_Width  = dx;
	m_Height = dy;
}

void CText3D::ToPlace(float transx, float transy, float transz,float rotx, float roty, float rotz){
	glPushMatrix();
	LocalIdentity();
	glLoadMatrixf(m_LocalMatrix);

	//注意次序，必须是先Trans后rot
    glTranslatef(transx, transy,transz);

	if(rotx){
		glRotatef(rotx, 1.0f,0.0f,0.0f);
	}
	if(roty){
		glRotatef(roty, 0.0f,1.0f,0.0f);
	}
	if(rotz){
		glRotatef(rotz, 0.0f,0.0f,1.0f);
	}
	if(m_TextOutline && m_Width!=0 && m_Height !=0){
        GLYPHMETRICSFLOAT& g = m_TextOutline[0].gmf; 
		float h = g.gmfBlackBoxY ;
		float length=0;
		for (int i=0;i<m_TextSize;i++)	// Loop To Find Text Length
		{
			length += m_TextOutline[0].gmf.gmfCellIncX;			// Increase Length By Each Characters Width
		}
        
		if(length==0)length=1;
		if(h==0)h=1;

		glScalef(m_Width/length,m_Height/h,0);
		glTranslatef(-length/2, 0,0);

	}
	//暂存运算结果
	glGetFloatv(GL_MODELVIEW_MATRIX,m_LocalMatrix);
    glPopMatrix();
}
	
void CText3D::Draw(GLfloat* ParentMatrix){

	glLoadMatrixf(ParentMatrix);  //引入父空间的矩阵变换，
	glMultMatrixf(m_LocalMatrix);     //得到本物体基于世界坐标的矩阵变换
	
	if (m_TextOutline)
	{
		glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
		//glEnable(GL_BLEND);
		glColor4ub(GetRValue(m_crDefault),GetGValue(m_crDefault),GetBValue(m_crDefault),255);
		for (int i=0; i<m_TextSize; i++)
		{
			_FontOutline& fo= m_TextOutline[i];
			glCallList(fo.ListBase);	// Draws The Display List Text
		}
        //glDisable(GL_BLEND);
		glPopAttrib();		
		glColor3ub(255,255,255);
	}
}
////////////////////////////////////////////////////////////////////////

} //namespace VISUALSPACE