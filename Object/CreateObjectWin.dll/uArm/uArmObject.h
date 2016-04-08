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
		1 cmd��open_port\n\
		Function����һ�����ж˿�\n\
		Input���ַ������˿����֣���ȱʡ�򿪶˿�COM3\n\
		Output��null\n\
		2 cmd��pos\n\
		Function�����û�е�۵���̬����\n\
		Input��������̬���ݣ����԰�˳������һ������:����ת(-90 ~ 90),����(0 ~ 210),�ߵ�(-180 ~ 150),����ת(-90 ~ 90), \n\
		Output��null\n\
		3 Cmd��grap\n\
		Function����ץ��\n\
		Input��null\n\
		Output��null\n\
		4 Cmd��release\n\
		Function�����ɿ�\n\
		Input��null\n\
		Output��null\n\
		5 Cmd��left\n\
		Function���û�е������ת\n\
		Input��һ����������ת�Ƕȣ�\n\
		Output��null\n\
		6 Cmd��right\n\
		Function���û�е������ת\n\
		Input��һ����������ת�Ƕȣ�\n\
		Output��null\n\
		7 Cmd��up\n\
		Function�����߻�е��\n\
		Input��һ�����������߽Ƕȣ�\n\
		Output��null\n\
		8 Cmd��down\n\
		Function�����ͻ�е��\n\
		Input��һ�����������ͽǶȣ�\n\
		Output��null\n\
		9 Cmd��goahead\n\
		Function�������е��\n\
		Input��һ����������չ�Ƕȣ�\n\
		Output��null\n\
		10 Cmd��goback\n\
		Function�����ػ�е��\n\
		Input��һ�����������ؽǶȣ�\n\
		Output��null\n\
		11 Cmd��hand_left\n\
		Function��������ѡ��\n\
		Input��һ����������ת�Ƕȣ�\n\
		Output��null\n\
		12 Cmd��hand_right\n\
		Function����������ת\n\
		Input��һ����������ת�Ƕȣ�\n\
		Output��null\n\
		13 Cmd��query_port\n\
		Function����ѯ���еĴ���\n\
		Input��null\n\
		Output��string(�������֣�\n\
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

	vector<ArmPos>  m_L1PosList;  //��ۿ����ƶ���Χ������ֵ
	vector<ArmPos>  m_L2PosList;  //ǰ�ۿ����ƶ���Χ������ֵ

	double          m_L1_Base;    //���
	double          m_L2_Base;

	double          m_LeftServoAngle;   //��Ӧ���Ƶ��Ǻ��
	double          m_RightServoAngle;  //��Ӧ���Ƶ���ǰ��
	double          m_RotServoAngle;    

	double          m_x;   //��ǰ��е���ֵ�λ������
	double          m_y;

	double          m_L1;  //��۳�
	double          m_L2;  //ǰ�۳�

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

	void Alert(int n); //n=��������

	bool GetPos(int LeftServoAngle,int RightServoAngle,double& x,double& y);
	/*
	����Ŀ������DestX��DestY���Լ�֮ǰ�����Ҷ���Ƕȣ�Ѱ���µ����Ҷ��ת���Ƕ�
	LastDisttance���ϴ�Ѱ�Ҿ��룬�����ݹ�Ѱ��ʱȷ��Ѱ�ҷ���
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
	
	/*���Object��ִ���������ڲ�״̬��Ϣ��Ӧ��ʵ��������������
      virtual uint32  ToString(string& s,uint32 pos =0) 
	  virtual uint32  FromString(string& s,uint32 pos=0) 
    */
};

#endif //_COMPILE_UARM_OBJECT
#endif //_UARM_OBJECT_H