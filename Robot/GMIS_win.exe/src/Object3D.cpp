// Object3D.cpp: implementation of the CObject3D class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable:4786)

#include "Object3D.h"

AUX_RGBImageRec* LoadBMP(const TCHAR* Filename)       // Loads A Bitmap Image
{
        FILE *File=NULL;                                // File Handle

        if (!Filename)                                  // Make Sure A Filename Was Given
        {
                return NULL;                            // If Not Return NULL
        }

        File=_tfopen(Filename,_T("r"));                       // Check To See If The File Exists

        if (File)                                       // Does The File Exist?
        {
                fclose(File);                           // Close The Handle
                return auxDIBImageLoad(Filename);       // Load The Bitmap And Return A Pointer
        }
        return NULL;                                    // If Load Failed Return NULL
}

void RECT3D::Draw(){
		glBegin(GL_TRIANGLES);
		glTexCoord2f(triangle[0].vertex[0].u,triangle[0].vertex[0].v); 
		glVertex3f(triangle[0].vertex[0].x,triangle[0].vertex[0].y,triangle[0].vertex[0].z);
		glTexCoord2f(triangle[0].vertex[1].u,triangle[0].vertex[1].v); 
		glVertex3f(triangle[0].vertex[1].x,triangle[0].vertex[1].y,triangle[0].vertex[1].z);
		glTexCoord2f(triangle[0].vertex[2].u,triangle[0].vertex[2].v); 
		glVertex3f(triangle[0].vertex[2].x,triangle[0].vertex[2].y,triangle[0].vertex[2].z);
		
		glTexCoord2f(triangle[1].vertex[0].u,triangle[1].vertex[0].v); 
		glVertex3f(triangle[1].vertex[0].x,triangle[1].vertex[0].y,triangle[1].vertex[0].z);
		glTexCoord2f(triangle[1].vertex[1].u,triangle[1].vertex[1].v); 
		glVertex3f(triangle[1].vertex[1].x,triangle[1].vertex[1].y,triangle[1].vertex[1].z);
		glTexCoord2f(triangle[1].vertex[2].u,triangle[1].vertex[2].v); 
		glVertex3f(triangle[1].vertex[2].x,triangle[1].vertex[2].y,triangle[1].vertex[2].z);
		glEnd();	
};

//即横向和纵向铺多少个单位的纹理
void RECT3D::SetTextureUnit(float Width,float Height){
	triangle[0].vertex[0].v = Height;
	triangle[0].vertex[2].u = Width;
	triangle[1].vertex[0].v = Height;
	triangle[1].vertex[1].u = Width;
	triangle[1].vertex[2].u = Width;
	triangle[1].vertex[2].v = Height;	
}

//对于3d矩形面分割成三角形
void RectToTri(float x0,float y0, float z0,
				float x1,float y1, float z1,
				float x2,float y2, float z2,
				float x3,float y3, float z3,
				TRIANGLE& Tri1,TRIANGLE& Tri2,float TexSize){


	//三角形1
	Tri1.vertex[0].x = x0;
	Tri1.vertex[0].y = y0;
	Tri1.vertex[0].z = z0;
	Tri1.vertex[0].u = 0;
	Tri1.vertex[0].v = TexSize;
 
	Tri1.vertex[1].x = x1;
	Tri1.vertex[1].y = y1;
	Tri1.vertex[1].z = z1;
	Tri1.vertex[1].u = 0;
	Tri1.vertex[1].v = 0;

	Tri1.vertex[2].x = x2;
	Tri1.vertex[2].y = y2;
	Tri1.vertex[2].z = z2;
	Tri1.vertex[2].u = TexSize;
	Tri1.vertex[2].v = 0;	

	//三角形2
	Tri2.vertex[0].x = x0;
	Tri2.vertex[0].y = y0;
	Tri2.vertex[0].z = z0;
	Tri2.vertex[0].u = 0;
	Tri2.vertex[0].v = TexSize;
 
	Tri2.vertex[1].x = x2;
	Tri2.vertex[1].y = y2;
	Tri2.vertex[1].z = z2;
	Tri2.vertex[1].u = TexSize;
	Tri2.vertex[1].v = 0;

	Tri2.vertex[2].x = x3;
	Tri2.vertex[2].y = y3;
	Tri2.vertex[2].z = z3;
	Tri2.vertex[2].u = TexSize;
	Tri2.vertex[2].v = TexSize;	
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CObject3D::CObject3D(int64 ID,tstring Fingerprint)
:CVSpace3(ID),m_Fingerprint(Fingerprint)
{
	m_TextureNum = 0;
	m_crDefault = RGB(152,127,110);
}
	

CObject3D::~CObject3D(){

};
	
void CObject3D::InitSpace(float dx, float dy, float dz){	
	float  x = dx/2;
	float  y = dy/2;
	float  z = dz/2;
/*
     -xy-z  __________________ xy-z
		   /|                /| 
          / |               / |
         /  |              /  | 
		/   |             /   | 
	   /    |       xyz  /    |
-xyz  /_____|___________/     | 
	  |     |           |     | x-y-z
	  |    / -----------|----/
	  |   / -x-y-z      |   /
	  |  /              |  /
	  | /               | / 
-x-yz |/_______________ |/x-yz

*/
    RECT3D& bottomface = GetSurFace(BOTTOMFACE);
    RectToTri(x,-y,-z,x,-y,z,-x,-y,z,-x,-y,-z,bottomface.triangle[0],bottomface.triangle[1],1);
    
	RECT3D& topface = GetSurFace(TOPFACE);
    RectToTri(-x,y,-z,-x,y,z,x,y,z,x,y,-z,topface.triangle[0],topface.triangle[1],1);

    RECT3D& leftface = GetSurFace(LEFTFACE);
    RectToTri(-x,y,z,-x,-y,z,-x,-y,-z,-x,y,-z,leftface.triangle[0],leftface.triangle[1],1);

    RECT3D& rightface = GetSurFace(RIGHTFACE);
    RectToTri(x,y,z,x,-y,z,x,-y,-z,x,y,-z,rightface.triangle[0],rightface.triangle[1],1);

    RECT3D& frontface = GetSurFace(FRONTFACE);
    RectToTri(-x,y,z,-x,-y,z,x,-y,z,x,y,z,frontface.triangle[0],frontface.triangle[1],1);

    RECT3D& backface = GetSurFace(BACKFACE);
    RectToTri(-x,y,-z,-x,-y,-z,x,-y,-z,x,y,-z,backface.triangle[0],backface.triangle[1],1);


	m_Text.SetTextRect(dx-0.2f,0.1f);
	m_Text.ToPlace(0,0.2f,z+0.01f,0,0,0);

 };

void CObject3D::SetTexture(int TexNum){
     m_TextureNum = TexNum;  
}

void CObject3D::Draw(GLfloat* ParentMatrix){
	glLoadMatrixf(ParentMatrix);      //引入父空间的矩阵变换，
	glMultMatrixf(m_LocalMatrix);     //与本物体的本地变换矩阵相乘
    glGetFloatv(GL_MODELVIEW_MATRIX,ParentMatrix);//得到基于全局坐标的矩阵变换

//	glEnable(GL_CULL_FACE);		//打开背面裁减
//	glCullFace(GL_BACK);

	
	if(m_TextureNum){

		glEnable(GL_TEXTURE_2D);							
	    glBindTexture(GL_TEXTURE_2D, m_TextureNum);
	}
    else{ 
	   glColor3ub(GetRValue(m_crDefault),GetGValue(m_crDefault),GetBValue(m_crDefault));
	}
	
	glColor3ub(0,23,53);
	RECT3D& bottomface = GetSurFace(BOTTOMFACE);
	glNormal3f( 0.0f, -1.0f, 0.0f);
	bottomface.Draw();
	
	glColor3ub(125,23,53);
	RECT3D& topface = GetSurFace(TOPFACE);
	glNormal3f( 0.0f, 1.0f, 0.0f);
	topface.Draw();
    
	glColor3ub(234,0,53);
	RECT3D& leftface = GetSurFace(LEFTFACE);
	glNormal3f( -1.0f, 0.0f, 0.0f);
	leftface.Draw();
	
	glColor3ub(23,124,21);
	RECT3D& rightface = GetSurFace(RIGHTFACE);
	glNormal3f( 1.0f, 0.0f, 0.0f);
	rightface.Draw();
    
	glColor3ub(56,23,0);
	RECT3D& frontface = GetSurFace(FRONTFACE);
	glNormal3f( 0.0f, 0.0f, 1.0f);
	frontface.Draw();
    
	glColor3ub(34,26,127);
	RECT3D& backface = GetSurFace(BACKFACE);
	glNormal3f( 0.0f, 0.0f, -1.0f);
	backface.Draw();

	GLfloat NewParentMatrix[16]; 

	::memcpy(NewParentMatrix,ParentMatrix,sizeof(NewParentMatrix));			
	m_Text.Draw(ParentMatrix);

    glDisable(GL_BLEND);

	if(m_TextureNum){
		glDisable(GL_TEXTURE_2D);
	}else{
		glColor3ub(255,255,255);
	}
//	glDisable(GL_CULL_FACE);
}