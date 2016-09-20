#ifndef _UARM_OBJECT_H
#define _UARM_OBJECT_H

#include "..\ObjectList.h"

#ifdef _COMPILE_UARM_OBJECT

#include <boost/asio.hpp>

#include "Mass.h"
#include "Pipeline.h"
#include <tchar.h>

#define CMD_GET_POSTURE  99
#define CMD_SET_POSTURE  100
#define CMD_ARM_POS		 101
#define CMD_HAND_ROT	 107
#define CMD_HAND_GRIP	 108
#define CMD_HAND_STATE	 109
#define CMD_SET_ANGLE    110
#define CMD_GET_ANGLE    111
#define CMD_ALERT		 112
#define CMD_CALIBRATE	 113
#define CMD_SET_ID       114
#define CMD_INFO         115
#define CMD_ATTACH_SERVO 116
#define CMD_SET_SPEED    117

using namespace boost::asio;

int QueryIdleCOM(list<tstring>& COM_List);

class CUArmObject : public Mass
{
public:
	static const TCHAR* UserManual;/* = __TEXT(\
		"\
		1 cmd：open_port\n\
		Function：打开一个串行端口\n\
		Input：字符串（端口名字），缺省打开端口COM3\n\
		Output：null\n\
		2 cmd：pos\n\
		Function：设置机械臂的姿态数据\n\
		Input：输入姿态数据，可以按顺序输入一项或多项:臂旋转(-90 ~ 90),伸缩(0 ~ 210),高低(-180 ~ 150),手旋转(-90 ~ 90), \n\
		Output：null\n\
		3 Cmd：grap\n\
		Function：手抓紧\n\
		Input：null\n\
		Output：null\n\
		4 Cmd：release\n\
		Function：手松开\n\
		Input：null\n\
		Output：null\n\
		5 Cmd：left\n\
		Function：让机械臂左旋转\n\
		Input：一个整数（旋转角度）\n\
		Output：null\n\
		6 Cmd：right\n\
		Function：让机械臂右旋转\n\
		Input：一个整数（旋转角度）\n\
		Output：null\n\
		7 Cmd：up\n\
		Function：升高机械臂\n\
		Input：一个整数（升高角度）\n\
		Output：null\n\
		8 Cmd：down\n\
		Function：降低机械臂\n\
		Input：一个整数（降低角度）\n\
		Output：null\n\
		9 Cmd：goahead\n\
		Function：伸出机械臂\n\
		Input：一个整数（伸展角度）\n\
		Output：null\n\
		10 Cmd：goback\n\
		Function：缩回机械臂\n\
		Input：一个整数（缩回角度）\n\
		Output：null\n\
		11 Cmd：hand_left\n\
		Function：手向左选择\n\
		Input：一个整数（旋转角度）\n\
		Output：null\n\
		12 Cmd：hand_right\n\
		Function：手像右旋转\n\
		Input：一个整数（旋转角度）\n\
		Output：null\n\
		13 Cmd：query_port\n\
		Function：查询空闲的串口\n\
		Input：null\n\
		Output：string(窗口名字）\n\
		"\
		);
		*/
	struct ArmPos{
		double x;
		double y;

		ArmPos(double x1,double y1)
			:x(x1),y(y1){
		}
	};

protected:
	tstring         m_Name;

	io_service      m_ioService; 
	serial_port*    m_pSerialPort;
	boost::system::error_code      m_ErrorCode;

	AnsiString      m_PortName; // default = "COM3";

	vector<ArmPos>  m_L1PosList;  //后臂可以移动范围内坐标值
	vector<ArmPos>  m_L2PosList;  //前臂可以移动范围内坐标值

	double          m_L1_Base;    //后臂
	double          m_L2_Base;

	double          m_LeftServoAngle;   //对应控制的是后臂
	double          m_RightServoAngle;  //对应控制的是前臂
	double          m_RotServoAngle;    

	double          m_x;   //当前机械臂手的位置坐标
	double          m_y;

	double          m_L1;  //后臂长
	double          m_L2;  //前臂长

	char            m_ReadBuffer[100];

protected:
	void SendToArm(char* buf,int size);
	void ReadMsgFromArm();
	void QueryPort(ePipeline* Pipe);
	bool GetArmPosture(ePipeline* Pipe);
	bool SetArmPosture(ePipeline* Pipe);
	bool Left(ePipeline* Pipe);
	bool Right(ePipeline* Pipe);
	bool Up(ePipeline* Pipe);
	bool Down(ePipeline* Pipe);
	bool Forward(ePipeline* Pipe);
	bool Back(ePipeline* Pipe);
	bool HandRot(ePipeline* Pipe);
	bool HandGrip(ePipeline* Pipe);
	bool HandState(ePipeline* Pipe);
	bool SetAngle(ePipeline* Pipe);
	bool GetAngle(ePipeline* Pipe);
	bool Alert(ePipeline* Pipe);
	bool Reset(ePipeline* Pipe);
	bool Calibrate(ePipeline* Pipe);
	bool AttachServo(ePipeline* Pipe,bool bAttach);
	bool SetID(ePipeline* Pipe);
	bool SetServoSpeed(ePipeline* Pipe);
	bool GetInfo(ePipeline* Pipe);
	bool GetAllInfo(ePipeline* Pipe);
	bool OpenPort(ePipeline* Pipe);

	tstring FormatArmInfo(int InfoID,int Value);

	void Alert(int n); //n=发生次数

	bool GetPos(int LeftServoAngle,int RightServoAngle,double& x,double& y);
	/*
	根据目标坐标DestX和DestY，以及之前的左右舵机角度，寻找新的左右舵机转动角度
	LastDisttance是上次寻找距离，用来递归寻找时确定寻找方向
	*/
	bool	Pos2Angle(double DestX,double DestY,double& LeftServoAngle, double& RightServoAngle,double OldLeftServoAngle,double OldRightServoAngle,double LastDistance);
	bool	Pos2Angle(double DestX,double DestY,double& LeftServoAngle, double& RightServoAngle);

//	void handle_read(const asio::error_code& ec, std::size_t bytes_transferred );  

public:
	CUArmObject(tstring Name,int64 ID);
	virtual ~CUArmObject();

	bool OpenSerialPort(const char* PortName);
	void CloseSerialPort();

	virtual tstring GetName();
	virtual TypeAB GetTypeAB();
	virtual bool Do(Energy* E);
	
	/*如果Object的执行依赖其内部状态信息则应该实现下面两个函数
      virtual uint32  ToString(string& s,uint32 pos =0) 
	  virtual uint32  FromString(string& s,uint32 pos=0) 
    */
};

#endif //_COMPILE_UARM_OBJECT
#endif //_UARM_OBJECT_H