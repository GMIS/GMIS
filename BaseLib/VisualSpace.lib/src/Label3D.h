/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _LABLE2D_H__
#define _LABLE2D_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VisibleSpace.h"
#include "WinSpace2.h"
#include "Text3D.h"

namespace VISUALSPACE{

class _glCDefaultFont2D
{
public:
	_glCDefaultFont2D();
	~_glCDefaultFont2D();
public:
	GLuint    m_DefaultFontBase;
	HFONT     m_hFont;
};

BOOL PASCAL glInitDefaultFont2D(tstring FontName);
_glCDefaultFont2D&  GetDefaultFont2D();

class CText2D
{
private:
	UCHAR* m_BmpBits;
public:
	int    m_Width;  //m_BmpBits的宽和高，如果使用DrawCharText则没有意义
	int    m_Height;
public:
	CText2D();
	virtual ~CText2D();

    //仅仅适合ASSIC字符组成的字符串,注意这里的位置坐标是光栅位置，(x,y)为字符串左下角
	void DrawCharText(float x,float y, AnsiString str,float r=1.0f,float g=1.0f,float b=1.0f,HFONT hFont = NULL); //hFont=NULL则使用缺省字体
	
	//根据指定的文字和字体在内存中打印成bitmap，适用中英文和多行文字
    const UCHAR* GetBmpTextBit(tstring str,HFONT hFont = NULL);

	//根据指定颜色直接输出GetBmpTextBit()的结果
	void DrawBmpText(float x,float y, tstring str,float r=1.0f,float g=1.0f,float b=1.0f,HFONT hFont = NULL);

	//假设已经调用过GetBmpTextBit完成了文字的光栅化
	void DrawBmpText(float x,float y, float r=1.0f,float g=1.0f,float b=1.0f,HFONT hFont = NULL);
	//得到之前的bitmap,方便重复绘制相同的文字
	const UCHAR* GetBmpTextBit(){ return m_BmpBits;}	

};


/*绘制一个矩形，可以设置不同的边框、背景和文字的颜色
  可以设置单行文字作为button, 也可以设置多行文字显示提示信息
  缺省:
  if(m_State&SPACE_SHOWBKG)绘制背景
  if(m_State&SPACE_SHOWBORDER)绘制边框
*/
#define SPACE_SHOWBORDER  SPACE_SHOWWINBORDER

class CLabel3 : public CVSpace3  
{
public:
	GLdouble   m_z;  //视锥体的近平面，在此绘制文字，它在你初始化视口时定义 
protected:
        
    GLdouble  m_LeftBottom[2];
	GLdouble  m_RightTop[2];

	COLORREF m_crBk;
	COLORREF m_crText;
	COLORREF m_crBorder;
	
	tstring   m_Text;
	HFONT     m_Font;

	bool     m_IsMutilLineText;  //决定m_Text2d绘制方式，当使用多行或文字较多时打开m_IsMutilLine更好
	CText2D  m_Text2D;

public:
    CLabel3();
	CLabel3(int64 ID, tstring Text);
	virtual ~CLabel3();
    
	//标准窗口坐标
    void SetText(tstring Text, bool IsMutilLine,HFONT hFont=NULL);
	virtual void Layout(bool Redraw = true);
	void SetColor(COLORREF crBk, COLORREF crText, COLORREF crBorder);
	int  GetTextLenght(){ return m_Text.size();}
	virtual void  Draw3D(float32* ParentMatrix,ePipeline* Pipe = NULL);

};


class  ButtonItem3 : public CLabel3{

public:
	ButtonItem3();
	ButtonItem3(int64 CmdID,const TCHAR* text);	
	virtual ~ButtonItem3(){};
	virtual void Draw3D(float32* ParentMatrix,ePipeline* Pipe = NULL);
};

} //namespace VISUALSPACE

#endif // _LABLE2D_H__
