/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _WORLDSHOW_H__
#define _WORLDSHOW_H__


#include "GLWin.h"
#include "TGLWin.h"
#include "Room3D.h"
#include "Label3D.h"
#include "objectView.h"
#include "NavigationBar.h"
#include "MapView.h"

//定义这个决定是使用单独线程绘制场景，还是在消息循环中绘制场景
//#define  _USE_THREAD_RENDER

#define  CM_OPENVIEW     2000
#define  CM_CLOSEVIEW    2001

#define  BNT_MAP           10
#define  BNT_CREATEROOM    20
#define  BNT_IMPORTOBJECT  30

const float piover180 = 0.0174532925f;

double getRotateAngle(double x1, double y1, double x2, double y2);

class CWorldShow;

//鼠标不在此View时将自动隐藏
class CAutoObjectView : public CObjectView
{
	friend CWorldShow;
public:
	tstring         m_ViewName;
    CWorldShow*     m_Parent;
	ObjectItem*     m_RBSelected;
    tstring         m_AddressText; //类似与  \Hoem\LiveRoom
public:
	CAutoObjectView();
	virtual ~CAutoObjectView(){};  	

	void Show();
	void Hide();
	void SetHeaderText(tstring Title,BOOL Enable=TRUE);

protected:
	void OnImportObject();
    void OnCreateChildRoom(bool IsContainer=false);
    void OnDeleteObjectFromRoom(tstring Fingerprint);
    void OnExportObjectAsFile(tstring Name, int64 FatherID, int64 ChildID);
	void OnSelectObject(ObjectItem* Item);

protected:
	virtual LRESULT ToolbarReaction(ButtonItem* Bnt);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseMove(int64 wParam, int64 lParam);
	LRESULT OnRButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

};

#define CM_GOTOSPACE  300

class CWorldShow :
#ifdef _USE_THREAD_RENDER
     public CTGLWin
#else 
     public CGLWin 
#endif	 
{

protected:
	/*一式两份，通常只显示一份，另一份则在进入之前后台更新
	*/
	struct  RoomView
	{
		CRoom3D          Room;
		CAutoObjectView  ObjectView;

		void Clear(){
		    ObjectView.OnClearAll(0,0);
		    Room.ClearAllObject();
		}
	};
    
	RoomView   m_Room1;
	RoomView   m_Room2;

	RoomView*  m_CurRoom;     //指出当前位于那个房间
    int64      m_WhoUpdating; //是否正在更新，更新期间目前不允许进入任何一个其他空间。

	
	mapunit*   m_CurDypass;  //指出当前所在走廊
	mapunit*   m_PreDypass;  //之前所在走廊

	bool       m_ShowBoth;   //当两个空间之间的过道打开时，两个房间都需要绘制，否则只绘制当前空间
	
	float      m_heading;
	
	float      m_xpos;
	float      m_zpos;

	
	GLfloat	    m_yrot;				// Y Rotation
	GLfloat     m_xrot;
	GLfloat     m_walkbias;
	GLfloat     m_walkbiasangle;
	GLfloat     m_lookupdown;

	CLabel3     m_RoomTitle;
    CLabel3     m_StatusInfo;
       
	bool        m_bWorldActive;  //指出是否可以漫游  

	int32       m_MenuBmpFont;

	RoomView* GetOtherRoom(RoomView* Room){
		return Room==&m_Room1 ? &m_Room2 :&m_Room1;
	}
	RoomView* GetUpdataRoom(int64 ParentID){
		return m_Room1.Room.m_Alias == ParentID? &m_Room1: &m_Room2;
	}
	CDoorWall* GetOtherDoor(RoomView* Room, mapunit* Dypass); //得到指定房间对应通道对应房间的门

protected:  //Navigation 用导航棒操作3D漫游
	enum NAVIBAR_STATE{NAVIBAR_INIT,NAVIBAR_STOP,NAVIBAR_MOVE};
#define  ID_TIMER_NAVI  1127
  
	NAVIBAR_STATE    m_NaviBarState;
	int32            m_NaviDistance;     //default = 25
	int32            m_NaviMoveAcceleration; //default = 2;
	int32            m_NaviMoveSpeed;        //init = 0;
	int32            m_NaviMoveTime;
	CNavigationBar   m_NaviBar;
    
protected: //用键盘操作3D漫游
	int32            m_Key;
	int32            m_MoveAcceleration; //default = 2;
	int32            m_MoveSpeed;        //init = 0;
	int32            m_MoveTime;

	void GoBack(int32 s);
	void GoAhread(int32 s);


	//x0,z0为漫游之前的位置
	void RoamingWorld(float x0,float z0);
    bool OpenDoor();  //能打开返回ture，反之false
    void EnterOtherRoom();
    void CloseDoor();
	
	friend  class CAutoObjectView;
public:
    int LoadGLTexture(const TCHAR* Bmp); //返回纹理号，失败返回-1
    int LoadGLTextureFromRes(uint32 ResHandle);
 
    virtual void Draw2DAll();
    virtual void Layout(bool Redraw=true);
public:
	CWorldShow();
	virtual ~CWorldShow();

	//在跳跃式方位指定空间之前，清空所有数据，同时设置好父空间
	void  Reset(int64 ChildID,int64 ParentID,tstring ParentName,SPACETYPE ParentType);

	int64 GetRoomID(){ return m_CurRoom->Room.m_Alias;};

	bool  AllowCreateRoom(){ return m_CurRoom->Room.AllowInsertRoom();}
    bool  AllowImportObject(){ return m_CurRoom->Room.GetOneBlankMapUnit() != NULL;}
   
    void  BeginUpdateRoom(tstring RoomName, int64 RoomID,int ObjectNum);
    void  EndUpdateRoom(int64 RoomID);

    void  ConnectRoomFail(tstring Reason);

	void  SetStatusText(tstring s);
	void  SetRoomTitle(tstring Title);

	void  AddObject(int64 ParentID,int64 ID,tstring& Name,SPACETYPE Type,tstring& Fingerprint,HICON hIcon);
	void  DeleteObject(tstring Fingerprint);
    int32 FindObject(tstring& Name, vector<ObjectItem>& RoomList);
	void  ClearAllObject(int64 ParentID);

	virtual LRESULT SendChildMessage(HWND Child,UINT Msg,int64 wParam, int64 lParam);
		
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
    virtual LRESULT ParentReaction(SpaceRectionMsg* SRM);

#ifdef _USE_THREAD_RENDER
	virtual void GLReaction();
#endif	

	LRESULT OnKeydown(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);

    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnRButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
	virtual bool InitScene();

	virtual void Draw3D(float32* ParentMatrix=NULL,ePipeline* Pipe = NULL);
	
};

#endif //_WORLDSHOW_H__
