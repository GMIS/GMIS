/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _OBJECT3D_H__
#define _OBJECT3D_H__


#include "VisibleSpace.h"
#include "Text3D.h"
#include <gl\glaux.h>

using namespace VISUALSPACE;

AUX_RGBImageRec* LoadBMP(const TCHAR* Filename);                // Loads A Bitmap Image


/////////////////////////////////////////////////////////////////
typedef struct tagVERTEX
{
	float x, y, z;
	float u, v;
} VERTEX;

typedef struct tagTRIANGLE
{
	VERTEX vertex[3];
} TRIANGLE;

class RECT3D 
{
public:
	TRIANGLE  triangle[2];
public:	
	void Draw();
	//即横向和纵向铺多少个单位的纹理
	void SetTextureUnit(float width,float Height);
};

	
//tool
	
void RectToTri(float x0,float y0, float z0,
		float x1,float y1, float z1,
		float x2,float y2, float z2,
		float x3,float y3, float z3,
		TRIANGLE& Tri1,TRIANGLE& Tri2,float TexSize=1.0f);




//假定为BOX,基于轴对称初始化
enum FACEPOS { FRONTFACE=0,LEFTFACE,BACKFACE,RIGHTFACE,TOPFACE,BOTTOMFACE};

class CObject3D : public CVSpace3  
{
protected:
	RECT3D          m_Surface[6];
	UINT32          m_TextureNum;
	COLORREF        m_crDefault;   //备用的显示颜色，当纹理没有设置时
public:
	CText3D         m_Text;
	tstring         m_Fingerprint;
public:
	CObject3D(int64 ID,tstring Fingerprint);
	virtual ~CObject3D();


/* 初始化空间顶点坐标,基于自身重心
            __________________ 
		   /|                /| 
          / |               / |
         /  |              /  | 
		/   |           dz/   | 
	   /    |            /    |
      /_____|____dx_____/     | 
	  |     |           |     | 
	  |    / -----------|----/
	  |   /             |   /
	  |  /            dy|  /
	  | /               | / 
      |/_______________ |/

*/
	void InitSpace(float dx, float dy, float dz);
	void SetTexture(int TextNum);
	

	RECT3D&  GetSurFace(FACEPOS FacePos){ return m_Surface[FacePos];};

	virtual void Draw(GLfloat* ParentMatrix);
};

#endif // !defined(_OBJECT3D_H__)
