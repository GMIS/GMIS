// Space3D.cpp: implementation of the CSpace3D class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4786)

#include "Room3D.h"


CWall::CWall(){
     m_crDefault = RGB(0,128,0);
	 m_TextNum = 0;
};
	
CWall::~CWall(){
    
};

void CWall::SetSize(float dx,float dy){
	float  x = dx/2.0f;
	float  y = dy/2.0f;
    float  z = 0.0f;
    RectToTri(-x,y,z,-x,-y,z,x,-y,z,x,y,z,m_Face.triangle[0],m_Face.triangle[1]);
};
	
void CWall::SetTexture(int TextNum,float TextWidth, float TextHeight){
	if(TextWidth==0 || TextHeight == 0)return;
	m_TextNum = TextNum;
    
	m_Face.SetTextureUnit(TextWidth,TextHeight);
}


void CWall::Draw(GLfloat* ParentMatrix){
	glLoadMatrixf(ParentMatrix);  //引入父空间的矩阵变换，
	glMultMatrixf(m_LocalMatrix);     //得到本物体基于世界坐标的矩阵变换
	
	if(m_TextNum){
		glEnable(GL_TEXTURE_2D);							
	    glBindTexture(GL_TEXTURE_2D, m_TextNum);
	}
    else{ 
	   glColor3ub(GetRValue(m_crDefault),GetGValue(m_crDefault),GetBValue(m_crDefault));
	}
			
	glNormal3f(0.0f,0.0f,1.0f);
	m_Face.Draw();
	if(m_TextNum){
		glDisable(GL_TEXTURE_2D);
	}else{
		glColor3ub(255,255,255);
	}	
}


CDoorWall::CDoorWall(){
	m_crDefault = RGB(128,0,0);
};
CDoorWall::CDoorWall(int64 ID, tstring Name,tstring Fingerprint){
	m_Alias = ID;
	m_Name.SetText(Name);
	m_Fingerprint = Fingerprint;
}		 

CDoorWall::~CDoorWall(){
	
}
	
/*
以门的中心为坐标原点   
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
void CDoorWall::SetSize(float dx,float dy){
	float  x = dx/2;
	float  y = dy/2;
    float  z = 0.0f;

	float  x1 = DOOR_SIZE/2;

    //wall
    RectToTri(-x,y,z,-x,-y,z,-x1,-y,z,-x1,y,z,m_LeftWall.triangle[0],m_LeftWall.triangle[1]);
    RectToTri(x1,y,z,x1,-y,z,x,-y,z,x,y,z,m_RightWall.triangle[0],m_RightWall.triangle[1]);
	
    z=DYPASS_DEPTH;
    //dypass
    RectToTri(-x1,y,0.0f,-x1,-y,0.0f,-x1,-y,-z,-x1,y,-z,m_DypassLeft.triangle[0],m_DypassLeft.triangle[1]);
    RectToTri(x1,y,-z,x1,-y,-z,x1,-y,0.0f,x1,y,0.0f,m_DypassRight.triangle[0],m_DypassRight.triangle[1]);
    RectToTri(x1,y,0.0f,-x1,y,0.0f,-x1,y,-z,x1,y,-z,m_DypassTop.triangle[0],m_DypassTop.triangle[1]);
    RectToTri(x1,-y,-z,-x1,-y,-z,-x1,-y,0.0f,x1,-y,0.0f,m_DypassDown.triangle[0],m_DypassDown.triangle[1]);

	//door
    RectToTri(-x1,y,-z,-x1,-y,-z,x1,-y,-z,x1,y,-z,m_Door.triangle[0],m_Door.triangle[1]);
 
	float w = DOOR_SIZE/4;
	float h = DOOR_SIZE/8;
	m_Name.SetTextRect(w,h);
	m_Name.ToPlace(-x1-0.4,y-0.3,0.05f,0.0f,0.0f,0.0f);
}
	
void CDoorWall::SetTexture(int WallTextNum,int FloorTextNum, int CeilingTextNum, int DoorTextNum){
	m_WallTextNum = WallTextNum;
    m_DoorTextNum = DoorTextNum;
	m_FloorTextNum = FloorTextNum;
	m_CeilingTextNum = CeilingTextNum;
}
		
void CDoorWall::Draw(GLfloat* ParentMatrix){
	glLoadMatrixf(ParentMatrix);  //引入父空间的矩阵变换，
	glMultMatrixf(m_LocalMatrix);     //得到本物体基于世界坐标的矩阵变换
	
	if(m_WallTextNum){
		glEnable(GL_TEXTURE_2D);			
		
		glBindTexture(GL_TEXTURE_2D, m_WallTextNum);		
		glNormal3f(0.0f,0.0f,1.0f);
		m_LeftWall.Draw();
		glNormal3f(0.0f,0.0f,1.0f);
		m_RightWall.Draw();
		glNormal3f(1.0f,0.0f,0.0f);
		m_DypassLeft.Draw();
		glNormal3f(-1.0f,0.0f,0.0f);
		m_DypassRight.Draw();

		glBindTexture(GL_TEXTURE_2D, m_FloorTextNum);		
		glNormal3f(0.0f,1.0f,0.0f);
		m_DypassDown.Draw();	
		glBindTexture(GL_TEXTURE_2D, m_CeilingTextNum);		
		glNormal3f(0.0f,-1.0f,0.0f);
		m_DypassTop.Draw();

		if(!(m_State & SPACE_OPENDOOR)){
			glBindTexture(GL_TEXTURE_2D, m_DoorTextNum);
			glNormal3f(0.0f,0.0f,1.0f);
			m_Door.Draw();
		}

		glDisable(GL_TEXTURE_2D);
	}
    else{ 
	    glColor3ub(GetRValue(m_crDefault),GetGValue(m_crDefault),GetBValue(m_crDefault));

		glNormal3f(0.0f,0.0f,1.0f);
		m_LeftWall.Draw();
		glNormal3f(0.0f,0.0f,1.0f);
		m_RightWall.Draw();
		glNormal3f(1.0f,0.0f,0.0f);
		m_DypassLeft.Draw();
		glNormal3f(-1.0f,0.0f,0.0f);
		m_DypassRight.Draw();

		glNormal3f(0.0f,1.0f,0.0f);
		m_DypassDown.Draw();	
		glBindTexture(GL_TEXTURE_2D, m_CeilingTextNum);		
		glNormal3f(0.0f,-1.0f,0.0f);
		m_DypassTop.Draw();

		if(!(m_State & SPACE_OPENDOOR)){
			glNormal3f(0.0f,0.0f,1.0f);
			m_Door.Draw();
		}
	}

	glGetFloatv(GL_MODELVIEW_MATRIX,ParentMatrix);//得到基于全局坐标的矩阵变换	
	m_Name.Draw(ParentMatrix);
	glColor3ub(255,255,255);

}




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRoom3D::CRoom3D()
{
	m_WallTexture = 0;
	m_FloorTexture = 0;
	m_CeilingTexture = 0;
	m_DoorTexture = 0;
    m_ObjectTexture = 0;

	m_Type = LOCAL_SPACE;

	m_x0 = 0;
	m_y0 = 0;
	m_z0 = 0;

	m_ObjectSize = 0.5f;
	m_ObjectHeight = 1.0f;
	m_ObjectPadding = 0.1f;

	for (int i=0; i<4; i++)
	{
		m_DoorWall[i] = NULL;
	}
}

CRoom3D::~CRoom3D()
{
	ClearAllObject();
}

	
CDoorWall* CRoom3D::Wall2Door(FACEPOS fc){
   CDoorWall* dw = m_DoorWall[fc];
   if(dw)return dw;

    dw = new CDoorWall;
	float  z = m_SpaceDz/2;
	float  x = m_SpaceDx/2;
	m_DoorWall[fc] = dw;
	dw->SetTexture(m_WallTexture,m_FloorTexture,m_CeilingTexture,m_DoorTexture);

	if(fc == FRONTFACE){
		dw->SetSize(m_SpaceDx,m_SpaceDy);
		dw->ToPlace(0,0,z,0,-180.0f,0);
	}else if(fc == LEFTFACE){ 
		dw->SetSize(m_SpaceDz,m_SpaceDy);
		dw->ToPlace(-x,0,0,0,90.0f,0);
	}else if (fc== BACKFACE) { 
		dw->SetSize(m_SpaceDx,m_SpaceDy);
		dw->ToPlace(0,0,-z,0,0,0);
	}else if (fc == RIGHTFACE){ 
		dw->SetSize(m_SpaceDz,m_SpaceDy);
		dw->ToPlace(x,0,0,0,-90.0f,0);
	}
    m_Dypass[fc].Space = dw;
	return dw;

}
	
CWall*     CRoom3D::Door2Wall(FACEPOS fc){
      
	CDoorWall* dw = m_DoorWall[fc];
    if(dw)delete dw;
    m_DoorWall[fc] = NULL;
    m_Dypass[fc].Space = NULL;
	
	return &m_Wall[fc];
}


void CRoom3D::SetSize(float dx, float dy, float dz){

    m_SpaceDx = dx;
	m_SpaceDy = dy;
	m_SpaceDz = dz;
	
	float  x = m_SpaceDx/2.0f;
	float  y = m_SpaceDy/2.0f;
	float  z = m_SpaceDz/2.0f;

	m_Floor.SetSize(m_SpaceDx,m_SpaceDz);
    m_Floor.ToPlace(0,-y,0,-90,0,0);

    m_Ceiling.SetSize(m_SpaceDx,m_SpaceDz);
    m_Ceiling.ToPlace(0,y,0,90,0,0);

  	CWall& LeftWall = m_Wall[LEFTFACE];
	
	LeftWall.SetSize(m_SpaceDz,m_SpaceDy);
    LeftWall.ToPlace(-x,0,0,0,-90.0f,0);
    LeftWall.m_crDefault = RGB(128,128,50);

	CWall& RightWall = m_Wall[RIGHTFACE];
	RightWall.SetSize(m_SpaceDz,m_SpaceDy);
    RightWall.ToPlace(x,0,0,0,90.0f,0);
    RightWall.m_crDefault = RGB(128,128,100);

	CWall& FrontWall = m_Wall[FRONTFACE];
	FrontWall.SetSize(m_SpaceDx,m_SpaceDy);
    FrontWall.ToPlace(0,0,z,0,0,0);
    FrontWall.m_crDefault = RGB(128,128,150);

	CWall& BackWall = m_Wall[BACKFACE];
	BackWall.SetSize(m_SpaceDx,m_SpaceDy);
    BackWall.ToPlace(0,0,-z,0,-180.0f,0);
    BackWall.m_crDefault = RGB(128,128,200);	

	InitMap(m_SpaceDx,m_SpaceDz);

	for (int i=0; i<4; i++)
	{
		CDoorWall* dw = m_DoorWall[i];
		if(dw){
			if(i == FRONTFACE){
				dw->SetSize(m_SpaceDx,m_SpaceDy);
				dw->ToPlace(0,0,z,0,-180.0f,0);
			}else if(i == LEFTFACE){ 
				dw->SetSize(m_SpaceDz,m_SpaceDy);
				dw->ToPlace(-x,0,0,0,90.0f,0);
			}else if (i== BACKFACE) { 
				dw->SetSize(m_SpaceDx,m_SpaceDy);
				dw->ToPlace(0,0,-z,0,0,0);
			}else if (i == RIGHTFACE){ 
				dw->SetSize(m_SpaceDz,m_SpaceDy);
				dw->ToPlace(x,0,0,0,-90.0f,0);
			}
		}
	}
};
	
bool CRoom3D::IsValidSpace(int64 SpaceID){
	if(m_Alias == SpaceID && m_DataValid)return true;
	return false;
};
	
mapunit*  CRoom3D::IsInDypassRect(float x,float z){
 	x = x-m_x0;
    z = z-m_z0;
    
	mapunit* m = NULL;
	for (int i=0; i<4; i++)
	{
		m = &m_Dypass[i];
        if(x >m->x1 && x<m->x2 && z>m->z1 && z<m->z2)return m; 
	}
    return NULL;
}
	
mapunit*  CRoom3D::IsInObjectRect(float x, float z){
 	x = x-m_x0;
    z = z-m_z0;
    
	mapunit* m = NULL;
	for (int i=0; i<m_MapList.size(); i++)
	{
		m = &m_MapList[i];
        if(m->Space && x >m->x1 && x<m->x2 && z>m->z1 && z<m->z2)return m; 
	}
    return NULL;
}

mapunit* CRoom3D::GetOneBlankMapUnit(){
    vector<mapunit>::iterator It = m_MapList.begin();
	while (It != m_MapList.end())
	{
		mapunit& M = *It;
		if(M.Space == NULL)return &M;
		It++;
	}
	return NULL;
};

bool CRoom3D::AllowInsertSpace(){
	for (int i=1; i<4; i++)
	{
		if(m_DoorWall[i] == NULL)return true;;
	}	
	return false;
}

void  CRoom3D::SetParentSpace(FACEPOS fc,int64 ID,tstring ParentName,SPACETYPE Type){

    mapunit* mp = GetParentSpace();
	if (mp)
	{
		if(mp->face != fc){
			Door2Wall(mp->face);
		}else{
			CDoorWall* dw = (CDoorWall*)mp->Space;
			dw->m_Alias = ID;
			dw->SetName(ParentName);
			return;
		}
	}

    CDoorWall* dw = Wall2Door(fc);
    dw->m_Alias = ID;
	dw->SetName(ParentName);

	float  x = m_SpaceDx/2;
	float  z = m_SpaceDz/2;

	if(fc == FRONTFACE){
		dw->SetSize(m_SpaceDx,m_SpaceDy);
		dw->ToPlace(0,0,z,0,-180.0f,0);
	}else if(fc == LEFTFACE){ 
		dw->SetSize(m_SpaceDz,m_SpaceDy);
		dw->ToPlace(-x,0,0,0,90.0f,0);
	}else if (fc== BACKFACE) { 
		dw->SetSize(m_SpaceDx,m_SpaceDy);
		dw->ToPlace(0,0,-z,0,0,0);
	}else if (fc == RIGHTFACE){ 
		dw->SetSize(m_SpaceDz,m_SpaceDy);
		dw->ToPlace(x,0,0,0,-90.0f,0);
	}

	dw->m_State |= SPACE_PARENT;
}

mapunit*  CRoom3D::GetParentSpace(){
	for (int i=0; i<4; i++)
	{
		mapunit& m = m_Dypass[i];
		if(m.Space && m.Space->m_State & SPACE_PARENT)return &m;
	}
	return NULL;
}
	
int CRoom3D::AddChildSpace(int64 ID,tstring Name, SPACETYPE Type,tstring Fingerprint){
	int index=-1;
	CDoorWall* dw;
	for (int i=0; i<4; i++)
	{
		dw = m_DoorWall[i];
		if(dw == NULL){
			index = i;
			break;
		}
	}
	
	if(index==-1)return -1;
   
	dw = new CDoorWall(ID,Name,Fingerprint);
    dw->SetTexture(m_WallTexture,m_FloorTexture,m_CeilingTexture,m_DoorTexture);
	
	float  x = m_SpaceDx/2;
	float  z = m_SpaceDz/2;

	if(index == FRONTFACE){
		dw->SetSize(m_SpaceDx,m_SpaceDy);
		dw->ToPlace(0,0,z,0,-180.0f,0);
	}else if(index == LEFTFACE){ 
		dw->SetSize(m_SpaceDz,m_SpaceDy);
		dw->ToPlace(-x,0,0,0,90.0f,0);
	}else if (index== BACKFACE) { 
		dw->SetSize(m_SpaceDx,m_SpaceDy);
		dw->ToPlace(0,0,-z,0,0,0);
	}else if (index == RIGHTFACE){ 
		dw->SetSize(m_SpaceDz,m_SpaceDy);
		dw->ToPlace(x,0,0,0,-90.0f,0);
	}

	m_DoorWall[index] = dw;
	m_Dypass[index].Space = dw; 

	return index;
}
	
int  CRoom3D::AddChildSpace(FACEPOS fc,int64 ID,tstring Name,tstring Fingerprint){
	CDoorWall* dw = m_DoorWall[fc];
	assert(dw==NULL);
	   
	dw = new CDoorWall(ID,Name,Fingerprint);
    dw->SetTexture(m_WallTexture,m_FloorTexture,m_CeilingTexture,m_DoorTexture);
	
	float  x = m_SpaceDx/2;
	float  z = m_SpaceDz/2;

	if(fc == FRONTFACE){
		dw->SetSize(m_SpaceDx,m_SpaceDy);
		dw->ToPlace(0,0,z,0,-180.0f,0);
	}else if(fc == LEFTFACE){ 
		dw->SetSize(m_SpaceDz,m_SpaceDy);
		dw->ToPlace(-x,0,0,0,90.0f,0);
	}else if (fc== BACKFACE) { 
		dw->SetSize(m_SpaceDx,m_SpaceDy);
		dw->ToPlace(0,0,-z,0,0,0);
	}else if (fc == RIGHTFACE){ 
		dw->SetSize(m_SpaceDz,m_SpaceDy);
		dw->ToPlace(x,0,0,0,-90.0f,0);
	}

	m_DoorWall[fc] = dw;
	m_Dypass[fc].Space = dw; 

	return fc;
}
	
mapunit* CRoom3D::AddChildObject(int64 ID,tstring Name,SPACETYPE Type,HICON hIcon,tstring Fingerprint){
	mapunit* mp = GetOneBlankMapUnit();
	if(mp==NULL)return NULL;

	CObject3D* ob = new CObject3D(0,Fingerprint);
	if(ob==NULL){
		return NULL;
	}
	ob->m_Alias = ID;
	ob->m_Text.SetText(Name.c_str());
    ob->SetTexture(m_ObjectTexture);

	ob->InitSpace(m_ObjectSize,m_ObjectHeight,m_ObjectSize);
	
    //PushChild(ob);
	float cgx = mp->x1 + (mp->x2-mp->x1)/2;
	float cgy = -m_SpaceDy/2+m_ObjectHeight/2;
	float cgz = mp->z1 + (mp->z2-mp->z1)/2;

	if(mp->face == LEFTFACE)ob->ToPlace(cgx,cgy,cgz,0,90.0f,0);
	else if(mp->face == RIGHTFACE)ob->ToPlace(cgx,cgy,cgz,0,-90.0f,0);
	else if(mp->face == FRONTFACE)ob->ToPlace(cgx,cgy,cgz,0,180.0f,0);
	else ob->ToPlace(cgx,cgy,cgz,0,0,0);
    
	mp->Space = ob;
	return mp;	
}
	
//真正的删除是在ObjectView中完成
void CRoom3D::DeleteChildObject(tstring Fingerprint){
	int index=-1;
	CDoorWall* dw;
	for (int i=0; i<4; i++)
	{
		dw = m_DoorWall[i];
		if(dw && dw->m_Fingerprint ==  Fingerprint){
			delete dw;
			m_DoorWall[i]=NULL;
			m_Dypass[i].Space = NULL;
            return ;
		};
	}
	
	vector<mapunit>::iterator It = m_MapList.begin();
	while (It != m_MapList.end())
	{
		mapunit& M = *It;
		CObject3D* ob = (CObject3D*)M.Space;
		if(ob && ob->m_Fingerprint == Fingerprint){
			delete ob;
			M.Space = NULL;
		}
		It++;
	};
}
    
void CRoom3D::ClearAllObject(){
    vector<mapunit>::iterator It = m_MapList.begin();
	while (It != m_MapList.end())
	{
		mapunit& M = *It;
		if(M.Space){
			delete M.Space;
			M.Space = NULL;
		}
		It++;
	};
	
	for (int i=0; i<4; i++)
	{
		CDoorWall* Door =  m_DoorWall[i];
		if(Door && !(Door->m_State & SPACE_PARENT)){
			delete Door;
			m_DoorWall[i] = NULL;

			mapunit& m = m_Dypass[i];
			if (m.Space)
			{
				//delete m.Space;  
				m.Space = NULL;   //m.Space=Door 已经被删除，这里只是避免再次被删除
			}
		}
	}

}

void   CRoom3D::Draw(GLfloat* ParentMatrix){

	glLoadMatrixf(ParentMatrix);      //引入父空间的矩阵变换，
	glMultMatrixf(m_LocalMatrix);     //得到本物体基于世界坐标的矩阵变换
	glGetFloatv(GL_MODELVIEW_MATRIX,ParentMatrix);

	GLfloat NewParentMatrix[16];

	::memcpy(NewParentMatrix,ParentMatrix,sizeof(NewParentMatrix));

    m_Floor.Draw(NewParentMatrix);

	::memcpy(NewParentMatrix,ParentMatrix,sizeof(NewParentMatrix));
	m_Ceiling.Draw(NewParentMatrix);
	
	for (int i=0; i<4; i++)
	{
		CWall& w = m_Wall[i];
		CDoorWall* dw = m_DoorWall[i];
		if(dw){ //有门就不画缺省的墙
			::memcpy(NewParentMatrix,ParentMatrix,sizeof(NewParentMatrix));		
			dw->Draw(NewParentMatrix);
		}else{
            ::memcpy(NewParentMatrix,ParentMatrix,sizeof(NewParentMatrix));		
			w.Draw(NewParentMatrix);
		}
	};


    vector<mapunit>::iterator It = m_MapList.begin();
	while (It != m_MapList.end())
	{
		mapunit& M = *It;
		if(M.Space != NULL){
			CObject3D* Ob = (CObject3D*)M.Space;
			::memcpy(NewParentMatrix,ParentMatrix,sizeof(NewParentMatrix));
			Ob->Draw(NewParentMatrix);
		}
		It++;
	};	
}

void CRoom3D::InitMap(float dx,float dz){
	float len = m_ObjectSize+m_ObjectPadding;
    float x = dx/2;
	float z = dz/2;
	int n1= dx/len;
	int n2= dz/len;

	float xd1 = -x+(dx-DOOR_SIZE)/2;
	float xd2 = xd1+DOOR_SIZE;

	float zd1 = -z+(dz-DOOR_SIZE)/2;
	float zd2 = zd1+DOOR_SIZE;

/*                        /
                   xd1   /  xd2
             n2 /---/---/---/---/---/
	           /---/---/---/---/---/ zd1
	    ------/---/---/---/---/---/------ x
	         /---/---/---/---/---/ zd2
	        /---/---/---/---/---/
       (-x,z)     /           n1
                  / 
                 z
*/
    mapunit m;
    
	for (int i=0; i<n1; i++)
	{
		for (int k=0; k<n2; k++)
		{

			if(i>0 && i<n1-1 && k>0 && k<n2-1)continue;  //把房间分成n1*n2的格，中间的不要；
			if((i==0 || i==n1-1) && (k==0 || k==n2 -1))continue;//四个角也不要
			m.x1 = -x+i*len;
			m.x2 = m.x1+len;
			m.z2 = z-k*len;
			m.z1 = m.z2-len;
            m.Space = NULL;
			
	
			if(i==0){
				//为四面墙的门预留出空位
				if( (m.z1 > zd1 && m.z1<zd2) || (m.z2> zd1 && m.z2< zd2 ))continue;  
				m.face = LEFTFACE;
			}
			else if(i==n1-1){
				//为四面墙的门预留出空位
				if( (m.z1 > zd1 && m.z1<zd2) || (m.z2> zd1 && m.z2< zd2 ))continue;  
				m.face=RIGHTFACE;
			}
			else if(k==0){
				if((m.x1>xd1 && m.x1<xd2) || (m.x2>xd1 && m.x2<xd2))continue;
				m.face = FRONTFACE;
			}
			else{  //k==n2-1
				if((m.x1>xd1 && m.x1<xd2) || (m.x2>xd1 && m.x2<xd2))continue;
				m.face = BACKFACE;
			}

			m_MapList.push_back(m);
		}
	}

	//预置门廊坐标
	m_Dypass[FRONTFACE].x1 = xd1;
	m_Dypass[FRONTFACE].x2 = xd2;
	m_Dypass[FRONTFACE].z1 = z-1.0f; //增加碰撞检测面积，
	m_Dypass[FRONTFACE].z2 = z+DYPASS_DEPTH;
    m_Dypass[FRONTFACE].face = FRONTFACE;
	m_Dypass[FRONTFACE].Space = NULL;

	m_Dypass[BACKFACE].x1 = xd1;
	m_Dypass[BACKFACE].x2 = xd2;
	m_Dypass[BACKFACE].z1 = -z-DYPASS_DEPTH;
	m_Dypass[BACKFACE].z2 = -z+1.0f;
    m_Dypass[BACKFACE].face = BACKFACE;
	m_Dypass[BACKFACE].Space = NULL;


	m_Dypass[LEFTFACE].x1 = -x-DYPASS_DEPTH;
	m_Dypass[LEFTFACE].x2 = -x+1.0f;
	m_Dypass[LEFTFACE].z1 = zd1;
	m_Dypass[LEFTFACE].z2 = zd2;
    m_Dypass[LEFTFACE].face = LEFTFACE;
	m_Dypass[LEFTFACE].Space = NULL;

	m_Dypass[RIGHTFACE].x1 = x-1.0f;
	m_Dypass[RIGHTFACE].x2 = x+DYPASS_DEPTH;
	m_Dypass[RIGHTFACE].z1 = zd1;
	m_Dypass[RIGHTFACE].z2 = zd2;
    m_Dypass[RIGHTFACE].face = RIGHTFACE;
	m_Dypass[RIGHTFACE].Space = NULL;	
};

/*
原则：子空间以自身中心或重心为原点设置坐标，父空间则负责设置子空间在父空间的位置和方向

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
	

void CRoom3D::InitTexture(GLuint WalltexNum, GLuint FloortexNum, GLuint CeilingtexNum, GLuint DoortexNum,GLuint ObjecttexNum){
	m_WallTexture    = WalltexNum;
	m_FloorTexture   = FloortexNum;
	m_CeilingTexture = CeilingtexNum;
	m_DoorTexture    = DoortexNum;
    m_ObjectTexture  = ObjecttexNum;

	m_Floor.SetTexture(m_FloorTexture,6,6);
	m_Ceiling.SetTexture(m_CeilingTexture,6,6);

	int i;
	for (i=0; i<4; i++)
	{
		CWall& w = m_Wall[i];
		w.SetTexture(m_WallTexture,6,1);
	};
	
	for (i=0; i<4; i++)
	{
		CDoorWall* w = m_DoorWall[i];
		if(w)w->SetTexture(m_WallTexture,m_FloorTexture,m_CeilingTexture,m_DoorTexture);
	}
}
 
