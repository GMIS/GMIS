/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _TEXT3D_H__
#define _TEXT3D_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VisibleSpace.h"
#include <tchar.h>

namespace VISUALSPACE{

class _glCDefaultFont3D
{
public:
	_glCDefaultFont3D();
	~_glCDefaultFont3D();
public:
	GLYPHMETRICSFLOAT m_Gmf[256]; 
	GLuint            m_DefaultFontBase;
};

BOOL PASCAL glInitDefaultFont3D(tstring FontName);


//使用缺省字体
class CText3DEN: public CVSpace3
{
public:
	tstring        m_Text;
	float          m_Width;
	float          m_Height;
	COLORREF       m_crDefault;

public:
	CText3DEN();
	~CText3DEN();

    void SetText(tstring Text); 
	void SetTextRect(float dx,float dy); //必须在ToPlace之前使用
	//注意，移动的坐标基点是字符串中心
	void ToPlace(float transx, float transy, float transz,float rotx, float roty, float rotz);
	virtual void Draw(GLfloat* ParentMatrix);
};

//中英文都适合，但效率要低点
class CText3D: public CVSpace3
{
public:
	tstring    m_Text;
	float      m_Width;
	float      m_Height;
	COLORREF   m_crDefault;
protected:
    struct  _FontOutline{
		GLYPHMETRICSFLOAT  gmf;
		GLuint             ListBase;
	};

	_FontOutline*   m_TextOutline;
	int             m_TextSize;
	void Clear();

public:
	CText3D();
	~CText3D();

	//请在ToPlace之前使用
	void SetTextRect(float dx,float dy); 
  
	//把文本包含的字符生成m_TextOutline
    void SetText(const TCHAR* lpszText,HFONT hFont=NULL); //hFont=NULL使用缺省字体Fixedsys

	//注意，移动的坐标基点是字符串中心
	void ToPlace(float transx, float transy, float transz,float rotx, float roty, float rotz);

	virtual void Draw(GLfloat* ParentMatrix);
};

} //namespace VISUALSPACE

#endif // _TEXT3D_H__