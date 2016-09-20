/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _ROOM3D_H__
#define _ROOM3D_H__

#include "Space.h"
#include "Object3D.h"
#include "VisibleSpace.h"

#define  DOOR_SIZE  1.5f
#define  DYPASS_DEPTH 0.4f


class CWall: public CVSpace3 
{
public:
	RECT3D     m_Face;
	COLORREF   m_crDefault;
	GLuint     m_TextNum;
public:
	CWall();
	~CWall();

	void SetTexture(int TextNum,float TextWidth, float TextHeight);    
	void SetSize(float dx,float dy);
	virtual void Draw(GLfloat* ParentMatrix);
};


/*
显示一个中间带门的墙   
                              x,y
                     / \      /|
					/    \   / | <-rightwall
                   /       \/  | 
        dypss ->  | \      /|  |
				  |   \   / |  / x,-y
                  |     \/  | /
				   \    /|  |/
				     \ / |  / x1,-y
				      /\ | /<- door
                     /  \|/
			  -x, y	/    / -x1,-y
                    |   /
					|  /
					| / <-leftwall
			  -x,-y |/
*/

#define  SPACE_OPENDOOR  0x00000001
#define  SPACE_PARENT    0x00000002
class CDoorWall: public CVSpace3
{
protected:
    RECT3D     m_LeftWall;
	RECT3D     m_RightWall;
	RECT3D     m_DypassLeft;
	RECT3D     m_DypassRight;
	RECT3D     m_DypassTop;
	RECT3D     m_DypassDown;
	RECT3D     m_Door;

	COLORREF   m_crDefault;
	GLuint     m_WallTextNum;
	GLuint     m_DoorTextNum;
	GLuint     m_FloorTextNum;
	GLuint     m_CeilingTextNum;

public:
	CText3DEN  m_Name;  //房间名
	tstring    m_Fingerprint;
public:
	CDoorWall();
	CDoorWall(int64 ID, tstring Name,tstring Fingerprint);
	~CDoorWall();

	void SetName(tstring text){m_Name.SetText(text);};
	tstring GetName(){ return m_Name.m_Text;};
	void SetSize(float dx,float dy);
	void SetTexture(int WallTextNum,int FloorTextNum, int CeilingTextNum, int DoorTextNum);
	void Draw(GLfloat* ParentMatrix);
	
};

class mapunit 
{
public:
	float    x1;
	float    x2;
	float    z1;
	float    z2;
	FACEPOS  face;
	CVSpace3*  Space;
	BOOL     ForbidMove;
public:
	mapunit(){
		x1=0; x2=0;
		z1=0; z2=0;
		Space = NULL;
		ForbidMove = false;
	}
	~mapunit(){
		if (Space)
		{
			delete Space;
			Space = NULL;
		}
	}
	mapunit(const mapunit& other){
		x1 = other.x1;
		x2 = other.x2;
		z1 = other.z1;
		z2 = other.z2;
		face = other.face;
		Space = other.Space;
        ForbidMove = other.ForbidMove;
	}
	mapunit& operator= (const mapunit& other){
		x1 = other.x1;
		x2 = other.x2;
		z1 = other.z1;
		z2 = other.z2;
		face = other.face;
		Space = other.Space;
        ForbidMove = other.ForbidMove;
		return *this;
	}

	bool InRect(float x, float z){
		if(x>x1 && x<x2 && z>z1 && z<z2)return true;
		return false;
	}
	
};

class CRoom3D : public CVSpace3  
{
public:
	tstring     m_Name;
	bool        m_DataValid;
	
	SPACETYPE   m_Type;
	tstring     m_Fingerprint;

	float      m_SpaceDx; //default = 6
	float      m_SpaceDy; //default = 1 
	float      m_SpaceDz; //default = 6
	
	/*此房间位于世界坐标的摆放原点
	  世界坐标通过此摆放点转换成基于自身原点的本地坐标，用于碰撞检测
	*/
	float      m_x0;  
	float      m_y0;
	float      m_z0;

	CWall        m_Floor;
	CWall        m_Ceiling;

    CWall        m_Wall[4];
    
	CDoorWall*   m_DoorWall[4]; //如果有则取代对应的wall

	GLuint	   m_WallTexture;
	GLuint     m_FloorTexture;
	GLuint     m_CeilingTexture;
	GLuint     m_DoorTexture;
    GLuint     m_ObjectTexture;

	float      m_ObjectSize; //底面是正方形
	float      m_ObjectHeight;

	float      m_ObjectPadding;
	

	vector<mapunit>  m_MapList;
    mapunit          m_Dypass[4];

protected:

	void InitMap(float dx,float dz);
    
	CDoorWall* Wall2Door(FACEPOS fc);
	CWall*     Door2Wall(FACEPOS fc);  

public:
	CRoom3D();
	virtual ~CRoom3D();
    
	bool IsValidSpace(int64 SpaceID);

	//指定的点（世界坐标）被转换成本地坐标后，检查是否位于相应矩形内
	mapunit*  IsInDypassRect(float x,float z);
	mapunit*  IsInObjectRect(float x, float z);
	
	virtual void  ToPlace(float transx, float transy, float transz,float rotx, float roty, float rotz){
           m_x0 = transx;
		   m_y0 = transy;
		   m_z0 = transz;
		   CVSpace3::ToPlace(transx,transy,transz,rotx,roty,rotz);
	}
	
	int32 GetMapUintSize(){ return m_MapList.size();};
    bool  AllowInsertSpace(); //检查是否还有空位可以插入新的房间
    mapunit*  GetOneBlankMapUnit();

	void  SetParentSpace(FACEPOS fc,int64 ID,tstring ParentName,SPACETYPE Type);
    mapunit*  GetParentSpace(); //没有返回NULL;

	//如果返回-1则表示没有空位
	int  AddChildSpace(int64 ID,tstring Name, SPACETYPE Type,tstring Fingerprint);
	int  AddChildSpace(FACEPOS fc,int64 ID,tstring Name,tstring Fingerprint);
	mapunit*  AddChildObject(int64 ID,tstring Name,SPACETYPE Type,HICON hIcon,tstring Fingerprint);
	void DeleteChildObject(tstring Fingerprint);
    void ClearAllObject();  //注意：不包括可能含有的通向父空间的门

 	void SetSize(float dx, float dy, float dz); 
	
	void InitTexture(GLuint WalltexNum, GLuint FloortexNum, GLuint CeilingtexNum, GLuint DoortexNum,GLuint ObjecttexNum);
	virtual	void Draw(GLfloat* ParentMatrix);

};

#endif // _ROOM3D_H__
