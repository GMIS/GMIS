// WorldShow.cpp: implementation of the CWorldShow class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4786)
#pragma warning(disable: 4244)

#include "GMIS.h"

#include "MainFrame.h"
#include "WorldShow.h"
#include "MapView.h"
#include <math.h>
#include "Object3D.h"
#include "Text3D.h"
#include "SystemSetting.h"
#include "resource.h"
#include <commdlg.h>
#include "Win32Tool.h"
#include "zip_utils\unzip.h"

double getRotateAngle(double x1, double y1, double x2, double y2)
{
	const double epsilon = 1.0e-6;
	const double nyPI = acos(-1.0);
	double dist, dot, degree, angle;

	// normalize
	dist = sqrt( x1 * x1 + y1 * y1 );
	x1 /= dist;
	y1 /= dist;
	dist = sqrt( x2 * x2 + y2 * y2 );
	x2 /= dist;
	y2 /= dist;
	// dot product
	dot = x1 * x2 + y1 * y2;
	if ( fabs(dot-1.0) <= epsilon ) 
		angle = 0.0;
	else if ( fabs(dot+1.0) <= epsilon ) 
		angle = nyPI;
	else {
		double cross;

		angle = acos(dot);
		//cross product
		cross = x1 * y2 - x2 * y1;
		// vector p2 is clockwise from vector p1 
		// with respect to the origin (0.0)
		if (cross < 0 ) { 
			angle = 2 * nyPI - angle;
		}    
	}
	degree = angle *  180.0 / nyPI;
	return degree;
}
static tstring  AfxNewObjectName;

BOOL CALLBACK GetNameDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	
	switch (message)
	{
		case WM_INITDIALOG:
			{
			 tstring* Title = (tstring*)lParam;
 			 ::SetWindowText(hDlg,Title->c_str());
             CenterWindow(hDlg,GetParent(hDlg));
             HWND hEdit   = GetDlgItem(hDlg,IDC_INPUTEDIT);
			 ::SetFocus(hEdit);
			return TRUE;
			}
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK ) 
			{
				HWND hName = GetDlgItem(hDlg,IDC_INPUTEDIT);
				TCHAR buf[100];
				::GetWindowText(hName,buf,99);
				AfxNewObjectName = buf;
			   	if(AfxNewObjectName.size()>0 ){
					::SetWindowText(hName,_T(""));
					EndDialog(hDlg, 1);
				}
				return TRUE;
			}else if(LOWORD(wParam) == IDCANCEL){
                EndDialog(hDlg, 0);
				return TRUE;
			}
	}
    return FALSE;
};

CAutoObjectView::CAutoObjectView(){
	m_State &= ~SPACE_SHOWWINBORDER;

	ObjectHeader* Header = (ObjectHeader*)m_ChildList[0];
	Header->m_crText = RGB(0,0,0);

	m_Toolbar.DeleteAll();
	
	ButtonItem*  Import= new ButtonItem(ID_ROOM_IMPORTOBJECT,_T("Import"),NULL,false);
	m_Toolbar.PushChild(Import);
	    
	ButtonItem* Export = new ButtonItem(ID_ROOM_EXPORTFILE,_T("Export"),NULL,false);
	m_Toolbar.PushChild(Export);
	  
	ButtonItem* Delete = new ButtonItem(ID_ROOM_DELETE,_T("Delete"),NULL,false);
	m_Toolbar.PushChild(Delete);
	
    ButtonItem* Select = new ButtonItem(ID_ROOM_SELECTOBJECT,_T("Select"),NULL,false);
	m_Toolbar.PushChild(Select);
	
    ButtonItem* CreateSpace = new ButtonItem(ID_ROOM_CREATEROOM,_T("CreateSpace"),NULL,false);
	m_Toolbar.PushChild(CreateSpace);
};
void CAutoObjectView::Show(){
   RECT rc;
   assert(m_Parent && ::IsWindow(m_Parent->GetHwnd()));
   ::GetClientRect(m_Parent->GetHwnd(),&rc);
   m_Parent->Dead();
   ::SetWindowPos(GetHwnd(),NULL,0,0,RectWidth(rc),RectHeight(rc),SWP_SHOWWINDOW);

}

void CAutoObjectView::Hide(){
	assert(m_Parent && ::IsWindow(m_Parent->GetHwnd()));
	SetWindowPos(GetHwnd(),NULL,0,0,0,0,SWP_HIDEWINDOW);
	m_Parent->Activate();
}

void CAutoObjectView::SetHeaderText(tstring Title,BOOL Enable/*=TRUE*/){
	m_ViewName = Title;
	tstring s = Format1024(_T("%s:     %d Object"),Title.c_str(),m_ChildList.size()-1);
	if(GetHwnd())
		SendChildMessage(GetHwnd(),OBJECT_SETTITLE,(int64)s.c_str(),(int64)Enable);
	else
		OnSetTitle((int64)Title.c_str(),(int64)Enable);
}

void CAutoObjectView::OnImportObject(){
	CLinker   World;
	GetBrain()->GetLinker(SPACE_SOURCE,World); 
    if (!World.IsValid()){
		MessageBox(GetHwnd(),_T("Connect Superior fail"),_T("Warning"),MB_OK);
		return;
	}

	if(!m_Parent->AllowImportObject()){
		::MessageBox(GetHwnd(),_T("Do not have enough space"),_T("Warning"),MB_OK);
		 return ;
	}

	OPENFILENAME ofn ;
	static TCHAR szFilter[] =	TEXT ("DLL Files (*.zip)\0*.zip\0")  \
		                        TEXT ("All Files (*.*)\0*.*\0\0") ;
	
	ofn.lStructSize 				= sizeof (OPENFILENAME) ;
	ofn.hwndOwner					= NULL ;
	ofn.hInstance					= NULL ;
	ofn.lpstrFilter 				= szFilter ;
	ofn.lpstrCustomFilter	= NULL ;
	ofn.nMaxCustFilter		= 0 ;
	ofn.nFilterIndex		= 0 ;
	ofn.nMaxFile				= MAX_PATH ;
	ofn.nMaxFileTitle			= MAX_PATH ;
	ofn.lpstrInitialDir 		= NULL ;
	ofn.lpstrTitle				= NULL ;
	ofn.nFileOffset 			= 0 ;
	ofn.nFileExtension			= 0 ;
	ofn.lpstrDefExt 			= TEXT ("zip") ;
	ofn.lCustData				= 0L ;
	ofn.lpfnHook				= NULL ;
	ofn.lpTemplateName			= NULL ;
     	
	TCHAR   szFileName[MAX_PATH], szFileTitle[MAX_PATH];
	szFileName[0]='\0';
    ofn.lpstrFile         		= szFileName ;
    ofn.lpstrFileTitle    		= szFileTitle; 
    ofn.Flags             		= OFN_HIDEREADONLY | OFN_CREATEPROMPT ;
	

	HANDLE 	   hFile = NULL;	

	//打开文件,得到文件数据
    if(!::GetOpenFileName (&ofn)){
		return;
	};

	//寻找是否有与ZIP名字相同的DLL文件
	HZIP zipFile = OpenZip(szFileName,0);
	ZIPENTRY ze; GetZipItem(zipFile,-1,&ze); int numitems=ze.index;
	
	tstring ObjectFileName = szFileTitle;
	tstring::size_type p = ObjectFileName.find(_T(".zip"));
	if (p!= tstring::npos)
	{
		ObjectFileName = ObjectFileName.substr(0,p);
		ObjectFileName += _T(".dll");
	}

	bool bFind = false;
	for (int i=0; i<numitems; i++)
	{ 
		ZIPENTRY ze;
		GetZipItem(zipFile,i,&ze); // fetch individual details
	    
		if(ze.name == ObjectFileName )
	    {
			bFind = true;
			break;
	    }
	}
	CloseZip(zipFile);

	if (!bFind)
	{
		tstring Info = Format1024(_T("Import object fail: not find %s in zip file"),ObjectFileName.c_str());	
		GetBrain()->OutSysInfo(Info.c_str());
		return;
	}

	tstring Info = Format1024(_T("Import object = %s"),ObjectFileName.c_str());	
	GetBrain()->OutSysInfo(Info.c_str());
	

	CMsg Msg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_ROBOT_CREATE_SPACE,DEFAULT_DIALOG,0);

	int64 CurSpaceID = m_Parent->GetSpaceID();

    ePipeline SpacePath;
	GetGUI()->GetSpacePath(SpacePath);
	
	ePipeline& rLetter = Msg.GetLetter(false);
	rLetter.PushInt(OBJECT_SPACE);
	rLetter.PushString(ObjectFileName);
	rLetter.PushString(szFileName);
	rLetter.PushPipe(SpacePath);
//	rLetter.PushInt64(CurSpaceID);

    GetBrain()->SendMsgToSpace(Msg);
}


void CAutoObjectView::OnCreateChildSpace(bool IsContainer/* =false */){
	CLinker   World;
	GetBrain()->GetLinker(SPACE_SOURCE,World); 
    if (!World.IsValid()){
		MessageBox(GetHwnd(),_T("Connect Superior fail"),_T("Warning"),MB_OK);
		return;
	}
			
	if(!m_Parent->AllowCreateSpace()){
		::MessageBox(GetHwnd(),_T("Do not have enough space"),_T("Create fail"),MB_OK);
		return ;
	}

	tstring Param = _T("Input New Space Name");
	if (IsContainer) Param = _T("Input New Container Name");
	
	tstring Name;
	int ret = DialogBoxParam (CWinSpace2::GetHinstance(),MAKEINTRESOURCE(IDD_INPUTEDIT), 
		              GetHwnd(), GetNameDlgProc, (LPARAM)&Param);
	if(ret>0){
		Name = AfxNewObjectName;
	}else{
		return;
	}

	CMsg Msg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_ROBOT_CREATE_SPACE,DEFAULT_DIALOG,0);
	ePipeline& Letter = Msg.GetLetter(false);
	if(IsContainer)Letter.PushInt(CONTAINER_SPACE);
	else  Letter.PushInt(LOCAL_SPACE);
	Letter.PushString(Name);
	GetBrain()->SendMsgToSpace(Msg);
};

void CAutoObjectView::OnDeleteObjectFromSpace(tstring Fingerprint){

	CLinker   World;
	GetBrain()->GetLinker(SPACE_SOURCE,World); 
    if (!World.IsValid()){
		MessageBox(GetHwnd(),_T("Connect Superior fail"),_T("Warning"),MB_OK);
		return;
	}

	ePipeline Path;
	GetGUI()->GetSpacePath(Path);
    
	
	CMsg Msg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_ROBOT_DEL_SPACE,DEFAULT_DIALOG,0);
	ePipeline& rLetter = Msg.GetLetter(false);
	rLetter.PushPipe(Path);
	rLetter.PushString(Fingerprint);
	GetBrain()->SendMsgToSpace(Msg);

}
void CAutoObjectView::OnExportObjectAsFile(tstring Name, int64 FatherID, int64 ChildID){
	
	CLinker  World;
	GetBrain()->GetLinker(SPACE_SOURCE,World); 
    if (!World.IsValid()){
		MessageBox(GetHwnd(),_T("Connect Superior fail"),_T("Warning"),MB_OK);
		return;
	}
/*
	AnsiString LocalIP = "127.0.0.1";
	AnsiString HostIP = GetBrain()->GetHostIP();
	if(HostIP != LocalIP){
		MessageBox(GetHwnd(),_T("Export fail: not local IP"),_T("Warning"),MB_OK);
		return ;
	}
*/
	OPENFILENAME ofn ;
	
	ofn.lStructSize 				= sizeof (OPENFILENAME) ;
	ofn.hwndOwner					= NULL ;
	ofn.hInstance					= NULL ;
	ofn.lpstrFilter 				= NULL;
	ofn.lpstrCustomFilter	= NULL ;
	ofn.nMaxCustFilter		= 0 ;
	ofn.nFilterIndex		= 0 ;
	ofn.nMaxFile				= MAX_PATH ;
	ofn.nMaxFileTitle			= MAX_PATH ;
	ofn.lpstrInitialDir 		= NULL ;
	ofn.lpstrTitle				= NULL ;
	ofn.nFileOffset 			= 0 ;
	ofn.nFileExtension			= 0 ;
	ofn.lpstrDefExt 			= NULL;
	ofn.lCustData				= 0L ;
	ofn.lpfnHook				= NULL ;
	ofn.lpTemplateName			= NULL ;
    
	TCHAR   szFileName[MAX_PATH], szFileTitle[MAX_PATH];
	_tcscpy(szFileName,Name.c_str());
	ofn.hwndOwner               = GetGUI()->GetHwnd();
    ofn.lpstrFile         		= szFileName ;
    ofn.lpstrFileTitle    		= szFileTitle ;
    ofn.Flags             		= OFN_OVERWRITEPROMPT ;
     
    if(!::GetSaveFileName (&ofn)){
		return;
	}
	tstring Task = Format1024(_T("Export object = %s"),szFileTitle);
    GetBrain()->OutSysInfo(Task.c_str());
		  
	CMsg Msg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_ROBOT_EXPORT_OBJECT,DEFAULT_DIALOG,0);
	ePipeline& Letter = Msg.GetLetter(false);
	Letter.PushInt(FatherID);
	Letter.PushInt(ChildID);
	Letter.PushString(szFileName);
	
    GetBrain()->SendMsgToSpace(Msg);
};	

void CAutoObjectView::OnSelectObject(ObjectItem* Ob){
	
	ePipeline Msg(TO_BRAIN_MSG::GUI_OBJECT_OPERATE::ID);
		
	Msg.PushInt(TO_BRAIN_MSG::GUI_OBJECT_OPERATE::SELECT_OBJECT);

	ePipeline Addr;
	GetGUI()->GetSpacePath(Addr);
//	Addr.PushString(Ob->m_Name);

	ePipeline* Item = new ePipeline;
	Item->PushString(Ob->m_Name);
	Item->PushInt(Ob->m_Type);
	Item->PushString(Ob->m_Fingerprint);
	Item->PushPipe(Addr);
	Item->PushInt(0);

	Msg.Push_Directly(Item);
	
	GetGUI()->SendMsgToBrainFocuse(Msg);
}

LRESULT CAutoObjectView::ToolbarReaction(ButtonItem* Bnt){
	//向brain发相应的命令
	switch(Bnt->m_Alias)
	{
	case ID_ROOM_IMPORTOBJECT:
		{  	
			OnImportObject();
		}
		break;
	case ID_ROOM_EXPORTFILE:
		{
			ObjectItem* Ob = (ObjectItem*)m_SpaceSelected;
			if(Ob ==NULL || Ob->m_Alias==0 || 
				Ob->m_Type != OBJECT_SPACE){
				::MessageBox(GetHwnd(),_T("illegal command"),_T("Tip"),MB_OK);
				return 0;
			}			
			OnExportObjectAsFile(Ob->m_Name, m_Parent->GetSpaceID(), Ob->m_Alias);
		}
		break;
	case ID_ROOM_DELETE:
		{
			ObjectItem* Ob = (ObjectItem*)m_SpaceSelected;
			if(Ob ==NULL || Ob->m_Alias==0 || 
				Ob->m_Type == ROBOT_VISITER){
				::MessageBox(GetHwnd(),_T("illegal command"),_T("Tip"),MB_OK);
				return 0;
			}	
			
			OnDeleteObjectFromSpace(Ob->m_Fingerprint);
		}
		break;
    case ID_ROOM_SELECTOBJECT:
		{
			ObjectItem* Ob = (ObjectItem*)m_SpaceSelected;
			if(Ob ==NULL || Ob->m_Alias==0 /*|| Ob->m_Type != OBJECT_SPACE*/){
				::MessageBox(GetHwnd(),_T("illegal command"),_T("Tip"),MB_OK);
				return 0;
			}	
			
			OnSelectObject(Ob);
		}
		break;
	case ID_ROOM_CREATEROOM:
		{
            OnCreateChildSpace(FALSE);	
		}
		break;
	}

	Invalidate();
	return 0;
}      
LRESULT CAutoObjectView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if(message == WM_MOUSEMOVE){
		return OnMouseMove(wParam,lParam);
	}else if (message == WM_RBUTTONDOWN)
	{
		return OnRButtonDown(wParam,lParam);
	}else if (message == WM_COMMAND)
	{
		return OnCommand(wParam,lParam);
	}
	else return CObjectView::Reaction(message,wParam,lParam);
}
LRESULT CAutoObjectView::OnRButtonDown(WPARAM wParam, LPARAM lParam){
	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;	
	m_RBSelected = (ObjectItem*)Hit(point);

	POINT pos;
	::GetCursorPos(&pos);
				
	HMENU hmenu;
	HMENU subMenu;
	
	HINSTANCE hInstance = CWinSpace2::GetHinstance();
	hmenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_ROOM));
	if(hmenu == NULL){
		return TRUE;
	}
	
	subMenu = GetSubMenu(hmenu, 0); 				
	::SetForegroundWindow(GetHwnd()); 
	::TrackPopupMenu(subMenu,0, pos.x, pos.y,0,GetHwnd(),NULL);
	::DestroyMenu(hmenu); 
	::PostMessage(GetHwnd(),WM_NULL, 0, 0);		

	return FALSE;
}
LRESULT CAutoObjectView::OnMouseMove(int64 wParam, int64 lParam){
	
	int xPos = LOWORD(lParam);  // horizontal position of cursor 
	int yPos = HIWORD(lParam);  // vertical position of cursor 
	
	POINT pt;
	::GetCursorPos(&pt);
	::ScreenToClient(GetHwnd(),&pt);
	RECT  rcClient;
	::GetClientRect(GetHwnd(),&rcClient);
	if(::PtInRect(&rcClient,pt)){
		if(::GetCapture()!=GetHwnd()){
			SetCapture(GetHwnd());
		}

		return CObjectView::OnMouseMove(wParam,lParam);
	}else {
		::ReleaseCapture();
		SetWindowPos(GetHwnd(),NULL,0,0,0,0,SWP_HIDEWINDOW);
		m_Parent->Activate();
	}
	return 0;
};

LRESULT  CAutoObjectView::OnCommand(WPARAM wParam, LPARAM lParam){
	
	switch(wParam){
	case ID_ROOM_IMPORTOBJECT:
		{  

			OnImportObject();
		}
		break;
	case ID_ROOM_EXPORTFILE:
		{
			if(m_RBSelected ==NULL || m_RBSelected->m_Alias==0 || 
				m_RBSelected->m_Type == ROBOT_VISITER){
				//::MessageBox(GetHwnd(),"illegal command","Warning",MB_OK);
				return 0;
			}			
			OnExportObjectAsFile(m_RBSelected->m_Name, m_Parent->GetSpaceID(), m_RBSelected->m_Alias);
		}
 		break;
   case ID_ROOM_DELETE:
		{
			if(m_RBSelected ==NULL || m_RBSelected->m_Alias==0 || 
				m_RBSelected->m_Type == ROBOT_VISITER){
				//::MessageBox(GetHwnd(),"illegal command","Warning",MB_OK);
				return 0;
			}	
			
			OnDeleteObjectFromSpace(m_RBSelected->m_Fingerprint);
		}
		break;
    case ID_ROOM_SELECTOBJECT:
		{
			OnSelectObject(m_RBSelected);
		}
		break;
	case ID_ROOM_CREATEROOM:
		{
            OnCreateChildSpace(FALSE);	
		}
		break;
	}			
	return 0;
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWorldShow::CWorldShow()
{

	m_Space1.ObjectView.m_Parent = this;
    m_Space2.ObjectView.m_Parent = this;

	m_WhoUpdating  = 0;

	m_NaviDistance = 80;
	m_NaviBarState = NAVIBAR_INIT;
	m_NaviMoveSpeed = 0;
	m_NaviMoveTime = 0;
	m_NaviMoveAcceleration = 2;

    m_MoveAcceleration = 2;
    m_MoveSpeed = 0;
    m_MoveTime = 0;
   
	ButtonItem3*  BntMap = new ButtonItem3;
	ButtonItem3*  BntCreateSpace = new ButtonItem3;
	ButtonItem3*  BntImportObject = new ButtonItem3;

    BntMap->m_Alias  = BNT_MAP;
	BntMap->SetColor(RGB(0,0,0),RGB(255,255,255),RGB(0,255,0));
	BntMap->SetText(_T("Map"),false);
	
	BntCreateSpace->m_Alias = BNT_CREATEROOM;
	BntCreateSpace->SetColor(RGB(0,0,0),RGB(255,255,255),RGB(0,255,0));
	BntCreateSpace->SetText(_T("CreateSpace"),false);

	BntImportObject->m_Alias = BNT_IMPORTOBJECT;
	BntImportObject->SetColor(RGB(0,0,0),RGB(255,255,255),RGB(0,255,0));
	BntImportObject->SetText(_T("ImportObject"),false);

	PushChild(BntMap);
	PushChild(BntCreateSpace);
	PushChild(BntImportObject);
};

CWorldShow::~CWorldShow()
{

};
	
void CWorldShow::Reset(int64 ChildID,int64 ParentID,tstring ParentName,SPACETYPE ParentType){
	m_State = 0;

	m_ShowBoth = false;

	m_heading = 0.0f;
	
	m_xpos = 0.0f;
	m_zpos = 0.0f;

 	m_CurDypass = NULL;
	m_PreDypass = NULL;

	m_yrot = 0.0f;				// Y Rotation
	m_xrot = 0.0f;
	m_walkbias = 0;
	m_walkbiasangle = 0;
	m_lookupdown = 0.0f;

	m_WhoUpdating  = 0;

	m_NaviMoveTime = 0;
    m_MoveTime = 0;

  	m_Space1.Space.ClearAllObject();
    m_Space2.Space.ClearAllObject();
  	m_Space1.Space.m_Alias = ChildID;
  	m_Space2.Space.m_Alias = -1;

	m_SpaceTitle.SetText(_T(""),false,SS.m_Font24);

	m_CurSpace = &m_Space1;

	wglMakeCurrent(m_DC, m_RC);

	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
    
	glMatrixMode(GL_PROJECTION);
	m_CurSpace->Space.ToPlace(0,0,0,0,0,0);
	m_CurSpace->Space.SetParentSpace(FRONTFACE,ParentID,ParentName,ParentType);
		
	wglMakeCurrent(NULL,NULL);

}
bool CWorldShow::InitScene(){

//	glEnable(GL_TEXTURE_2D);                            // Enable Texture Mapping
//	glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_ONE);					// Set The Blending Function For Translucency

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// This Will Clear The Background Color To Black
	glClearDepth(1.0);									// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);								// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
//	glShadeModel(GL_SMOOTH);							// Enables Smooth Color Shading
	//glEnable(GL_LIGHT0);					// Enable Default Light (Quick And Dirty)	( NEW )
	//glEnable(GL_LIGHTING);					// Enable Lighting				( NEW )
	//glEnable(GL_COLOR_MATERIAL);		    // Enable Coloring Of Material			( NEW )


	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	glInitDefaultFont2D(_T("Arial"));
	glInitDefaultFont3D(_T("Arial"));

	m_StatusInfo.SetColor(RGB(64,64,64),RGB(0,255,0),RGB(0,255,0));

	m_SpaceTitle.SetColor(RGB(64,64,64),RGB(255,0,0),RGB(0,255,0));
    m_SpaceTitle.SetText(_T("Space Unconnected"),false,SS.m_Font24);
    
	m_SpaceTitle.m_State = 0; //不显示背景和border
    m_StatusInfo.m_State = 0;

	m_Space1.Space.SetSize(12,2,12); //调用必须先于纹理设置
	m_Space2.Space.SetSize(12,2,12);
	
	//int roomwalltex = LoadGLTexture(_T("roomwall.bmp"));
    int roomwalltex = LoadGLTextureFromRes(IDB_ROOMWALL);
	if(roomwalltex==-1)return false;

	//int floortex = LoadGLTexture(_T("floor.bmp"));
	int floortex = LoadGLTextureFromRes(IDB_FLOOR);
	if(floortex == -1)return false;
	
	//int ceilingtex = LoadGLTexture(_T("ceiling.bmp"));
	int ceilingtex = LoadGLTextureFromRes(IDB_CEILING); 
	if(ceilingtex == -1)return false;

    //int doortex = LoadGLTexture(_T("door.bmp"));
    int doortex = LoadGLTextureFromRes(IDB_DOOR);
	if(doortex == -1)return false;

 	//int metaltex = LoadGLTexture(_T("metal.bmp"));
	int metaltex = LoadGLTextureFromRes(IDB_METAL);

	m_Space1.Space.InitTexture(roomwalltex,floortex,ceilingtex,doortex,0);
	m_Space2.Space.InitTexture(roomwalltex,floortex,ceilingtex,doortex,0);
		

	Reset(LOCAL_SPACEID,1,_T("Outer World"),OUTER_SPACE);
   
	ConnectSpaceFail(_T("Space Disconnected"));

//	AddObject(0,"Object",OBJECT_SPACE,NULL,100,"3DE25F61");
	for(int i=0; i<0; i++){
	
//	for(int i=0; i<m_CurSpace->Space.GetMapUintSize(); i++){
	//	AddObject(i,"Object",OBJECT_SPACE,NULL,100,"3DE25F61");
	};

/*	int index =m_CurSpace->Space.AddChildSpace(100,"LiveSpace",LOCAL_SPACE);
    m_Space2.Space.m_DataValid = true;
	m_Space2.Space.m_Alias = 100;
	m_Space2.Space.SetParentSpace(RIGHTFACE,1,"LiveSpace",LOCAL_SPACE);
*/    
	return true;
};

void CWorldShow::BeginUpdateSpace(tstring SpaceName, int64 SpaceID,int ObjectNum){

	SpaceView* UpdateSpace = GetUpdataSpace(SpaceID);
    UpdateSpace->Clear();
	UpdateSpace->Space.m_Name = SpaceName;
	if(m_CurSpace==UpdateSpace){
		m_SpaceTitle.SetText(SpaceName,false,SS.m_Font24);
	}

	tstring s = Format1024(_T("Updata Space %s..."),SpaceName.c_str()); 
	m_StatusInfo.SetText(s,false);
	   
	Invalidate();
}
    
void CWorldShow::EndUpdateSpace(int64 SpaceID){
	SpaceView* UpdateSpace = GetUpdataSpace(SpaceID);
  
	UpdateSpace->Space.m_DataValid = true;
    m_StatusInfo.SetText(_T(""),false);
	UpdateSpace->ObjectView.SetHeaderText(UpdateSpace->Space.m_Name);

	if(UpdateSpace != m_CurSpace && m_CurDypass){ //如果用户还正等待在更新的空间门前则打开
		//找到通向对应空间的门
		CDoorWall* dw = m_CurSpace->Space.m_DoorWall[m_CurDypass->face];
		if(dw == NULL)return;		
		if(UpdateSpace->Space.IsValidSpace(dw->m_Alias)){
			wglMakeCurrent(m_DC, m_RC);
			OpenDoor();
			wglMakeCurrent(NULL, NULL);
		}
	}
	m_WhoUpdating = 0;
	Invalidate();	
}

void CWorldShow::RoamingWorld(float x0,float z0){

        wglMakeCurrent(m_DC, m_RC);

		//把新位置的世界坐标转换成基于当前房间的本地坐标
		float xm = m_xpos-m_CurSpace->Space.m_x0;
		float zm = m_zpos-m_CurSpace->Space.m_z0;
		
		//优先检查是否已经在走廊内
        if (m_CurDypass)//如果之前在
        { 
		    if (m_CurDypass->InRect(xm,zm) ) //改变后的位置也在
			{
				return ; //属于正常，什么也不做
		    } 
		    else //现在不在了，否则判断是否出界，并走向哪里
		    {
				if(m_CurDypass->face == FRONTFACE){
				     if (xm<m_CurDypass->x1 || xm>m_CurDypass->x2) m_xpos=x0;
					 if(zm<m_CurDypass->z1){
						 CloseDoor(); //离开走廊，重新回到当前房间
					 }
					 else if (zm>m_CurDypass->z2) //离开走廊，进入此门相对的房间
					 {
						 SpaceView* Space = GetOtherSpace(m_CurSpace);
                         if (Space->Space.IsValidSpace(m_CurDypass->Space->m_Alias))
                         {
							 EnterOtherSpace();
                         }else{
                             m_zpos =z0;
							 m_xpos =x0;
						 }				 
					 }     
				}else if (m_CurDypass->face == LEFTFACE)
				{
				     if (zm<m_CurDypass->z1 || zm>m_CurDypass->z2) m_zpos=z0;
					 if(xm>m_CurDypass->x2){
						 CloseDoor();
					 }
					 else if (xm<m_CurDypass->x1)
					 {
						 SpaceView* Space = GetOtherSpace(m_CurSpace);
                         if (Space->Space.IsValidSpace(m_CurDypass->Space->m_Alias))
                         {
							 EnterOtherSpace();
                         }else{
                             m_zpos =z0;
							 m_xpos =x0;
						 }				 
					 }     					
				} 
				else if(m_CurDypass->face == BACKFACE)
				{
				     if (xm<m_CurDypass->x1 || xm>m_CurDypass->x2) m_xpos=x0;
					 if(zm>m_CurDypass->z2){
						 CloseDoor();
					 }
					 else if (zm<m_CurDypass->z1)
					 {
						 SpaceView* Space = GetOtherSpace(m_CurSpace);
                         if (Space->Space.IsValidSpace(m_CurDypass->Space->m_Alias))
                         {
							 EnterOtherSpace();
                         }else{
                             m_zpos =z0;
							 m_xpos =x0;
						 }			
					 }     
				}else if (m_CurDypass->face == RIGHTFACE)
				{
				     if (zm<m_CurDypass->z1 || zm>m_CurDypass->z2) m_zpos=z0;
					 if(xm<m_CurDypass->x1 ){
						 CloseDoor();
					 }
					 else if (xm>m_CurDypass->x2)
					 {
						 SpaceView* Space = GetOtherSpace(m_CurSpace);
                         if (Space->Space.IsValidSpace(m_CurDypass->Space->m_Alias))
                         {
							 EnterOtherSpace();
                         }else{
                             m_zpos =z0;
							 m_xpos =x0;
						 }			
					 }     	
				}
		    }
			wglMakeCurrent(NULL, NULL);
            return ;
        }
				
		mapunit* m = m_CurSpace->Space.IsInDypassRect(m_xpos,m_zpos);
		if(m) //之前不在门廊，现在打开对应的门
        {
			if (m_WhoUpdating) //退回去，不允许进入另一个空间
			{
				::MessageBox(GetHwnd(),_T("Please wait for updating end"),_T("Warning"),MB_OK);
				m_xpos = x0;  //退回去
				m_zpos = z0;
				Invalidate();
			}else{
				m_CurDypass = m;
				if(OpenDoor()==false){ //退回去
					m_xpos = x0;
					m_zpos = z0;
					m_CurDypass = NULL;
					Invalidate();
				}
			}
			wglMakeCurrent(NULL, NULL);
			return ;
        }
         
        //是否与房间里的物体碰撞
        m = m_CurSpace->Space.IsInObjectRect(m_xpos,m_zpos);
		if (m)
		{
			//假设X不变，z改变如果依然与Object碰撞则z不变
			if(m->InRect(x0-m_CurSpace->Space.m_x0,zm)){
				m_zpos = z0;
                m_MoveTime = 0;
				m_MoveSpeed = 0;
			}
			//假设z不变，x改变如果依然与Object碰撞则x不变
			if(m->InRect(xm,z0-m_CurSpace->Space.m_z0)){
				m_xpos = x0;
				m_MoveTime = 0;
				m_MoveSpeed = 0;
			}

		    //如果x z每一个改变都将导致碰撞则完全停止位移,会出现这种情况吗？
			//不知道,如果有就只好后退了
			wglMakeCurrent(NULL, NULL);
			return ;
		}
         
		//处理好与物体的碰撞，还要检查是否与房间墙壁发生碰撞
		float dx = m_CurSpace->Space.m_SpaceDx/2.0-0.4;
		float dz = m_CurSpace->Space.m_SpaceDz/2.0-0.4;


		//确保在墙壁组成的包围盒里,但四周的门廊（如果有的话）是一个例外

		if(xm<-dx){
			CDoorWall* dw = m_CurSpace->Space.m_DoorWall[LEFTFACE];
			mapunit& dyp = m_CurSpace->Space.m_Dypass[LEFTFACE];
			if(dw==NULL || (m_zpos<dyp.z1 || m_zpos>dyp.z2)){
				m_xpos = x0;
				m_MoveTime = 0;
				m_MoveSpeed = 0;
			}
		}
		if(xm>dx){
			CDoorWall* dw = m_CurSpace->Space.m_DoorWall[RIGHTFACE];
			mapunit& dyp = m_CurSpace->Space.m_Dypass[RIGHTFACE];
			if(dw == NULL || ( m_zpos<dyp.z1 || m_zpos>dyp.z2)){
				m_xpos = x0;
				m_MoveTime = 0;
				m_MoveSpeed = 0;
			}
		}

		if(zm<-dz){
			CDoorWall* dw = m_CurSpace->Space.m_DoorWall[BACKFACE];
			mapunit& dyp = m_CurSpace->Space.m_Dypass[BACKFACE];
			if(dw == NULL || (m_xpos<dyp.x1 || m_xpos>dyp.x2)){
				m_zpos = z0;
				m_MoveTime = 0;
				m_MoveSpeed = 0;
			}
		}
		if(zm>dz){
			CDoorWall* dw = m_CurSpace->Space.m_DoorWall[FRONTFACE];
			mapunit& dyp = m_CurSpace->Space.m_Dypass[FRONTFACE];
			if(dw == NULL || (m_xpos<dyp.x1 || m_xpos>dyp.x2)){
				m_zpos = z0;
				m_MoveTime = 0;
				m_MoveSpeed = 0;
			}
		}

        wglMakeCurrent(NULL, NULL);
};
    

/*
- 检查欲进入房间是否数据有效，有效则直接另一个房间在合适位置，打开门，此时双方可见
- 否则给出提示，冻结漫游操作，冻结鼠标右键操作
- 否则连接世界空间，要求获得指定空间信息
- 冻结当前空间操作，并设置一个备用空间为当前空间来承接空间更新信息
- 给出按钮，允许用户选择放弃
- 给出进度条提示（一个圆形进度条，中间为放弃按钮） 
- 如果无法连接指定空间则直接放弃，恢复之前的空间为当前空间，恢复
  漫游操作
- 如果跟新完毕则打开房间，
*/
bool CWorldShow::OpenDoor(){
   if(m_CurDypass==NULL)return false;
   
   //找到空间的门
   CDoorWall* dw = m_CurSpace->Space.m_DoorWall[m_CurDypass->face];
   if(dw == NULL)return false;
   tstring Name = dw->GetName();

   //确定对应空间数据是否有效
   SpaceView* OtherSpace = GetOtherSpace(m_CurSpace);
   if(OtherSpace->Space.IsValidSpace(dw->m_Alias)){
	    //打开当前门，让两个空间都显示
        dw->m_State |=  SPACE_OPENDOOR;
	   	
		CDoorWall* Otherdw = GetOtherDoor(m_CurSpace,m_CurDypass);
		Otherdw->m_State |=  SPACE_OPENDOOR;
		m_ShowBoth = true;
		return true;
   }

   //欲访问空间地址
   int64 ParentID = 0; 
   int64 ChildID  = 0;
   
   //如果另一个空间数据无效，则需要连接数据库获取
   if(dw->m_Alias == 1){
		::MessageBox(GetHwnd(),_T("Access Denied"),_T("Warning"),MB_OK);
		return false;
   }else {
	    //先摆放另一个房间，使其正门对准我们即将通过的门
		ChildID = dw->m_Alias;
		
		float dx1 = m_CurSpace->Space.m_SpaceDx/2;
		float dz1 = m_CurSpace->Space.m_SpaceDz/2;

		float dx2 = OtherSpace->Space.m_SpaceDx/2;
		float dz2 = OtherSpace->Space.m_SpaceDz/2;
 
		assert(m_CurDypass != NULL);
		float x,y,z; //新房间的位置坐标（世界坐标）
		y = m_CurSpace->Space.m_y0; //高度都一样
		if(m_CurDypass->face == FRONTFACE){
			x = m_CurSpace->Space.m_x0;
			z = m_CurSpace->Space.m_z0+(dz1+dz2+ DYPASS_DEPTH+DYPASS_DEPTH);
			OtherSpace->Space.ToPlace(x,y,z,0,0,0);
            if (dw->m_Alias == LOCAL_SPACEID){
 				OtherSpace->Space.SetParentSpace(FRONTFACE,1, _T("Outer World"),m_CurSpace->Space.m_Type);
			} 
			if (dw->m_State & SPACE_PARENT) //等于返回自己的父空间
			{   //则在父空间注册通向自己的的门
				OtherSpace->Space.AddChildSpace(BACKFACE,m_CurSpace->Space.m_Alias, m_CurSpace->Space.m_Name,dw->m_Fingerprint);
                MapItem* SpaceItem = GetGUI()->m_MapView.FindChildItem(m_CurSpace->Space.m_Alias);
                if(SpaceItem){ //找到父空间的父空间ID作为地址向World获取数据
					MapItem* ParentItem = (MapItem*)SpaceItem->m_Parent;
				    if(ParentItem)ParentID = ParentItem->m_Alias;
				}
			}
            else{
				ParentID = m_CurSpace->Space.m_Alias;
				OtherSpace->Space.SetParentSpace(BACKFACE,m_CurSpace->Space.m_Alias, m_CurSpace->Space.m_Name,m_CurSpace->Space.m_Type);
            }
            CDoorWall*  Otherdw = OtherSpace->Space.m_DoorWall[BACKFACE];
            Otherdw->m_State |=  SPACE_OPENDOOR;

		}else if (m_CurDypass->face == BACKFACE){
			x = m_CurSpace->Space.m_x0;
			z = m_CurSpace->Space.m_z0-(dz1+dz2+ DYPASS_DEPTH+DYPASS_DEPTH);
			OtherSpace->Space.ToPlace(x,y,z,0,0,0);
			if (dw->m_Alias == LOCAL_SPACEID){
				OtherSpace->Space.SetParentSpace(FRONTFACE,1, _T("Outer World"),m_CurSpace->Space.m_Type);
            }
						
			if (dw->m_State & SPACE_PARENT) //等于返回自己的父空间
			{   //则在父空间注册通向自己的的门
				OtherSpace->Space.AddChildSpace(FRONTFACE,m_CurSpace->Space.m_Alias, m_CurSpace->Space.m_Name,dw->m_Fingerprint);
                MapItem* SpaceItem = GetGUI()->m_MapView.FindChildItem(m_CurSpace->Space.m_Alias);
                if(SpaceItem){ //找到父空间的父空间ID作为地址向World获取数据
					MapItem* ParentItem = (MapItem*)SpaceItem->m_Parent;
				    if(ParentItem)ParentID = ParentItem->m_Alias;
				}
			}
			else{
				ParentID = m_CurSpace->Space.m_Alias;
				OtherSpace->Space.SetParentSpace(FRONTFACE,m_CurSpace->Space.m_Alias, m_CurSpace->Space.m_Name,m_CurSpace->Space.m_Type);
			}
			CDoorWall*  Otherdw = OtherSpace->Space.m_DoorWall[FRONTFACE];
            Otherdw->m_State |=  SPACE_OPENDOOR;

		}else if (m_CurDypass->face == LEFTFACE){
            z = m_CurSpace->Space.m_z0;
			x = m_CurSpace->Space.m_x0-(dx1+dz2+ DYPASS_DEPTH+DYPASS_DEPTH);
            OtherSpace->Space.ToPlace(x,y,z,0,0,0);
			if (dw->m_Alias == LOCAL_SPACEID){
                OtherSpace->Space.SetParentSpace(FRONTFACE,1, _T("Outer World"),m_CurSpace->Space.m_Type);
			}
					
			if (dw->m_State & SPACE_PARENT) //等于返回自己的父空间
			{   //则在父空间注册通向自己的的门
				OtherSpace->Space.AddChildSpace(RIGHTFACE,m_CurSpace->Space.m_Alias, m_CurSpace->Space.m_Name,dw->m_Fingerprint);
                MapItem* SpaceItem = GetGUI()->m_MapView.FindChildItem(m_CurSpace->Space.m_Alias);
                if(SpaceItem){ //找到父空间的父空间ID作为地址向World获取数据
					MapItem* ParentItem = (MapItem*)SpaceItem->m_Parent;
				    if(ParentItem)ParentID = ParentItem->m_Alias;
				}
			}else{
				ParentID = m_CurSpace->Space.m_Alias;
				OtherSpace->Space.SetParentSpace(RIGHTFACE,m_CurSpace->Space.m_Alias, m_CurSpace->Space.m_Name,m_CurSpace->Space.m_Type);
            }
			CDoorWall*  Otherdw = OtherSpace->Space.m_DoorWall[RIGHTFACE];
            Otherdw->m_State |=  SPACE_OPENDOOR;

		}else{ //RIGHTFASE
            z = m_CurSpace->Space.m_z0;
			x = m_CurSpace->Space.m_x0+(dx1+dz2+ DYPASS_DEPTH+DYPASS_DEPTH);
            OtherSpace->Space.ToPlace(x,y,z,0,0,0);
            if (dw->m_Alias == LOCAL_SPACEID){
                OtherSpace->Space.SetParentSpace(FRONTFACE,1, _T("Outer World"),m_CurSpace->Space.m_Type);
			}
					
			if (dw->m_State & SPACE_PARENT) //等于返回自己的父空间
			{   //则在父空间注册通向自己的的门
				OtherSpace->Space.AddChildSpace(LEFTFACE,m_CurSpace->Space.m_Alias, m_CurSpace->Space.m_Name,dw->m_Fingerprint);
                MapItem* SpaceItem = GetGUI()->m_MapView.FindChildItem(m_CurSpace->Space.m_Alias);
                if(SpaceItem){ //找到父空间的父空间ID作为地址向World获取数据
					MapItem* ParentItem = (MapItem*)SpaceItem->m_Parent;
				    if(ParentItem)ParentID = ParentItem->m_Alias;
				}
			}else{
				ParentID = m_CurSpace->Space.m_Alias;
				OtherSpace->Space.SetParentSpace(LEFTFACE,m_CurSpace->Space.m_Alias, m_CurSpace->Space.m_Name,m_CurSpace->Space.m_Type);
            }
			CDoorWall*  Otherdw = OtherSpace->Space.m_DoorWall[LEFTFACE];
            Otherdw->m_State |=  SPACE_OPENDOOR;

		};
        if(ChildID == LOCAL_SPACEID)ParentID = ROOT_SPACE;

	    OtherSpace->Clear();
		OtherSpace->Space.m_Alias = ChildID;
        OtherSpace->Space.m_Name  = dw->GetName();

   }

   tstring s = Format1024(_T("Connecting space %s..."),Name.c_str());
   GetBrain()->OutSysInfo(s.c_str());

   CLinker World;
   GetBrain()->GetLinker(SPACE_SOURCE,World);
   if(World.IsValid()){
       m_WhoUpdating = dw->m_Alias;

	   ePipeline Path;
	   GetGUI()->GetSpacePath(Path);

	   CMsg Msg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_ROBOT_GOTO_SPACE,DEFAULT_DIALOG,0);
	   ePipeline& Letter = Msg.GetLetter(false);
	   Letter.PushPipe(Path);
	   GetBrain()->SendMsgToSpace(Msg);
	   
   }else{
	   ::MessageBox(GetHwnd(),_T("Space connected fail"),_T("Warning"),MB_OK);
	   m_StatusInfo.SetText(_T(""),false);
	   return false;
   }
   return true;
}

/*
- 进入另一个房间，改变仅仅当前空间的相关设置 
*/
void CWorldShow::EnterOtherSpace(){
	FACEPOS fc = m_CurDypass->face;
//	CDoorWall* dw = m_CurSpace->Space.m_DoorWall[fc];
//	assert(dw);
//	dw->m_State &= ~SPACE_OPENDOOR;
    m_PreDypass = m_CurDypass;

    SpaceView* OtherSpace = GetOtherSpace(m_CurSpace);
	m_CurSpace = OtherSpace;
        
    m_CurDypass = NULL;
	//得到新当前空间对应入口过道
	if(fc == FRONTFACE){
		  m_CurDypass = &m_CurSpace->Space.m_Dypass[BACKFACE];
	}else if (fc == BACKFACE){
		  m_CurDypass = &m_CurSpace->Space.m_Dypass[FRONTFACE];
	}else if (fc == LEFTFACE){
 		  m_CurDypass = &m_CurSpace->Space.m_Dypass[RIGHTFACE];
	}else{ //RIGHTFASE
		  m_CurDypass = &m_CurSpace->Space.m_Dypass[LEFTFACE];
	};
    assert(m_CurDypass);
//	fc = m_CurDypass->face;
//	dw = m_CurSpace->Space.m_DoorWall[fc];
//	assert(dw);
//	dw->m_State &= ~SPACE_OPENDOOR;		
	
	m_SpaceTitle.SetText(m_CurSpace->Space.m_Name,false,SS.m_Font24);
	SendParentMessage(CM_GOTOSPACE,m_CurSpace->Space.m_Alias,0,NULL); //重置map当前条目
};

CDoorWall* CWorldShow::GetOtherDoor(SpaceView* Space, mapunit* Dypass){
    assert(Space);
	assert(Dypass);
	CDoorWall* dw = NULL;
	SpaceView* OtherSpace = GetOtherSpace(Space);
	assert(OtherSpace);
	
	if(Dypass->face == FRONTFACE){
		    dw = OtherSpace->Space.m_DoorWall[BACKFACE];
	}else if (Dypass->face == BACKFACE){
		    dw = OtherSpace->Space.m_DoorWall[FRONTFACE];
	}else if (Dypass->face == LEFTFACE){
 		    dw = OtherSpace->Space.m_DoorWall[RIGHTFACE];
	}else{ //RIGHTFASE
		    dw = OtherSpace->Space.m_DoorWall[LEFTFACE];
	};
    assert(dw);
	return dw;
}
void CWorldShow::CloseDoor(){
	
	CDoorWall* dw = m_CurSpace->Space.m_DoorWall[m_CurDypass->face];
	assert(dw);
	dw->m_State &= ~SPACE_OPENDOOR;
	
	if (m_PreDypass)
	{
       SpaceView* Space = GetOtherSpace(m_CurSpace);
	   dw = Space->Space.m_DoorWall[m_PreDypass->face];
	   assert(dw);
	   dw->m_State &= ~SPACE_OPENDOOR;
	}
	m_CurDypass = NULL;
	m_PreDypass = NULL;
	m_ShowBoth = false;
}

int CWorldShow::LoadGLTexture(const TCHAR* Bmp) //返回纹理号
{
        int TextNum = -1;
	    AUX_RGBImageRec* TextureImage[1];               // Create Storage Space For The Texture
        TextureImage[0] = NULL;

        TextureImage[0] = LoadBMP(Bmp);
		if (!TextureImage[0])return -1;

		GLuint texture[1];
		glGenTextures(1, &texture[0]);          // Create Three Textures
		
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
        TextNum = texture[0];
				
		if (TextureImage[0]->data)              // If Texture Image Exists
		{
			free(TextureImage[0]->data);    // Free The Texture Image Memory
		}

		free(TextureImage[0]);                  // Free The Image Structure

        return TextNum;                                  // Return The Status	
};
    
int CWorldShow::LoadGLTextureFromRes(uint32 ResHandle){
	HBITMAP hBMP;									
	BITMAP	BMP;   

	GLuint texture[1];
	glGenTextures(1, &texture[0]);							
	
	hBMP=(HBITMAP)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(ResHandle), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	
	if (hBMP)		
	{				

		::GetObjectW(hBMP,sizeof(BMP), &BMP);
		glPixelStorei(GL_UNPACK_ALIGNMENT,4);				
		glBindTexture(GL_TEXTURE_2D, texture[0]);			
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); 

		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, BMP.bmWidth, BMP.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
		::DeleteObject(hBMP);						
	}
	return texture[0];
}



void CWorldShow::ConnectSpaceFail(tstring Reason){
	ClearAllObject(m_CurSpace->Space.m_Alias);
	m_SpaceTitle.SetText(Reason,false,SS.m_Font24);
}

void  CWorldShow::SetSpaceTitle(tstring Title){
	m_SpaceTitle.SetText(Title,false,SS.m_Font24);
	Invalidate();
}

void  CWorldShow::SetStatusText(tstring s){
	m_StatusInfo.SetText(s,false,SS.m_Font24);
}

void CWorldShow::AddObject(int64 ParentID,int64 ID,tstring& Name,SPACETYPE Type,tstring& Fingerprint,HICON hIcon){
	ObjectItem* Item = new ObjectItem(ID,Name,Type,Fingerprint,hIcon);
	//SpaceView* UpdataView = GetUpdataSpace(ParentID);
	if (GetHwnd())
	{
		SendChildMessage(GetHwnd(),OBJECT_ADD,(int64)Item,ParentID);
	} 
	else
	{
		SpaceRectionMsg srm;
		srm.Msg = OBJECT_ADD;
		srm.wParam = (int64)Item;
		srm.lParam = ParentID;
		ChildReaction(&srm);
	}
};
    
int32 CWorldShow::FindObject(tstring& Name, vector<ObjectItem>& SpaceList){
	if(GetHwnd()){
		SendChildMessage(m_CurSpace->ObjectView.GetHwnd(),OBJECT_FIND,(int64)&Name,(int64)&SpaceList);
		if (SpaceList.size())  //可能出现用户转换当前空间
		{
			SpaceView* View = GetOtherSpace(m_CurSpace);
			SendChildMessage(m_CurSpace->ObjectView.GetHwnd(),OBJECT_FIND,(int64)&Name,(int64)&SpaceList);
		}
	}else{
		m_CurSpace->ObjectView.OnFindObject((WPARAM)&Name,(LPARAM)&SpaceList);
		if(SpaceList.size()){
			SpaceView* View = GetOtherSpace(m_CurSpace);
			View->ObjectView.OnFindObject((WPARAM)&Name,(LPARAM)&SpaceList);
		}
	}
	return SpaceList.size();
}

void CWorldShow::DeleteObject(tstring Fingerprint){
	if (GetHwnd())
	{
		SendChildMessage(GetHwnd(),OBJECT_DEL,(int64)&Fingerprint,0);
	} 
	else
	{
		SpaceRectionMsg srm;
		srm.Msg = OBJECT_DEL;
		srm.wParam = (int64)&Fingerprint;
		srm.lParam = 0;
		ChildReaction(&srm);
	}
};
 
void CWorldShow::ClearAllObject(int64 ParentID){
	if (GetHwnd())
	{
		SendChildMessage(GetHwnd(),OBJECT_CLR,0,ParentID);
	} 
	else
	{
		SpaceRectionMsg srm;
		srm.Msg = OBJECT_CLR;
		srm.wParam = 0;
		srm.lParam = ParentID;
		ChildReaction(&srm);
	}
};

LRESULT CWorldShow::SendChildMessage(HWND Child,UINT Msg,int64 wParam, int64 lParam){
	assert(Child);
	
	SpaceRectionMsg SRM;
	
	SRM.Msg = Msg;
	SRM.wParam = wParam;
	SRM.lParam = lParam;
	SRM.ChildAffected = NULL;
	SRM.WinSpace = this;
	
#ifdef _USE_THREAD_REANDER
	if(GetHwnd() == Child)
		SendLocalMessage(WM_CHILDRECTION,(WPARAM)&SRM,0);
		return 0;
	else 
        return ::SendMessage(Child,WM_CHILDRECTION,(WPARAM)&SRM,0);
#else
	return ::SendMessage(Child,WM_CHILDRECTION,(WPARAM)&SRM,0);
#endif		
}

LRESULT CWorldShow::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
    if (message == WM_TIMER)
	{
		 return OnTimer(wParam,lParam);
	}
	else  if (message == WM_KEYDOWN)
    {
#ifdef _USE_THREAD_RENDER
		SendLocalMessage(WM_KEYDOWN,wParam,lParam,NULL);
#else
		return OnKeydown(wParam, lParam);
#endif
    }else if(message == WM_KEYUP){
#ifdef _USE_THREAD_RENDER
		SendLocalMessage(WM_KEYUP,wParam,lParam,NULL);
#else
		return OnKeyUp(wParam, lParam);
#endif		
	}else if(message == WM_MOUSEMOVE){
		OnMouseMove(wParam, lParam);
	}else if (message == WM_CREATE){
		return OnCreate(wParam, lParam);
	}else if (message == WM_RBUTTONDOWN)
	{
		return OnRButtonDown(wParam,lParam);
	}else if (message == WM_LBUTTONDOWN)
	{
		return OnLButtonDown(wParam,lParam);
	}
	else{
#ifdef _USE_THREAD_RENDER
        return CTGLWin::Reaction(message,wParam,lParam);		
#else
		return CGLWin::Reaction(message,wParam,lParam);		
#endif
	} 
	return 0;
};

#ifdef _USE_THREAD_RENDER

void CWorldShow::GLReaction(){
	SpaceRectionMsg SRM;
    
	while (GetLocalMessage(SRM)) //如果不一次处理所有消息，当绘制不及时可能导致键盘消息累积，产生控制延迟
	{
		if (SRM.Msg == WM_SIZE)
		{
			DWORD lParam = SRM.lParam;
			OnGLSceneSize(LOWORD(lParam),HIWORD(lParam));
		}else if (SRM.Msg == WM_KEYDOWN)
		{
			OnKeydown(SRM.wParam,SRM.lParam);
		}else if (SRM.Msg == WM_KEYUP)
		{
			OnKeyUp(SRM.wParam,SRM.lParam);
		}else if (SRM.Msg == WM_CHILDRECTION)
		{
			ChildReaction((SpaceRectionMsg*)wParam);
		}
	}
};
#endif

LRESULT CWorldShow::OnCreate(WPARAM wParam, LPARAM lParam){
    //必须在CGLWin::OnCreate之前初始化好子窗口
	RECT rc={0};
	HINSTANCE AppInstance = GetHinstance();
	if(!m_Space1.ObjectView.Create(AppInstance,NULL,WS_CHILD,rc,GetHwnd(),10))return -1;
	if(!m_Space2.ObjectView.Create(AppInstance,NULL,WS_CHILD,rc,GetHwnd(),11))return -1;

	if(RegisterToolClass(AppInstance)==FALSE)return -1;
	

    if(!m_NaviBar.Create(WS_CHILD|WS_CLIPCHILDREN,rc,GetHwnd()))return -1;
    
 
#ifdef _USE_THREAD_RENDER
        if(CTGLWin::OnCreate(wParam,lParam)==-1)return -1;		
#else
		if(CGLWin::OnCreate(wParam,lParam) ==-1)return -1;;		
#endif	

	return 0;
};
	


LRESULT CWorldShow::OnKeydown(WPARAM wParam, LPARAM lParam){
	
	if(m_WhoUpdating == m_CurSpace->Space.m_Alias)return 0;
			
	if (wParam == VK_UP ){
		m_Key = wParam;
	    //计算当前速度，以及根据最新速度将移动距离
		m_MoveTime++;
		m_MoveSpeed += m_MoveAcceleration*m_MoveTime; 
		if(m_MoveSpeed>10)m_MoveSpeed=10;
		int32 r1 = m_MoveSpeed*m_MoveTime; //这里是整数，3D坐标系统中应该至少除以100
			
		GoAhread(r1);

	}else if( wParam == VK_DOWN ){
		m_Key = wParam;
	    //计算当前速度，以及根据最新速度将移动距离
		m_MoveTime++;
		m_MoveSpeed += m_MoveAcceleration*m_MoveTime; 
		if(m_MoveSpeed>10)m_MoveSpeed=10;
		int32 r1 = m_MoveSpeed*m_MoveTime; //这里是整数，3D坐标系统中应该至少除以100
			
		GoBack(r1);
		
	}else if(wParam == VK_RIGHT){
		m_Key = wParam;
	    //计算当前速度，以及根据最新速度将移动距离
		m_MoveTime++;
		m_MoveSpeed += m_MoveAcceleration*m_MoveTime; 
		if(m_MoveSpeed>10)m_MoveSpeed=10;
		int32 r1 = m_MoveSpeed*m_MoveTime; //这里是整数，3D坐标系统中应该至少除以100
			
		float  yRot = (float)r1/100.0f;
		m_yrot += -yRot;
		m_heading = m_yrot;
		Invalidate();
	}else if(wParam == VK_LEFT){
		m_Key = wParam;
	    //计算当前速度，以及根据最新速度将移动距离
		m_MoveTime++;
		m_MoveSpeed += m_MoveAcceleration*m_MoveTime; 
		if(m_MoveSpeed>10)m_MoveSpeed=10;
		int32 r1 = m_MoveSpeed*m_MoveTime; //这里是整数，3D坐标系统中应该至少除以100
			
		float  yRot = (float)r1/100.0f;
		m_yrot += yRot;
		m_heading = m_yrot;
		Invalidate();
	}
    return 0;
};
 
LRESULT CWorldShow::OnKeyUp(WPARAM wParam, LPARAM lParam){
    if(m_Key){
		m_MoveTime = 0;
		m_Key = 0;
	}
	return 0;
};

LRESULT CWorldShow::OnRButtonDown(WPARAM wParam, LPARAM lParam){
	::ShowWindow(m_NaviBar.GetHwnd(),SW_HIDE);
	m_CurSpace->ObjectView.Show();
	SetFocus(m_CurSpace->ObjectView.GetHwnd());
	return 0;
}

LRESULT CWorldShow::OnLButtonDown(WPARAM wParam, LPARAM lParam){
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;	
	CVSpace2*  Space = Hit(point);
	if(Space==NULL)return 0;

	if( Space->m_Alias == BNT_MAP){
		SendParentMessage(CM_OPENVIEW,BNT_MAP,0,NULL);

	}else if (Space->m_Alias == BNT_CREATEROOM){
	    m_CurSpace->ObjectView.OnCreateChildSpace(FALSE);
	}else if (Space->m_Alias == BNT_IMPORTOBJECT){
        //test AddObject(12,"Test.dll",OBJECT_SPACE,SS.m_DefaultConvIcon,12345,"DSFEFT");
		m_CurSpace->ObjectView.OnImportObject();
	}
	return 0;
}

LRESULT CWorldShow::OnMouseMove(WPARAM wParam, LPARAM lParam){
	::SetFocus(GetHwnd());
	if (m_NaviBarState == NAVIBAR_INIT)
	{
        RECT rc;
		::GetClientRect(GetHwnd(),&rc);
		int x = RectWidth(rc)/2;
		int y = RectHeight(rc)/2;
		m_NaviBar.MoveTo(x,y);

		POINT p; 
        p.x = x; p.y = y;
		::ClientToScreen(GetHwnd(),&p);
		::SetCursorPos(p.x,p.y);
        m_NaviBarState = NAVIBAR_STOP;
		
	}else if(m_NaviBarState == NAVIBAR_STOP){
       	POINT p; 	    
		::GetCursorPos(&p);	
		::ScreenToClient(GetHwnd(),&p);
	
		int dx = p.x-m_NaviBar.m_x;
		int dy = p.y-m_NaviBar.m_y;
		int32 d = sqrt((float)(dx*dx) + (float)(dy*dy));

		if(d>m_NaviDistance && m_SpaceFocused==NULL){
			m_NaviBarState = NAVIBAR_MOVE;
			::SetTimer(GetHwnd(),ID_TIMER_NAVI,30,NULL);
		}
	}
	return 0;
}
	
LRESULT CWorldShow::OnTimer(WPARAM wParam, LPARAM lParam){
     if(wParam == ID_TIMER_NAVI){
		POINT p; 	    
		::GetCursorPos(&p);	
		::ScreenToClient(GetHwnd(),&p);

		RECT rc; 
		::GetClientRect(GetHwnd(),&rc);
		if (::PtInRect(&rc,p)==FALSE)
		{
			m_NaviMoveTime = 0;
			m_NaviMoveSpeed = 0;
			m_NaviBarState = NAVIBAR_STOP;
			::KillTimer(GetHwnd(),ID_TIMER_NAVI);
            return 0;
		}

		int32 dx = p.x-m_NaviBar.m_x;
		int32 dy = p.y-m_NaviBar.m_y;
        int32 r = sqrt((float)(dx*dx) + (float)(dy*dy));
		if(r>m_NaviDistance){

            //计算当前速度，以及根据最新速度将移动距离
			m_NaviMoveTime++;
			m_NaviMoveSpeed += m_NaviMoveAcceleration*m_NaviMoveTime; 
			if(m_NaviMoveSpeed>30)m_NaviMoveSpeed=30;
		    int32 r1 = m_NaviMoveSpeed*m_NaviMoveTime; //向光标方向前进了这么多
			if(r-r1<m_NaviDistance)r1=r-m_NaviDistance;

			//根据将移动的距离找到对应的坐标点
			//sin(a)= y/r= (y0-y1)/r = y?-y1/r1   y? = (y0-y1)*r1/r+y1
            //cos(a)= x/r= (x0-x1)/r = x?-x1/r1   x? = (x0-x1)*r1/r+x1;
            int32 x = (dx)*r1/r+m_NaviBar.m_x;
			int32 y = (dy)*r1/r+m_NaviBar.m_y;
			m_NaviBar.MoveTo(x,y);

		}else{
			m_NaviMoveTime = 0;
			m_NaviMoveSpeed = 0;
			m_NaviBarState = NAVIBAR_STOP;
			::KillTimer(GetHwnd(),ID_TIMER_NAVI);
		}
	 }
	 return 0;
}


LRESULT CWorldShow::ChildReaction(SpaceRectionMsg* srm){
	  
	wglMakeCurrent(m_DC, m_RC);

	switch(srm->Msg)
	{
	case OBJECT_ADD:
		{
			SpaceView* UpdataView = GetUpdataSpace(srm->lParam);
			ObjectItem* Item = (ObjectItem*)srm->wParam;

			UpdataView->ObjectView.OnAddObject((WPARAM)Item,0);
			if(Item->m_Type == LOCAL_SPACE){
				int index = UpdataView->Space.AddChildSpace(Item->m_Alias,Item->m_Name,Item->m_Type,Item->m_Fingerprint);
				if (index != -1 && UpdataView==m_CurSpace && m_CurDypass == NULL){//观察者自动转向新加入的物体：
					float x2,y2;
					if(index == FRONTFACE){
						x2 =0.0f; 
						y2 =1.0f;
					}else if(index == LEFTFACE){ 
						x2 = -1.0f;
						y2 = 0.0f;
					}else if (index== BACKFACE) { 
						x2 = 0.0f;
						y2 = -1.0f;
					}else if (index == RIGHTFACE){ 
						x2 = 1.0f;
						y2 = 0.0f;
					}
					
					//根据当前场景旋转得到一个向量  (初始观察者面相90°)
					float x1 = 1.0f*cos((m_heading+90)*piover180);
					float y1 = 1.0f*sin((m_heading+90)*piover180);

					double roty = getRotateAngle(x1, y1,x2, y2);
                    	
					m_heading+= roty;
					m_yrot=m_heading;
                    Invalidate();
				}
			}else{

				mapunit* mp = UpdataView->Space.AddChildObject(Item->m_Alias,Item->m_Name,Item->m_Type,Item->m_hIcon,Item->m_Fingerprint);
				if (mp){
					if(UpdataView==m_CurSpace && m_CurDypass == NULL)
					{   //观察者自动转向新加入的物体:   
						//1 根据当前场景旋转得到一个向量  (初始观察者面相90°)
						float x1 = 1.0f*cos((m_heading+90)*piover180);
						float y1 = 1.0f*sin((m_heading+90)*piover180);
						
						//2 根据物体重心得到另一个向量
						float cgx = mp->x1 + (mp->x2-mp->x1)/2;
						float cgz = mp->z1 + (mp->z2-mp->z1)/2;
						cgz = -cgz; //这里的z实际上是y轴，但它向下为正，所以必须调过
						
						//3计算旋转角
						double roty = getRotateAngle(x1, y1,cgx, cgz);
						//4 旋转场景
						m_heading+= roty;
						m_yrot=m_heading;
						Invalidate();
					}
				}
			}
		}
		break;
	case OBJECT_DEL:
		{
		LRESULT ret = m_CurSpace->ObjectView.OnDeleteObject(srm->wParam,0);
		if(ret){
			 tstring Fingerprint = *(tstring*)srm->wParam;
		     m_CurSpace->Space.DeleteChildObject(Fingerprint);
		}
        //让另个房间数据无效，因为可能删除的正是那个房间。 
		SpaceView* View = GetOtherSpace(m_CurSpace);
		View->Clear();
		View->Space.m_Alias = 0;
		View->Space.m_DataValid = false;
		}
		break;
	case OBJECT_CLR:
		{
		SpaceView* UpdataView = GetUpdataSpace(srm->lParam);
        UpdataView->Clear();
		}
	    break;
	}
     
	wglMakeCurrent(m_DC, m_RC);
	return 0;	
};

void CWorldShow::GoAhread(int32 s){
	//暂存改变之前的位置
	float x0 = m_xpos;
	float z0 = m_zpos;
	
	float r = (float)s/1000.0f;
	//新位置
	m_xpos -= (float)sin(m_heading*piover180) * r;
	m_zpos -= (float)cos(m_heading*piover180) * r;
	if (m_walkbiasangle >= 359.0f)m_walkbiasangle = 0.0f;
	else m_walkbiasangle+= 10;
	
	m_walkbias = (float)sin(m_walkbiasangle * piover180)/20.0f;
	
	RoamingWorld(x0,z0);
	Invalidate();
};

void CWorldShow::GoBack(int32 s){
	//暂存改变之前的位置
	float x0 = m_xpos;
	float z0 = m_zpos;
	
	float r = (float)s/1000.0f;
	
	m_xpos += (float)sin(m_heading*piover180) * r;
	m_zpos += (float)cos(m_heading*piover180) * r;
	if (m_walkbiasangle <= 1.0f)m_walkbiasangle = 359.0f;
	else m_walkbiasangle-= 10;
	
	m_walkbias = (float)sin(m_walkbiasangle * piover180)/20.0f;
	RoamingWorld(x0,z0); 
	Invalidate();
};

LRESULT CWorldShow::ParentReaction(SpaceRectionMsg* SRM){
	if (SRM->Msg == CM_WORLDTRUN)
	{
		if(m_WhoUpdating == m_CurSpace->Space.m_Alias)return 0;

		float  yRot = (float)SRM->wParam/100.0f;
		float  xRot = (float)SRM->lParam/100.0f;

		m_yrot += -yRot;
		m_xrot += -xRot;
		m_heading = m_yrot;

        if(m_xrot>30)m_xrot = 30;
		if(m_xrot<-30)m_xrot = -30;

		Invalidate();

	}else if (SRM->Msg == CM_WORLDMOVE)
	{
		if(m_WhoUpdating == m_CurSpace->Space.m_Alias)return 0;

		if (SRM->wParam == ID_GOAHREAD)
		{
            GoAhread(SRM->lParam);
		}else if(SRM->wParam == ID_GOBACK){
            GoBack(SRM->lParam);
		}
		Invalidate();
	}
	return 0;
}

void CWorldShow::Draw3D(float32* ParentMatrix /*=NULL*/,ePipeline* Pipe /*=NULL*/){
  
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	glColor3ub(255,255,255);

 	GLfloat xtrans = -m_xpos;
	GLfloat ztrans = -m_zpos;
	GLfloat ytrans = -m_walkbias-0.25f;
	GLfloat sceneroty = 360.0f - m_yrot;
 
	glLoadIdentity();		// Reset The Current Modelview Matrix


	//注意旋转次序，绕X轴应该场景旋转后的点头运动，必须放在最后
	//由于矩阵乘法的结果是反序,所以必须先于旋转roty，否则场景将看起来绕Z轴左右摇摆倾斜
	if(m_xrot){   
		glRotatef(m_xrot, 1.0f,0.0f,0.0f);
	}

	if(sceneroty){
		glRotatef(sceneroty, 0.0f,1.0f,0.0f);
	}
	
 	glTranslatef(xtrans, ytrans,ztrans); 
	
	GLfloat  WorldTrans[16];
	glGetFloatv(GL_MODELVIEW_MATRIX,WorldTrans);

	if(m_ShowBoth){
		GLfloat  W[16];
		::memcpy(W,WorldTrans,sizeof(W));	
		m_Space1.Space.Draw(W);

		::memcpy(W,WorldTrans,sizeof(W));
		m_Space2.Space.Draw(W);
    }else{ 
		m_CurSpace->Space.Draw(WorldTrans);
	}
	
	Draw2DAll();

}
//在OpenGL窗口上输出字符函数:
GLvoid glPrint(GLint x, GLint y, int set,const char *fmt , ...)	// Where The Printing Happens
{
	char		string[256];							// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf(string, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);	

	if (set>1)
	{
		set=1;
	}
//	glBindTexture(GL_TEXTURE_2D, Map.GetFixTexture(0));	// Select Our Font Texture
	glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPushMatrix();										// Store The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glOrtho(0,640,0,480,-1,1);							// Set Up An Ortho Screen
	glPushMatrix();										// Store The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glEnable(GL_BLEND);
	glTranslated(x,y,0);								// Position The Text (0,0 - Bottom Left)
//	glListBase(Map.fontlistbase-32+(128*set));						// Choose The Font Set (0 or 1)
_glCDefaultFont2D& font =  GetDefaultFont2D();
		
	glListBase(font.m_DefaultFontBase);						// Choose The Font Set (0 or 1)

	glCallLists(strlen(string),GL_UNSIGNED_BYTE,string);// Write The Text To The Screen
	glDisable(GL_BLEND);
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
}
void CWorldShow::Draw2DAll(){

//	glPrint(100,00,0,"test");

    RECT rcClient;
    ::GetClientRect(GetHwnd(),&rcClient);
    int h = RectHeight(rcClient);
    int w = RectWidth(rcClient);

	glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPushMatrix();										// Store The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glOrtho(0,w,0,h,-1,1);							// Set Up An Ortho Screen
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glPushMatrix();										// Store The Modelview Matrix
	glLoadIdentity();				
 
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_SRC_ALPHA);
		
        m_SpaceTitle.Draw3D(NULL);
		
#ifdef _USE_THREAD_RENDER
        CTGLWin::Draw3D(NULL,NULL);
#else 
        CGLWin::Draw3D(NULL,NULL);
#endif	 	

		m_StatusInfo.Draw3D(NULL);		
		glDisable(GL_BLEND);
   
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glEnable(GL_DEPTH_TEST);			

}


void CWorldShow::Layout(bool Redraw){
    RECT rcClient;
    ::GetClientRect(GetHwnd(),&rcClient);
    int h = RectHeight(rcClient);
    int w = RectWidth(rcClient);
	
	glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPushMatrix();										// Store The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glOrtho(0,w,0,h,-1,1);							// Set Up An Ortho Screen
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glPushMatrix();										// Store The Modelview Matrix
	glLoadIdentity();				


	m_SpaceTitle.SetArea(10,10,280,26);
	m_SpaceTitle.Layout();

	m_StatusInfo.SetArea(10,30,280,50);
    m_StatusInfo.Layout();

    
    ButtonItem3*  BntMap = (ButtonItem3*)m_ChildList[0];
	ButtonItem3*  BntCreateSpace = (ButtonItem3*)m_ChildList[1];
	ButtonItem3*  BntImportObject = (ButtonItem3*)m_ChildList[2];

	/* 右对齐
	BntImportObject->SetArea(w-96,h-22,w-10,h-6);
	BntImportObject->Layout();
	
	BntCreateSpace->SetArea(w-190,h-22,w-106,h-6);
	BntCreateSpace->Layout();

    BntMap->SetArea(w-260,h-22,w-200,h-6);
	BntMap->Layout();
    */
 	//左对齐
	BntMap->SetArea(10,h-22,56,h-6);
	BntMap->Layout();
	
	BntCreateSpace->SetArea(66,h-22,152,h-6);
	BntCreateSpace->Layout();

    BntImportObject->SetArea(172,h-22,248,h-6);
	BntImportObject->Layout();
	

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glEnable(GL_DEPTH_TEST);	

}

