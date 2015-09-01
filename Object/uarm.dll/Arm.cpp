#include "Arm.h"
#include <boost/bind.hpp>
#include <math.h>
#include "TaskDialog.h"
#include "ThinDataTransProtocal.h"
CArm::CArm(){
	
	m_ID = 0;

	m_PortName = "COM3";  //defualt
	m_pSerialPort = NULL;

	m_L1 = 0.15;
	m_L2 = 0.16;

	double pi = 3.1415926;
	int i;
	for (i=30;i<175; i++)  //计算左舵机在移动范围内后臂每度坐标
	{
		double a = (double)(180.0f-i)*pi/180.0f;  //以机械臂前伸为正向
		double x = cos(a)*m_L1;
		double y = sin(a)*m_L1;

		ArmPos pos(x,y);
		m_L1PosList.push_back(pos);
	}

	for (i=0; i<90;i++) //计算右舵机在移动范围内前臂每度坐标
	{
		double a = (double)(90.0f-i)*pi/180.0f;  //注意，i只是舵机角度，90-i才是前臂的真实角度
		double x = sin(a)*m_L2;
		double y = cos(a)*m_L2;

		ArmPos pos(x,y);
		m_L2PosList.push_back(pos);
	}

	//机械臂初始位置
	m_LeftServoAngle  = 45;
	m_RightServoAngle  = 20;
	m_RotServoAngle = 0;

	GetPos(m_LeftServoAngle,m_RightServoAngle,m_x,m_y);

};

CArm::~CArm(){
	if (m_pSerialPort)
	{
		CloseSerialPort();
		delete m_pSerialPort;
		m_pSerialPort = NULL;
	}
};

bool CArm::GetPos(int LeftServoAngle,int RightServoAngle,double& x,double& y){
	if(LeftServoAngle<30 || LeftServoAngle>175){
		return false;
	}
	
	if(RightServoAngle<0 ||  RightServoAngle>90){
		return false;
	}

	if((LeftServoAngle-RightServoAngle)<15){
		return false;
	}

	x = m_L1PosList[LeftServoAngle-30].x+  m_L2PosList[RightServoAngle].x;
	y = m_L1PosList[LeftServoAngle-30].y - m_L2PosList[RightServoAngle].y;
	return true;
}

bool CArm::Pos2Angle(double DestX,double DestY,double& LeftServoAngle, double& RightServoAngle){
	
	for (int i=30; i<175; i++)
	{
		for (int j=0; j<90;j++)
		{
			double x,y;
			GetPos(i,j,x,y);
			
			if (abs(x-DestX)<=0.002f && abs(y-DestY)<=0.002f)
			{
				LeftServoAngle  = i;
				RightServoAngle = j;
				return true;
			}
		}

	}
	return false;
}
bool CArm::Pos2Angle(double DestX,double DestY,double& LeftServoAngle, double& RightServoAngle,double OldLeftServoAngle,double OldRightServoAngle,double LastDistance){
	if(OldLeftServoAngle<30.0f || OldLeftServoAngle >175.0f){
		return false;
	}
	if(OldRightServoAngle<0 || OldRightServoAngle >90){
		return false;
	}
	if((OldLeftServoAngle-OldRightServoAngle)<15){
		return false;
	}

	double x1,y1;

	double d1=100,d2=100,d3=100,d4=100; //预置一个不可能的极大值

	//计算四个可能的移动方向上与目标点的距离，但省略开平方这一步
	
		
	if(GetPos(OldLeftServoAngle,OldRightServoAngle+1,x1,y1)){	
		d1 = (DestX-x1)*(DestX-x1)-(DestY-y1)*(DestY-y1); 
	}

	if (GetPos(OldLeftServoAngle,OldRightServoAngle-1,x1,y1))
	{
		d2 = (DestX-x1)*(DestX-x1)-(DestY-y1)*(DestY-y1);
	}

	if(GetPos(OldLeftServoAngle+1,OldRightServoAngle,x1,y1)){
		d3 = (DestX-x1)*(DestX-x1)-(DestY-y1)*(DestY-y1);
	}

	if(GetPos(OldLeftServoAngle-1,OldRightServoAngle,x1,y1)){
		d4 = (DestX-x1)*(DestX-x1)-(DestY-y1)*(DestY-y1);
	}

	if(d1<= d2 && d1<=d3 && d1<=d4){ //如果d1最小

	    LeftServoAngle= OldLeftServoAngle;
		RightServoAngle= OldRightServoAngle+1;

		if (d1>LastDistance)  //比之前的距离还大，则表示没找到
		{
			return false;
		}
		//计算实际距离
		if (d1< 0.0001 || sqrt(d1) < 0.002f) //距离在2毫米则算成功
		{
			return true;
		}
		//否则继续找
		return Pos2Angle(DestX,DestY,LeftServoAngle,RightServoAngle,LeftServoAngle,RightServoAngle,d1);
	}
	else if(d2<= d1 && d2<=d3 && d2<=d4){ //如果d2最小

		LeftServoAngle= OldLeftServoAngle;
		RightServoAngle= OldRightServoAngle-1;

		if (d2>LastDistance)  //比之前的距离还大，则表示没找到
		{
			return false;
		}
		//计算实际距离
		if (d2< 0.0001 || sqrt(d2) < 0.002f) //距离在2毫米则算成功
		{
			return true;
		}
		//否则继续找
		return Pos2Angle(DestX,DestY,LeftServoAngle,RightServoAngle,LeftServoAngle,RightServoAngle,d2);
	} 
	else if(d3<= d1 && d3<=d2 && d3<=d4){ //如果d3最小

		LeftServoAngle= OldLeftServoAngle+1;
		RightServoAngle= OldRightServoAngle;

		if (d3>LastDistance)  //比之前的距离还大，则表示没找到
		{
			return false;
		}

		if (d3< 0.0001 || sqrt(d3)< 0.002f) //距离在2毫米则算成功
		{
			return true;
		}
		//否则继续找
		return Pos2Angle(DestX,DestY,LeftServoAngle,RightServoAngle,LeftServoAngle,RightServoAngle,d3);
	}
	else { //如果d4最小

		LeftServoAngle= OldLeftServoAngle+1;
		RightServoAngle= OldRightServoAngle;

		if (d4>LastDistance)  //比之前的距离还大，则表示没找到
		{
			return false;
		}
		if (d4< 0.0001 ||sqrt(d4) < 0.002f) //距离在2毫米则算成功
		{
			return true;
		}
		//否则继续找
		return Pos2Angle(DestX,DestY,LeftServoAngle,RightServoAngle,LeftServoAngle,RightServoAngle,d4);
	} 

	return false;
};

bool CArm::OpenSerialPort(const char* PortName){
	try
	{	
		m_PortName = PortName;
		if(!m_pSerialPort){
			m_pSerialPort = new serial_port(m_ioService );
			if(!m_pSerialPort){
				return false;
			}
		}
		if (m_pSerialPort->is_open())
		{
			m_pSerialPort->close();
		}

		m_PortName = PortName;
		m_pSerialPort->open(m_PortName, m_ErrorCode );
		if(m_ErrorCode!=0){
			return false;
		}

		m_pSerialPort->set_option( serial_port::baud_rate( 9600 ), m_ErrorCode );
		m_pSerialPort->set_option( serial_port::flow_control( serial_port::flow_control::none ), m_ErrorCode );//奇怪，只要加了此句，官方的鼠标控制就失效,不加，失效后又不能正常打开
		m_pSerialPort->set_option( serial_port::parity( serial_port::parity::none ), m_ErrorCode );
		m_pSerialPort->set_option( serial_port::stop_bits( serial_port::stop_bits::one ), m_ErrorCode);
		m_pSerialPort->set_option( serial_port::character_size( 8 ), m_ErrorCode);


	}
	catch (...)
	{
		return false;
	}
	return m_ErrorCode==0;
}
void CArm::CloseSerialPort(){
	if(m_pSerialPort){
		m_pSerialPort->close();	
	}	
};

void CArm::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred )
{
	int n=0;
}

void  CArm::SendToArm(char* buf,int size)
{
	if (m_pSerialPort)
	{
		uint8 header[2];
		header[0]=0xFF;
		header[1]=0xBB;

		size_t len = write( *m_pSerialPort, buffer(header,2), m_ErrorCode );
		assert(len==2);
		
		len = write( *m_pSerialPort, buffer(buf,size), m_ErrorCode );
		assert(len == size);
	}
}
void  CArm::ReadMsgFromArm(){
	if (m_pSerialPort)
	{
		/*异步方式
		async_read( *m_pSerialPort, buffer(m_ReadBuffer,11), 
		boost::bind( &CArm::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred) );  
		*/

		//同步+定时器方式
		::memset(&m_ReadBuffer,0,100);

		int n=0;
		char buf[100];
		do{
			size_t len = m_pSerialPort->read_some(boost::asio::buffer(buf), m_ErrorCode);
			for (int i=0; i<len & n<100; i++)
			{
				m_ReadBuffer[n++] = buf[i];
			}
		}while(n < 100);

	}
}

bool CArm::GetArmPosture(CTaskDialog* Dialog,ePipeline* Pipe){
	
	int cmd = CMD_GET_POSTURE;

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}

	int32 ArmID;

	pos=0;
	pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ArmID);
	assert(pos);

	int code;
	pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,code);
	assert(pos);

	int HanRotAngle;
	pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,HanRotAngle);
	assert(pos);

	int ArmRotAngle;
	pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ArmRotAngle);
	assert(pos);

	int LeftAngle;
	pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,LeftAngle);
	assert(pos);

	int RightAngle;
	pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,RightAngle);
	assert(pos);


	Dialog->RuntimeOutput(0,_T("HanRotAngle: %d   ArmRotAngle:%d	LeftAngle:%d   RightAngle:%d"),HanRotAngle,ArmRotAngle,LeftAngle,RightAngle);

	Pipe->PushInt(HanRotAngle);
	Pipe->PushInt(ArmRotAngle);
	Pipe->PushInt(LeftAngle);
	Pipe->PushInt(RightAngle);

	return true;
};

bool CArm::SetArmPosture(CTaskDialog* Dialog,ePipeline* Pipe)
{

	if (!Pipe->HasTypeAB(0x11110000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x11110000"));
		return false;
	}		

	int cmd = CMD_SET_POSTURE;

	int64 HanRotAngle = Pipe->PopInt();
	int64 ArmRotAngle = Pipe->PopInt();
	int64 LeftAngle = Pipe->PopInt();
	int64 RightAngle = Pipe->PopInt();

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,HanRotAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,ArmRotAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,LeftAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,RightAngle);
	assert(pos);

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
};

bool CArm::Left(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		

	int cmd = CMD_ARM_POS;

	int64 RotAngle = Pipe->PopInt();
	RotAngle = m_RotServoAngle-RotAngle;
	if(RotAngle<-90 || RotAngle>90){
		Alert(Dialog,1);
		Dialog->RuntimeOutput(_T("The arm can not reach the specified position"));

		return true;
	}
	m_RotServoAngle=RotAngle;

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_RotServoAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_LeftServoAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_RightServoAngle);
	assert(pos);

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
};

bool CArm::Right(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		
	int cmd = CMD_ARM_POS;

	int64 RotAngle = Pipe->PopInt();

	RotAngle = m_RotServoAngle+RotAngle;
	if(RotAngle<-90 || RotAngle>90){
		Alert(Dialog,1);
		Dialog->RuntimeOutput(_T("The arm can not reach the specified position"));

		return true;
	}
	
	m_RotServoAngle = RotAngle;

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_RotServoAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_LeftServoAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_RightServoAngle);
	assert(pos);

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复
	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CArm::Up(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		

	int cmd = CMD_ARM_POS;

	int64 Distance = Pipe->PopInt();

	//厘米换成米
	double dy = Distance;
	dy = dy/100.0f;

	double LeftAngle,RightAngle;
	bool ret = Pos2Angle(m_x,m_y+dy,LeftAngle,RightAngle);
	if (!ret)
	{
		Alert(Dialog,1);
		Dialog->RuntimeOutput(_T("The arm can not reach the specified position"));
		return true;
	}

	m_y = m_y+dy;
	m_LeftServoAngle  = LeftAngle;
	m_RightServoAngle = RightAngle;


	char buf[100];
	int pos=0;
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_RotServoAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,LeftAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,RightAngle);
	assert(pos);

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;

}

bool CArm::Down(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		
	int cmd = CMD_ARM_POS;

	int64 Distance = Pipe->PopInt();

	//厘米换成米
	double dy = Distance;
	dy = dy/100.0f;

	double LeftAngle,RightAngle;
	bool ret = Pos2Angle(m_x,m_y-dy,LeftAngle,RightAngle);
	if (!ret)
	{
		Alert(Dialog,1);
		Dialog->RuntimeOutput(_T("The arm can not reach the specified position"));

		return true;
	}
	m_y = m_y-dy;
	m_LeftServoAngle  = LeftAngle;
	m_RightServoAngle = RightAngle;


	char buf[100];
	int pos=0;
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_RotServoAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,LeftAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,RightAngle);
	assert(pos);

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CArm::Forward(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		
	int cmd = CMD_ARM_POS;

	int64 Distance = Pipe->PopInt();

	double LeftAngle,RightAngle;

	//厘米换成米
	double dx = Distance;
	dx = dx/100.0f;


	bool ret = Pos2Angle(m_x+dx,m_y,LeftAngle,RightAngle);

	if (!ret)
	{
		Alert(Dialog,1);
		Dialog->RuntimeOutput(_T("The arm can not reach the specified position"));

		return true;
	}

	m_x = m_x+dx;
	m_LeftServoAngle  = LeftAngle;
	m_RightServoAngle = RightAngle;


	char buf[100];
	int pos=0;
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_RotServoAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,LeftAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,RightAngle);
	assert(pos);

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}

	return true;

}

bool CArm::Back(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		
	int cmd = CMD_ARM_POS;

	int64 Distance = Pipe->PopInt();

	//厘米换成米
	double dx = Distance;
	dx = dx/100.0f;

	double LeftAngle,RightAngle;

	bool ret = Pos2Angle(m_x-dx,m_y,LeftAngle,RightAngle);
	if (!ret)
	{
		Alert(Dialog,1);
		Dialog->RuntimeOutput(_T("The arm can not reach the specified position"));

		return true;
	}

	m_x = m_x-dx;
	m_LeftServoAngle  = LeftAngle;
	m_RightServoAngle = RightAngle;

	char buf[100];
	int pos=0;
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,m_RotServoAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,LeftAngle);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,RightAngle);
	assert(pos);

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复
	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CArm::HandRot(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}	

	int HandRotAngle = Pipe->PopInt();
	if(HandRotAngle<-85 || HandRotAngle>85){
		Alert(Dialog,1);
		Dialog->RuntimeOutput(_T("The arm can not reach the specified position"));

		return true;
	}

	int cmd = CMD_HAND_ROT;
	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 
	pos = TDTP_INT_ToString(buf,100,pos,HandRotAngle);

	SendToArm(buf,100);
	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CArm::HandGrip(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		

	int bGrip = Pipe->PopInt();

	int cmd = CMD_HAND_GRIP;
	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 

	pos = TDTP_INT_ToString(buf,100,pos,bGrip);

	SendToArm(buf,100);
	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}

	return true;

}
bool CArm::HandState(CTaskDialog* Dialog,ePipeline* Pipe){
	int cmd = CMD_HAND_STATE;

	int pos=0;

	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复
	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}else{

		int32 ArmID;

		int pos=0;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ArmID);
		assert(pos);

		int code;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,code);
		assert(pos);

		int Grip=0;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,Grip);
		assert(pos);

		tstring s;
		if(Grip==2){
			s = _T("Hand is released");
			Pipe->PushInt(0);
		}else if(Grip == 1){
			Pipe->PushInt(1);
			s = _T("Hand is griping");
		}
		Dialog->RuntimeOutput(s);
	}
	return true;
}

bool CArm::SetAngle(CTaskDialog* Dialog,ePipeline* Pipe){

	if (!Pipe->HasTypeAB(0x11000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x11000000"));
		return false;
	}

	int cmd = CMD_SET_ANGLE;
	int ServoID = Pipe->PopInt();
	int Angle   = Pipe->PopInt();

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 
	pos = TDTP_INT_ToString(buf,100,pos,ServoID);
	pos = TDTP_INT_ToString(buf,100,pos,Angle);
	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
};
bool CArm::GetAngle(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		

	int ServoID = Pipe->PopInt();

	int cmd = CMD_GET_ANGLE;

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 
	pos = TDTP_INT_ToString(buf,100,pos,ServoID);

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}else{

		int32 ArmID;

		int pos=0;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ArmID);
		assert(pos);

		int code;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,code);
		assert(pos);

		int Angle;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,Angle);
		assert(pos);

		Dialog->RuntimeOutput(0,_T("ServoID: %d   Angle:%d"),ServoID,Angle);

		Pipe->PushInt(Angle);
	}
	return true;
}
bool CArm::Alert(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x11100000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x11100000"));
		return false;
	}		

	int cmd = CMD_ALERT;
	int times = Pipe->PopInt();
	int runtimes = Pipe->PopInt();
	int interval = Pipe->PopInt();


	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 
	pos = TDTP_INT_ToString(buf,100,pos,times);
	pos = TDTP_INT_ToString(buf,100,pos,runtimes);
	pos = TDTP_INT_ToString(buf,100,pos,interval);

	SendToArm(buf,100);
	ReadMsgFromArm(); //等待回复
	return true;
}

void CArm::Alert(CTaskDialog* Dialog,int n)
{
	ePipeline Pipe;
	Pipe.PushInt(n);
	Pipe.PushInt(50);
	Pipe.PushInt(50);
	Alert(Dialog,&Pipe);
}
bool CArm::Reset(CTaskDialog* Dialog,ePipeline* Pipe){
	
	//重置手臂姿态
	int cmd = CMD_ARM_POS;
	m_LeftServoAngle  = 45;
	m_RightServoAngle  = 20;
	m_RotServoAngle = 0;

	GetPos(m_LeftServoAngle,m_RightServoAngle,m_x,m_y);

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 
	pos = TDTP_INT_ToString(buf,100,pos,m_RotServoAngle);
	pos = TDTP_INT_ToString(buf,100,pos,m_LeftServoAngle);
	pos = TDTP_INT_ToString(buf,100,pos,m_RightServoAngle);

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复
	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}

	//重置手ROT
	cmd = CMD_HAND_ROT;
	pos=0;
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 

	pos = TDTP_INT_ToString(buf,100,pos,0);

	SendToArm(buf,100);
	ReadMsgFromArm(); //等待回复

	return true;
}
bool CArm::Calibrate(CTaskDialog* Dialog,ePipeline* Pipe){

	int cmd = CMD_CALIBRATE;

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 

	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复

	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}else{
		int32 LeftServoBase = 0;
		int32 RightServoBase = 0;
		int32 RotServoBase = 0;
		int32 HandRotServoBase = 0;

		int32 ArmID;

		int pos=0;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ArmID);
		assert(pos);

		int code;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,code);
		assert(pos);

		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,LeftServoBase);
		assert(pos);

		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,RightServoBase);
		assert(pos);

		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,RotServoBase);
		assert(pos);
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,HandRotServoBase);
		assert(pos);

		tstring s = Format1024(_T("Left Servo Base:%d\nRight Servo Base:%d \nRot Servo Base:%d\nHand Rot Servo Base:%d\n"),LeftServoBase,RightServoBase,RotServoBase,HandRotServoBase);
		Dialog->RuntimeOutput(s);
	}

	return true;
}

bool CArm::AttachServo(CTaskDialog* Dialog,ePipeline* Pipe,bool bAttach){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		

	int cmd = CMD_ATTACH_SERVO;

	int ServoID = Pipe->PopInt();
	
	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 
	pos = TDTP_INT_ToString(buf,100,pos,ServoID);
	pos = TDTP_INT_ToString(buf,100,pos,bAttach?1:0);

	SendToArm(buf,100);
	ReadMsgFromArm(); 
	return true;
}

bool CArm::SetID(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		

	int cmd = CMD_SET_ID;

	int NewID = Pipe->PopInt();
	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 
	pos = TDTP_INT_ToString(buf,100,pos,NewID); 
	SendToArm(buf,100);


	ReadMsgFromArm(); //等待回复
	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}else{

		m_ID = NewID;

		int32 ArmID;
		int pos=0;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ArmID);
		assert(pos);
		assert(ArmID==m_ID);

		int code;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,code);
		assert(pos);
	}
	return true;
}

bool CArm::SetServoSpeed(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x11000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x11000000"));
		return false;
	}		

	int cmd = CMD_SET_SPEED;

	int ServoID = Pipe->PopInt();
	int Speed   = Pipe->PopInt();

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 
	pos = TDTP_INT_ToString(buf,100,pos,ServoID); 
	pos = TDTP_INT_ToString(buf,100,pos,Speed); 
	SendToArm(buf,100);

	ReadMsgFromArm(); //等待回复
	
	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CArm::GetInfo(CTaskDialog* Dialog,ePipeline* Pipe){
	if (!Pipe->HasTypeAB(0x10000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		

	int cmd = CMD_INFO;

	int InfoID = Pipe->PopInt();

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 
	pos = TDTP_INT_ToString(buf,100,pos,InfoID); 

	SendToArm(buf,100);


	ReadMsgFromArm(); //等待回复
	if (m_ErrorCode == boost::asio::error::eof)
	{
		//连接中断
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}else{

		int32 ArmID;
		pos=0;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ArmID);
		assert(pos);
		assert(ArmID==m_ID);

		int code;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,code);
		assert(pos);

		int ID=0;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ID);
		assert(pos);


		int Value;
		pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,Value);

		tstring s = FormatArmInfo(InfoID,Value);	
		Dialog->RuntimeOutput(s);
	}
	return true;
}
bool CArm::GetAllInfo(CTaskDialog* Dialog,ePipeline* Pipe){
	tstring s;
	for (int i=1; i<14; i++)
	{
		int cmd = CMD_INFO;

		int pos=0;
		char buf[100];
		pos = TDTP_INT_ToString(buf,100,pos,m_ID);
		pos = TDTP_INT_ToString(buf,100,pos,cmd); 
		pos = TDTP_INT_ToString(buf,100,pos,i);  //info id 

		SendToArm(buf,100);
		ReadMsgFromArm(); //等待回复

		if (m_ErrorCode == boost::asio::error::eof)
		{
			//连接中断
			AnsiString error = m_ErrorCode.message();
			tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
			Pipe->SetLabel(s.c_str());
			return false;
		}
		else if (m_ErrorCode){
			AnsiString error = m_ErrorCode.message();
			tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
			Pipe->SetLabel(s.c_str());

			return false;
		}else{

			int32 ArmID;
			pos=0;
			pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ArmID);
			assert(pos);
			assert(ArmID==m_ID);

			int code;
			pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,code);
			assert(pos);

			int ID=0;
			pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,ID);
			assert(pos);

			int Value;
			pos = TDTP_INT_FromString(m_ReadBuffer,100,pos,Value);

			s += FormatArmInfo(ID,Value);	
		}
	}
	Dialog->RuntimeOutput(s);
	return true;
}

bool CArm::OpenPort(CTaskDialog* Dialog,ePipeline* Pipe){

	if (!Pipe->HasTypeAB(0x30000000))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x30000000"));
		return false;
	}		
	
	tstring PortName = Pipe->PopString();

	AnsiString Name = eSTRING::WStoUTF8(PortName);
	if(!OpenSerialPort(Name.c_str())){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("Open serial port %s fail:%s"),PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	};
	return true;
}

bool CArm::Do(CTaskDialog* Dialog,ePipeline* Pipe){

	if (!Pipe->HasTypeAB(0x30000000)) //第一个数据必须TYPE_STRING
	{
		return false;
	}

	tstring ActionName = Pipe->PopString();
	transform(ActionName.begin(),ActionName.end(),ActionName.begin(), ::tolower);

	if (!m_pSerialPort || !m_pSerialPort->is_open())
	{
		if (ActionName == _T("close_port"))
		{
			return true;
		}

		if (ActionName != _T("open_port"))
		{
			bool ret = OpenSerialPort(m_PortName.c_str());
			if (!ret)
			{
				AnsiString error = m_ErrorCode.message();
				tstring s = Format1024(_T("Open serial port %s fail:%s"),m_PortName.c_str(),eSTRING::UTF8toWS(error).c_str());
				Pipe->SetLabel(s.c_str());
				return false;
			}
		}
	}



	bool ret=true;
	if (ActionName == _T("get_posture"))
	{
		ret = GetArmPosture(Dialog,Pipe);
	}else if (ActionName == _T("set_posture"))
	{
		ret = SetArmPosture(Dialog,Pipe);
	}
	else if (ActionName == _T("left"))
	{
		ret = Left(Dialog,Pipe);
	}else if (ActionName == _T("right"))
	{
		ret = Right(Dialog,Pipe);
	}
	else if (ActionName == _T("up"))
	{
		ret = Up(Dialog,Pipe);
	}
	else if (ActionName == _T("down"))
	{
		ret = Down(Dialog,Pipe);
	}
	else if (ActionName == _T("forward"))
	{
		ret = Forward(Dialog,Pipe);
	}
	else if (ActionName == _T("back"))
	{
		ret = Back(Dialog,Pipe);
	}
	else if (ActionName == _T("hand_rot"))
	{	
		ret = HandRot(Dialog,Pipe);

	}else if (ActionName == _T("hand_grip"))
	{
		ret = HandGrip(Dialog,Pipe);
	}
	else if(ActionName == _T("hand_state"))
	{
		ret = HandState(Dialog,Pipe);
	}
	else if (ActionName == _T("set_angle"))
	{
		ret = SetAngle(Dialog,Pipe);
	}
	else if(ActionName == _T("get_angle"))
	{
		ret = GetAngle(Dialog,Pipe);
	} 
	else if (ActionName == _T("alert"))
	{
		ret = Alert(Dialog,Pipe);
	}else if (ActionName == _T("reset"))
	{
		ret = Reset(Dialog,Pipe);
	}
	else if(ActionName == _T("calibrate"))
	{
		ret = Calibrate(Dialog,Pipe);
	}else if (ActionName == _T("detach"))
	{
		ret = AttachServo(Dialog,Pipe,false);
	}else if (ActionName == _T("attach"))
	{
		ret = AttachServo(Dialog,Pipe,true);
	}
	else if(ActionName == _T("set_id"))
	{
		ret = SetID(Dialog,Pipe);
	}else if (ActionName == _T("set_speed"))
	{
		ret = SetServoSpeed(Dialog,Pipe);
	}
	else if(ActionName == _T("get_info"))
	{
		ret = GetInfo(Dialog,Pipe);
	}
	else if (ActionName == _T("all_info"))
	{
		ret = GetAllInfo(Dialog,Pipe);
	}
	else if (ActionName == _T("open_port"))
	{
		ret = OpenPort(Dialog,Pipe);
	}else if (ActionName == _T("close_port"))
	{
		CloseSerialPort();		
	}
	return ret;
};

tstring CArm::FormatArmInfo(int InfoID,int Value){
	tstring s;
	switch(InfoID)
	{

	case 1:
		s+=Format1024(_T("ID: %d\n"),Value);
		break;
	case 2:
		s+=Format1024(_T("Mode:	%d\n"),Value);
		break;
	case 3:
		s+=Format1024(_T("offsetL: %d\n"),Value);
		break;
	case 4:
		s+=Format1024(_T("offsetR: %d\n"),Value);
		break;
	case 5:
		s+=Format1024(_T("LeftServoBase: %d\n"),Value);
		break;
	case 6:
		s+=Format1024(_T("RightServoBase: %d\n"),Value);
		break;
	case 7:
		s+=Format1024(_T("RotServoBase: %d\n"),Value);
		break;
	case 8:
		s+=Format1024(_T("HandRotServoBase: %d\n"),Value);
		break;
	case 9:
		s+=Format1024(_T("heightLst: %d\n"),Value);
		break;
	case 10:
		s+=Format1024(_T("height: %d\n"),Value);
		break;
	case 11:
		s+=Format1024(_T("stretch: %d\n"),Value);
		break;
	case 12:
		s+=Format1024(_T("rotation: %d\n"),Value);
		break;
	case 13:
		s+=Format1024(_T("handRot: %d\n"),Value);
		break;
	default:
		break;
	}
	return s;
}
