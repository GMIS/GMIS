// SpaceView.cpp: implementation of the CSpaceView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)


#include "VisibleSpace.h"
#include <algorithm>
#include <map>

namespace VISUALSPACE{
		

CVSpace2::CVSpace2():
m_Alias(0),
m_State(0),
m_Parent(NULL)
{
	m_SizeX =0; m_SizeY = 0;
	m_AreaLeft =0;
	m_AreaRight = 0;
	m_AreaTop = 0;
	m_AreaBottom = 0;
};	
CVSpace2::CVSpace2(int64 ID):
m_Alias(ID),
m_State(0),
m_Parent(NULL)
{
	m_SizeX =0; m_SizeY = 0;
	m_AreaLeft =0;
	m_AreaRight = 0;
	m_AreaTop = 0;
	m_AreaBottom = 0;
};	

CVSpace2::CVSpace2(int64 ID,CVSpace2* Parent):
m_Alias(ID),
m_State(0),
m_Parent(Parent)
{
	m_SizeX =0; m_SizeY = 0;
	m_AreaLeft =0;
	m_AreaRight = 0;
	m_AreaTop = 0;
	m_AreaBottom = 0;

}
CVSpace2::~CVSpace2()
{
	DeleteAll();
};

typedef deque<CVSpace2*>::iterator         SpaceIt;	
typedef deque<CVSpace2*>::reverse_iterator SpaceRIt;	

CVSpace2* CVSpace2::FindSpace(int64 ID,bool rBegin /*=false*/){//正向找
	if(rBegin){
		SpaceRIt It = m_ChildList.rbegin();
		while(It<m_ChildList.rend()){
			CVSpace2* Space = (*It);
			if(Space->m_Alias == ID){
				return Space;
			}else{		
				Space = Space->FindSpace(ID,rBegin);
				if(Space)return Space;
			} 
			It++;
		}
		return NULL;
	}
	SpaceIt It = m_ChildList.begin();
	while(It<m_ChildList.end()){
		CVSpace2* Space = (*It);
		if(Space->m_Alias == ID){
			return Space;
		}else{		
			Space = Space->FindSpace(ID,rBegin);
			if(Space)return Space;
		} 
		It++;
	}
	return NULL;
}   

CVSpace2* CVSpace2::FindeChild(ePipeline& Address,CVSpace2* Parent)
{
	int64 ID = Address.PopInt();
    
	if (Parent == NULL)
	{
		return NULL; 		
	}
	
    deque<CVSpace2*>::iterator it = Parent->m_ChildList.begin();
	while(it != Parent->m_ChildList.end())
	{
		CVSpace2* Child = *it;
		if (Child->m_Alias == ID)
		{
			if (Address.Size())
			{
				Child = FindeChild(Address,Child);
				return Child;
			}else{
				return Child;
			}
		}
		
		it++;
	}
	return NULL;
}

void CVSpace2::DeleteAll(){
	if(m_ChildList.size()==0)return;
	SpaceIt It = m_ChildList.begin();
    SpaceIt ItEnd = m_ChildList.end();
	while(It != ItEnd){
		CVSpace2* Space = * It;
		if(Space)delete Space;
		*It = NULL;
		It++;
	}
	m_ChildList.clear();
}

CVSpace2*  CVSpace2::HitTest(int32 x,int32 y){	
	POINT point;
	point.x = x;
	point.y = y;
	RECT rc = GetArea();
	if(!::PtInRect(&rc,point))return NULL;	
	CVSpace2* Ret = NULL;
	deque<CVSpace2*>::iterator It = m_ChildList.begin();
	while(It != m_ChildList.end()){
		Ret = (*It)->HitTest(x,y); 	
		if(Ret)return Ret;
		It++;
	}
	return this;
}		

CVSpace2* CVSpace2::RemoveChild(CVSpace2* Space){
	deque<CVSpace2*>::iterator It = find(m_ChildList.begin(),m_ChildList.end(),Space);
    if(It != m_ChildList.end())m_ChildList.erase(It);
	return Space;
}

CVSpace2* CVSpace2::PushChild(CVSpace2* Space, int32 pos){
	assert(Space!=NULL);
	Space->m_Parent =this;
	if(pos == -1){
		m_ChildList.push_back(Space);
		return Space;
	}
	if((uint32)pos<=m_ChildList.size()){
		m_ChildList.insert(m_ChildList.begin()+pos,Space);
		return Space;
	}
	else return NULL;
};



	
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVSpace3::CVSpace3()
{
 	LocalIdentity();
}
	
CVSpace3::CVSpace3(int64 ID)
:CVSpace2(ID){
	LocalIdentity();
}

CVSpace3::~CVSpace3()
{
};
   
void CVSpace3::ToPlace(float32 transx, float32 transy, float32 transz,float32 rotx, float32 roty, float32 rotz){

    glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
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
	//暂存运算结果
	glGetFloatv(GL_MODELVIEW_MATRIX,m_LocalMatrix);
    glPopMatrix();
};


}//namespace VISUALSPACE
