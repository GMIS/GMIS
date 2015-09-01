/*
*
*这是一个MINI GUI Framework,与其他GUI Framework不同的是，它不直接提供丰富的GUI控件
*给你用，而是给出一个标准的路线图，让程序员能在非常短的时间内实现自己想要的任何控件。
*
*首先把任何显示元素想象成一个空间，那么内部的显示元素就是其子空间，所有空间都具备以下
*特点，并用四个虚函数来表示
*    Draw()         //呈现几何效果
*    HitTest()      //决定力学传导
*    Layout()       //决定内部结构
*
*窗口对于其父窗口也是一个显示元素，具备上述特点，但窗口作为具有活性的空间，又具有其他
*特点：
*
*    Rection()      //决定内部反应
*
*对于任何想要控件，只要我们能把上述虚函数实现自己的版本，就能轻松完成，并且2D和3D能遵守
*统一思想，有时这比学习一个现成的陌生控件所需成本更低。
*
*值得一提的是控件的坐标体系，有两种选择，一种所有空间的坐标都基于窗口坐标，另一种是所有
*空间坐标都基于其父空间。前者是目前的常用方法，后者从长远看，方便显示元素的独立产生。
*
*目前，同时提供两种坐标的处理，使用时必须指定其中一种，不能混合使用。
*
*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _VISIBLESPACE_H__
#define _VISIBLESPACE_H__


#pragma warning(disable : 4786)

#include "Pipeline.h"
#include <windows.h>
#include <deque>
#include <string>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
//#include <gl\glaux.h>		// Header File For The Glaux Library

using namespace std;

namespace VISUALSPACE{


//用户可以定义自己的状态，但通常保留以下几个
#define SPACE_SELECTED   0x00000001
#define SPACE_FOCUSED    0x00000002 
#define SPACE_ACTIVATE   0x00000004 
#define SPACE_WARNING    0x00000008
#define SPACE_DISABLE    0x00000010

//visible space 
class  CVSpace2 
{
public:
	//RECT		      m_Area;        不使用现成结构以利于后续3D承继  
    int32             m_AreaLeft;    //基于父空间的位置坐标（而不是显示窗口，这意味着真正绘制时必须转换成窗口坐标）
    int32             m_AreaTop;
	int32             m_AreaRight;
	int32             m_AreaBottom;
	
	//SIZE            m_Size;        
	int32             m_SizeX;       //实际大小，有时m_SizeX并不等于m_AreaRight-m_AreaLeft，比如在绘制一棵树时，
	int32             m_SizeY;       //自身节点绘制依照m_Area，但size却包括所有子节点构成的总面积
 
	int64             m_Alias;       //数字识别ID

	uint32            m_State;       //状态
	
	CVSpace2*	      m_Parent;
	deque<CVSpace2*>  m_ChildList;
	
public:
	CVSpace2();
	CVSpace2(int64 ID);
	CVSpace2(int64 ID,CVSpace2* Parent);
	virtual ~CVSpace2();
	
	RECT  GetArea(){
		RECT rc;
		rc.left = m_AreaLeft; rc.top = m_AreaTop; rc.right = m_AreaRight; rc.bottom = m_AreaBottom;
		return rc;
	};

	RECT  GetArea(ePipeline* Pipe){
   	   	RECT rc ;
		int32 x = *(int32*)Pipe->GetData(0);
		int32 y = *(int32*)Pipe->GetData(1);
		//转换成基于窗口的坐标
		rc.left   = m_AreaLeft   - x;
		rc.top    = m_AreaTop    - y;
		rc.right  = m_AreaRight  - x;
		rc.bottom = m_AreaBottom - y;
		return rc;
	};

	void         SetArea(CVSpace2* Space,bool bLayout=false){
		                 m_AreaTop=Space->m_AreaTop;m_AreaLeft=Space->m_AreaLeft;
	                     m_AreaRight=Space->m_AreaRight;m_AreaBottom=Space->m_AreaBottom; 
	}
	void         SetArea(const RECT& rc){
		                 m_AreaTop=rc.top;m_AreaLeft=rc.left;m_AreaRight=rc.right;m_AreaBottom=rc.bottom;
	};
	void         SetArea(int32 left,int32 top,int32 right,int32 bottom){
		                 m_AreaTop=top;m_AreaLeft=left;m_AreaRight=right;m_AreaBottom=bottom;
	};
	void         SetAreaSize(int32 w,int32 h){
		                 m_AreaRight=m_AreaLeft+w; m_AreaBottom=m_AreaTop+h;
	};
	void         DeflateArea(int32 left,int32 top, int32 right,int32 bottom){
	                     m_AreaLeft+=left; m_AreaRight-=right; m_AreaTop+=top; m_AreaBottom-=bottom;	                
	};

    int32        AreaHeight(){ return m_AreaBottom-m_AreaTop;};
	int32        AreaWidth(){return m_AreaRight-m_AreaLeft;};

    SIZE         GetSize(){ SIZE s; s.cx = m_SizeX,s.cy=m_SizeY;return s;};
    void         SetSize(int32 sx, int32 sy){ m_SizeX = sx, m_SizeY = sy;};

	CVSpace2*    PushChild(CVSpace2* Space,int32 Pos=-1); //-1表示push_back,成功返回
	CVSpace2*    FindSpace(int64 ID,bool rBegin=false);   //包括子空间, rBegin=true则从后面往前找		
	CVSpace2*    RemoveChild(CVSpace2* Space); //注意：没有Delete 
	void         DeleteAll();

	static CVSpace2* FindeChild(ePipeline& Address,CVSpace2* Parent);

	virtual CVSpace2*  HitTest(int32 x,int32 y);
	virtual void       Layout(bool Redraw = true){};
	
	//Pipe提供一种动态参数绘制的可能
	virtual void    Draw(HDC hDC,ePipeline* Pipe = NULL){
		for(uint32 i=0; i<m_ChildList.size(); i++)m_ChildList[i]->Draw(hDC,Pipe);
	}
		
	virtual void    Draw3D(float32* ParentMatrix,ePipeline* Pipe = NULL){
		return; //这个版本没有意义
	}

	
	//实验,Pipe提供观察者也就是绘制窗口的坐标体系
	//绘制自身和绘制包含子物体有时候并不相等，因此需要分开来实现
	///////
	virtual void    DrawSelf(HDC hDC,ePipeline* Pipe){
	 /*示例
	   	RECT rc = GetArea(); 
		
		转换成基于窗口的坐标
		rc.left   = m_AreaLeft   - *(int32*)Pipe->GetData(0);
		rc.top    = m_AreaTop    - *(int32*)Pipe->GetData(1);
		rc.right  = m_AreaRight  - *(int32*)Pipe->GetData(2);
		rc.bottom = m_AreaBottom - *(int32*)Pipe->GetData(3);

		FillRect(hDC,rc,RGB(1,1,1));
       
	 */
	};

	/*
		假设在某个窗口（其也是一个CVSpace2）的开始处理WM_PAINT：
		RECT rcClient;
		::GetClientRect(m_hWnd,&rcClient);

		int32 y = GetVScrollPos();
		int32 x = GetHScrollPos();

		//转换坐标基于文档
		rcClient.left   +=x;
		rcClient.right  +=x;
		rcClient.top    +=y;
		rcClient.bottom +=y;

		ePipeline Pipe;
		Pipe.PushInt32(rcClient.left)
		Pipe.PushInt32(rcClient.top)
		Pipe.PushInt32(rcClient.right)
		Pipe.PushInt32(rcClient.bottom)

		DrawChild(this,DCMem,&Pipe);
		
	*/
	virtual void      DrawChild(HDC hDC,ePipeline* Pipe){
    /*
		RECT rcClient;  基于Space的显示窗口的坐标，
		rcClient.left   = *(int32*)Pipe->GetData(0);
		rcClient.top    = *(int32*)Pipe->GetData(1);
		rcClient.right  = *(int32*)Pipe->GetData(2);
		rcClient.bottom = *(int32*)Pipe->GetData(3);

		vector<CVSpace2*>::iterator It = Parent->m_ChildList.begin();

		while(It<m_ChildList.end()){
			CVSpace2*  Space = *It;
			assert(Space);
		    检查坐标，位于视口之外的不需要位置
			if (Space->m_AreaBottom<rcClient.top || Space->m_AreaRight<rcClient.left)
			{
				It++;
			
				continue;
			}
			if (Space->m_AreaTop>rcClient.bottom || Space->m_AreaLeft > rcClient.right)
			{
				return;
			}
            先绘制自身
			Space->DrawSelf(hDC,Pipe);
			
			if(Space->m_ChildList.size()){ 有子物体则递归绘制子物体
			    ePipeline ChildPipe  绘制前，视口坐标转换成基于Space的
				ChildPipe.PushInt32(rcClient.left + Space->m_AreaLeft);
				ChildPipe.PushInt32(rcClient.top  + Space->m_AreaTop);
				ChildPipe.PushInt32(rcClient.right  + Space->m_AreaLeft);
				ChildPipe.PushInt32(rcClient.bottom + Space->m_AreaTop);
				DrawChild(Space,hDC,ChildPipe);
			}
	   }
    */
	};
};

/////////////////////////////////////////////////////////

class CVSpace3  : public CVSpace2
{
public:
	int32         m_AreaFront;
	int32         m_AreaBack;
	
	int32         m_SizeZ;
    
protected:
	float32       m_LocalMatrix[16];   //存储本物体本地位置的矩阵变换结果，见ToPlace()
public:
	CVSpace3();
	CVSpace3(int64 ID);
	virtual ~CVSpace3();

	void    LocalIdentity(){for (int i=0; i<16; i++)m_LocalMatrix[i]= i%5?0.0f:1.0f;};    

    //把此空间摆放在父空间的某个地点及朝向，并预先计算出矩阵变换结果保存在m_LocalMatrix[16]中
	//注意：必须确保此函数的调用处于opengl环境中，比如wglMakeCurrent(m_DC, m_RC);
	virtual void       ToPlace(float32 transx, float32 transy, float32 transz,float32 rotx, float32 roty, float32 rotz);
	virtual CVSpace3*  HitTest(float32 x, float32 y, float32 z){return NULL;};

	//3D绘制
	virtual void       Draw3D(float32* ParentMatrix,ePipeline* Pipe = NULL){
		if (ParentMatrix)
		{
			glLoadMatrixf(ParentMatrix);      //引入父空间的矩阵变换，
			glMultMatrixf(m_LocalMatrix);     //与本物体的本地变换矩阵相乘
			glGetFloatv(GL_MODELVIEW_MATRIX,ParentMatrix);//得到基于全局坐标的矩阵变换
		}
	    
		/*
		glBegin();
		...
		glEnd();
		*/
				
		//然后绘制子空间
		for(uint32 i=0; i<m_ChildList.size(); i++){
			CVSpace3* Space = (CVSpace3*)m_ChildList[i];
			if (ParentMatrix)
			{
				float32 NewParentMatrix[16]; 
				::memcpy(NewParentMatrix,ParentMatrix,sizeof(NewParentMatrix));
				Space->Draw3D(NewParentMatrix,Pipe);
			}else{
                Space->Draw3D(ParentMatrix,Pipe);
			}
		}

	}

};

	
} //namespace VISUALSPACE
/////////////////////////////////////////////////////////////////////////////
#endif // !defined(_VISIBLESPACE_H__)
