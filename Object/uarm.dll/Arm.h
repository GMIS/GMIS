#ifndef _GMIS_ARM_H_
#define _GMIS_ARM_H_

#include "AbstractSpace.h"
#include "UserTimer.h"


#include <boost/asio.hpp>  
#include <boost/asio/serial_port.hpp> 

using namespace boost::asio;

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

class CTaskDialog;
class CArm
{
	struct ArmPos{
		double x;
		double y;

		ArmPos(double x1,double y1)
			:x(x1),y(y1){
		}
	};


	int             m_ID;
	io_service      m_ioService; 
	serial_port*    m_pSerialPort;
	boost::system::error_code m_ErrorCode;

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
	void  SendToArm(char* buf,int size);
	void  ReadMsgFromArm();
	bool GetArmPosture(CTaskDialog* Dialog,ePipeline* Pipe);
	bool SetArmPosture(CTaskDialog* Dialog,ePipeline* Pipe);
	bool Left(CTaskDialog* Dialog,ePipeline* Pipe);
	bool Right(CTaskDialog* Dialog,ePipeline* Pipe);
	bool Up(CTaskDialog* Dialog,ePipeline* Pipe);
	bool Down(CTaskDialog* Dialog,ePipeline* Pipe);
	bool Forward(CTaskDialog* Dialog,ePipeline* Pipe);
	bool Back(CTaskDialog* Dialog,ePipeline* Pipe);
	bool HandRot(CTaskDialog* Dialog,ePipeline* Pipe);
	bool HandGrip(CTaskDialog* Dialog,ePipeline* Pipe);
	bool HandState(CTaskDialog* Dialog,ePipeline* Pipe);
	bool SetAngle(CTaskDialog* Dialog,ePipeline* Pipe);
	bool GetAngle(CTaskDialog* Dialog,ePipeline* Pipe);
	bool Alert(CTaskDialog* Dialog,ePipeline* Pipe);
	bool Reset(CTaskDialog* Dialog,ePipeline* Pipe);
	bool Calibrate(CTaskDialog* Dialog,ePipeline* Pipe);
	bool AttachServo(CTaskDialog* Dialog,ePipeline* Pipe,bool bAttach);
	bool SetID(CTaskDialog* Dialog,ePipeline* Pipe);
	bool SetServoSpeed(CTaskDialog* Dialog,ePipeline* Pipe);
	bool GetInfo(CTaskDialog* Dialog,ePipeline* Pipe);
	bool GetAllInfo(CTaskDialog* Dialog,ePipeline* Pipe);
	bool OpenPort(CTaskDialog* Dialog,ePipeline* Pipe);

	tstring FormatArmInfo(int InfoID,int Value);

	void Alert(CTaskDialog* Dialog,int n); //n=发生次数

	bool GetPos(int LeftServoAngle,int RightServoAngle,double& x,double& y);
	/*
	根据目标坐标DestX和DestY，以及之前的左右舵机角度，寻找新的左右舵机转动角度
	LastDisttance是上次寻找距离，用来递归寻找时确定寻找方向
	*/
	bool	Pos2Angle(double DestX,double DestY,double& LeftServoAngle, double& RightServoAngle,double OldLeftServoAngle,double OldRightServoAngle,double LastDistance);
	bool	Pos2Angle(double DestX,double DestY,double& LeftServoAngle, double& RightServoAngle);

	void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred );  
public:
	CArm();
	virtual ~CArm();

	bool OpenSerialPort(const char* PortName);
	void CloseSerialPort();
	
	virtual bool Do(CTaskDialog* Dialog,ePipeline* ExePipe);
};



#endif  //_GMIS_ARM_H_