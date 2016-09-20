#include "uArmObject.h"
#include "ThinDataTransProtocal.h"
#include <math.h>
#include <algorithm>

#ifdef _COMPILE_UARM_OBJECT


#define MAX_KEY_LENGTH   255
#define MAX_VALUE_NAME  16383

int QueryIdleCOM(list<tstring>& COM_List){

	HKEY hKey;
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hKey) ){
		TCHAR   achClass[MAX_PATH] = TEXT(""); // buffer for class name
		DWORD   cchClassName = MAX_PATH; // size of class string
		DWORD   cSubKeys=0;                  // number of subkeys
		DWORD   cbMaxSubKey;             // longest subkey size
		DWORD   cchMaxClass;             // longest class string
		DWORD   cValues;             // number of values for key
		DWORD   cchMaxValue;         // longest value name
		DWORD   cbMaxValueData;      // longest value data
		DWORD   cbSecurityDescriptor; // size of security descriptor
		FILETIME ftLastWriteTime;     // last write time

		DWORD i, retCode;

		// Get the class name and the value count.
		retCode = RegQueryInfoKey(
			hKey,                   // key handle
			achClass,               // buffer for class name
			&cchClassName,          // size of class string
			NULL,                   // reserved
			&cSubKeys,              // number of subkeys
			&cbMaxSubKey,           // longest subkey size
			&cchMaxClass,           // longest class string
			&cValues,               // number of values for this key
			&cchMaxValue,           // longest value name
			&cbMaxValueData,        // longest value data
			&cbSecurityDescriptor,  // security descriptor
			&ftLastWriteTime);      // last write time

		if (cValues > 0) {
			TCHAR szPortName[25]; 
			TCHAR achValue[MAX_VALUE_NAME];	

			DWORD Type;
			for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) {

				achValue[0] = '\0';
				DWORD cchValue = MAX_VALUE_NAME;
				DWORD dwSizeofPortName = sizeof(szPortName);

				if (ERROR_SUCCESS == RegEnumValue(hKey, i, achValue, &cchValue, NULL, &Type, (UCHAR*)szPortName, &dwSizeofPortName)) {
					tstring com = szPortName;
					COM_List.push_back(com);	
				}
			}
		}
		else{
			return 0;
		}
	}
	RegCloseKey(hKey);
	int n = COM_List.size();
	return n;
}

const TCHAR* CUArmObject::UserManual = __TEXT(\
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

//////////////////////////////////////////////////////////////////////////
CUArmObject::CUArmObject(tstring Name,int64 ID)
	:Mass(ID){

		m_PortName = "";  
		m_pSerialPort = NULL;

		m_L1 = 0.15;
		m_L2 = 0.16;

		double pi = 3.1415926;
		int i;
		for (i=30;i<175; i++)  //�����������ƶ���Χ�ں��ÿ������
		{
			double a = (double)(180.0f-i)*pi/180.0f;  //�Ի�е��ǰ��Ϊ����
			double x = cos(a)*m_L1;
			double y = sin(a)*m_L1;

			ArmPos pos(x,y);
			m_L1PosList.push_back(pos);
		}

		for (i=0; i<90;i++) //�����Ҷ�����ƶ���Χ��ǰ��ÿ������
		{
			double a = (double)(90.0f-i)*pi/180.0f;  //ע�⣬iֻ�Ƕ���Ƕȣ�90-i����ǰ�۵���ʵ�Ƕ�
			double x = sin(a)*m_L2;
			double y = cos(a)*m_L2;

			ArmPos pos(x,y);
			m_L2PosList.push_back(pos);
		}

		//��е�۳�ʼλ��
		m_LeftServoAngle  = 45;
		m_RightServoAngle  = 20;
		m_RotServoAngle = 0;

		GetPos(m_LeftServoAngle,m_RightServoAngle,m_x,m_y);

};

CUArmObject::~CUArmObject(){
	if (m_pSerialPort)
	{
		CloseSerialPort();
		delete m_pSerialPort;
		m_pSerialPort = NULL;
	}
};

tstring CUArmObject::GetName(){
	return m_Name;
};

TypeAB CUArmObject::GetTypeAB(){
	return 0x00000000;
}

bool CUArmObject::GetPos(int LeftServoAngle,int RightServoAngle,double& x,double& y){
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

bool CUArmObject::Pos2Angle(double DestX,double DestY,double& LeftServoAngle, double& RightServoAngle){
	
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
bool CUArmObject::Pos2Angle(double DestX,double DestY,double& LeftServoAngle, double& RightServoAngle,double OldLeftServoAngle,double OldRightServoAngle,double LastDistance){
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

	double d1=100,d2=100,d3=100,d4=100; //Ԥ��һ�������ܵļ���ֵ

	//�����ĸ����ܵ��ƶ���������Ŀ���ľ��룬��ʡ�Կ�ƽ����һ��
	
		
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

	if(d1<= d2 && d1<=d3 && d1<=d4){ //���d1��С

	    LeftServoAngle= OldLeftServoAngle;
		RightServoAngle= OldRightServoAngle+1;

		if (d1>LastDistance)  //��֮ǰ�ľ��뻹�����ʾû�ҵ�
		{
			return false;
		}
		//����ʵ�ʾ���
		if (d1< 0.0001 || sqrt(d1) < 0.002f) //������2��������ɹ�
		{
			return true;
		}
		//���������
		return Pos2Angle(DestX,DestY,LeftServoAngle,RightServoAngle,LeftServoAngle,RightServoAngle,d1);
	}
	else if(d2<= d1 && d2<=d3 && d2<=d4){ //���d2��С

		LeftServoAngle= OldLeftServoAngle;
		RightServoAngle= OldRightServoAngle-1;

		if (d2>LastDistance)  //��֮ǰ�ľ��뻹�����ʾû�ҵ�
		{
			return false;
		}
		//����ʵ�ʾ���
		if (d2< 0.0001 || sqrt(d2) < 0.002f) //������2��������ɹ�
		{
			return true;
		}
		//���������
		return Pos2Angle(DestX,DestY,LeftServoAngle,RightServoAngle,LeftServoAngle,RightServoAngle,d2);
	} 
	else if(d3<= d1 && d3<=d2 && d3<=d4){ //���d3��С

		LeftServoAngle= OldLeftServoAngle+1;
		RightServoAngle= OldRightServoAngle;

		if (d3>LastDistance)  //��֮ǰ�ľ��뻹�����ʾû�ҵ�
		{
			return false;
		}

		if (d3< 0.0001 || sqrt(d3)< 0.002f) //������2��������ɹ�
		{
			return true;
		}
		//���������
		return Pos2Angle(DestX,DestY,LeftServoAngle,RightServoAngle,LeftServoAngle,RightServoAngle,d3);
	}
	else { //���d4��С

		LeftServoAngle= OldLeftServoAngle+1;
		RightServoAngle= OldRightServoAngle;

		if (d4>LastDistance)  //��֮ǰ�ľ��뻹�����ʾû�ҵ�
		{
			return false;
		}
		if (d4< 0.0001 ||sqrt(d4) < 0.002f) //������2��������ɹ�
		{
			return true;
		}
		//���������
		return Pos2Angle(DestX,DestY,LeftServoAngle,RightServoAngle,LeftServoAngle,RightServoAngle,d4);
	} 

	return false;
};

bool CUArmObject::OpenSerialPort(const char* PortName){
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
		if(m_ErrorCode.value()!=0){
			return false;
		}

		m_pSerialPort->set_option( serial_port::baud_rate( 9600 ), m_ErrorCode );
		m_pSerialPort->set_option( serial_port::flow_control( serial_port::flow_control::none ), m_ErrorCode );
		m_pSerialPort->set_option( serial_port::parity( serial_port::parity::none ), m_ErrorCode );
		m_pSerialPort->set_option( serial_port::stop_bits( serial_port::stop_bits::one ), m_ErrorCode);
		m_pSerialPort->set_option( serial_port::character_size( 8 ), m_ErrorCode);


	}
	catch (...)
	{
		return false;
	}
	return m_ErrorCode.value()==0;
}
void CUArmObject::CloseSerialPort(){
	if(m_pSerialPort){
		m_pSerialPort->close();	
	}	
};

/*
void CUArmObject::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred )
{
	int n=0;
}
*/
void  CUArmObject::SendToArm(char* buf,int size)
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
void  CUArmObject::ReadMsgFromArm(){
	if (m_pSerialPort)
	{
		/*�첽��ʽ
		async_read( *m_pSerialPort, buffer(m_ReadBuffer,11), 
		boost::bind( &CUArmObject::handle_read, this, asio::placeholders::error, asio::placeholders::bytes_transferred) );  
		*/

		//ͬ��+��ʱ����ʽ
		::memset(&m_ReadBuffer,0,100);

		int n=0;
		char buf[100];
		do{
			size_t len = m_pSerialPort->read_some(buffer(buf), m_ErrorCode);
			for (int i=0; i<len & n<100; i++)
			{
				m_ReadBuffer[n++] = buf[i];
			}
		}while(n < 100);

	}
}


void CUArmObject::QueryPort(ePipeline* Pipe){
	list<tstring> com_list;
	int n = QueryIdleCOM(com_list);
	if (n==0)
	{
		Pipe->PushString(_T("no available serial port"));
		return;
	}
	for(int i=0; i<com_list.size(); i++){
		tstring ComName = com_list.front();
		Pipe->PushString(ComName);
	}
}
bool CUArmObject::GetArmPosture(ePipeline* Pipe){
	
	int cmd = CMD_GET_POSTURE;

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd

	SendToArm(buf,100);

	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode = error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
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

	Pipe->PushInt(HanRotAngle);
	Pipe->PushInt(ArmRotAngle);
	Pipe->PushInt(LeftAngle);
	Pipe->PushInt(RightAngle);

	return true;
};

bool CUArmObject::SetArmPosture(ePipeline* Pipe)
{

	if (!Pipe->HasTypeAB(PARAM_TYPE4(TYPE_INT,TYPE_INT,TYPE_INT,TYPE_INT)))
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

	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
};

bool CUArmObject::Left(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		

	int cmd = CMD_ARM_POS;

	int64 RotAngle = Pipe->PopInt();
	RotAngle = m_RotServoAngle-RotAngle;
	if(RotAngle<-90 || RotAngle>90){
		Alert(1);
		//RuntimeOutput(_T("The arm can not reach the specified position"));

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

	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
};

bool CUArmObject::Right(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		
	int cmd = CMD_ARM_POS;

	int64 RotAngle = Pipe->PopInt();

	RotAngle = m_RotServoAngle+RotAngle;
	if(RotAngle<-90 || RotAngle>90){
		Alert(1);
		//RuntimeOutput(_T("The arm can not reach the specified position"));

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

	ReadMsgFromArm(); //�ȴ��ظ�
	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CUArmObject::Up(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		

	int cmd = CMD_ARM_POS;

	int64 Distance = Pipe->PopInt();

	//���׻�����
	double dy = Distance;
	dy = dy/100.0f;

	double LeftAngle,RightAngle;
	bool ret = Pos2Angle(m_x,m_y+dy,LeftAngle,RightAngle);
	if (!ret)
	{
		Alert(1);
		//RuntimeOutput(_T("The arm can not reach the specified position"));
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

	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;

}

bool CUArmObject::Down(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		
	int cmd = CMD_ARM_POS;

	int64 Distance = Pipe->PopInt();

	//���׻�����
	double dy = Distance;
	dy = dy/100.0f;

	double LeftAngle,RightAngle;
	bool ret = Pos2Angle(m_x,m_y-dy,LeftAngle,RightAngle);
	if (!ret)
	{
		Alert(1);
		//RuntimeOutput(_T("The arm can not reach the specified position"));

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

	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CUArmObject::Forward(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		
	int cmd = CMD_ARM_POS;

	int64 Distance = Pipe->PopInt();

	double LeftAngle,RightAngle;

	//���׻�����
	double dx = Distance;
	dx = dx/100.0f;


	bool ret = Pos2Angle(m_x+dx,m_y,LeftAngle,RightAngle);

	if (!ret)
	{
		Alert(1);
		//RuntimeOutput(_T("The arm can not reach the specified position"));

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

	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}

	return true;

}

bool CUArmObject::Back(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}		
	int cmd = CMD_ARM_POS;

	int64 Distance = Pipe->PopInt();

	//���׻�����
	double dx = Distance;
	dx = dx/100.0f;

	double LeftAngle,RightAngle;

	bool ret = Pos2Angle(m_x-dx,m_y,LeftAngle,RightAngle);
	if (!ret)
	{
		Alert(1);
		//RuntimeOutput(_T("The arm can not reach the specified position"));

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

	ReadMsgFromArm(); //�ȴ��ظ�
	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial port %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CUArmObject::HandRot(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x10000000"));
		return false;
	}	

	int HandRotAngle = Pipe->PopInt();
	if(HandRotAngle<-85 || HandRotAngle>85){
		Alert(1);
		//RuntimeOutput(_T("The arm can not reach the specified position"));

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
	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CUArmObject::HandGrip(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
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
	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}

	return true;

}
bool CUArmObject::HandState(ePipeline* Pipe){
	int cmd = CMD_HAND_STATE;

	int pos=0;

	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); //cmd

	SendToArm(buf,100);

	ReadMsgFromArm(); //�ȴ��ظ�
	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
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
		//RuntimeOutput(s);
	}
	return true;
}

bool CUArmObject::SetAngle(ePipeline* Pipe){

	if (!Pipe->HasTypeAB(PARAM_TYPE2(TYPE_INT,TYPE_INT)))
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

	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
};
bool CUArmObject::GetAngle(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
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

	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
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

		//RuntimeOutput(0,_T("ServoID: %d   Angle:%d"),ServoID,Angle);

		Pipe->PushInt(Angle);
	}
	return true;
}
bool CUArmObject::Alert(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE3(TYPE_INT,TYPE_INT,TYPE_INT)))
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
	ReadMsgFromArm(); //�ȴ��ظ�
	return true;
}

void CUArmObject::Alert(int n)
{
	ePipeline Pipe;
	Pipe.PushInt(n);
	Pipe.PushInt(50);
	Pipe.PushInt(50);
	Alert(&Pipe);
};

bool CUArmObject::Reset(ePipeline* Pipe){
	
	//�����ֱ���̬
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

	ReadMsgFromArm(); //�ȴ��ظ�
	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}

	//������ROT
	cmd = CMD_HAND_ROT;
	pos=0;
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	assert(pos);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 

	pos = TDTP_INT_ToString(buf,100,pos,0);

	SendToArm(buf,100);
	ReadMsgFromArm(); //�ȴ��ظ�

	return true;
}
bool CUArmObject::Calibrate(ePipeline* Pipe){

	int cmd = CMD_CALIBRATE;

	int pos=0;
	char buf[100];
	pos = TDTP_INT_ToString(buf,100,pos,m_ID);
	pos = TDTP_INT_ToString(buf,100,pos,cmd); 

	SendToArm(buf,100);

	ReadMsgFromArm(); //�ȴ��ظ�

	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
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
		Pipe->PushString(s);
	}

	return true;
}

bool CUArmObject::AttachServo(ePipeline* Pipe,bool bAttach){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
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

bool CUArmObject::SetID(ePipeline* Pipe){
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


	ReadMsgFromArm(); //�ȴ��ظ�
	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
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

bool CUArmObject::SetServoSpeed(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE2(TYPE_INT,TYPE_INT)))
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

	ReadMsgFromArm(); //�ȴ��ظ�
	
	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());

		return false;
	}
	return true;
}

bool CUArmObject::GetInfo(ePipeline* Pipe){
	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_INT)))
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

	ReadMsgFromArm(); //�ȴ��ظ�
	if (m_ErrorCode == error::eof)
	{
		//�����ж�
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	}
	else if (m_ErrorCode){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
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
		
		Pipe->PushString(s);
	}
	return true;
}
bool CUArmObject::GetAllInfo(ePipeline* Pipe){
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
		ReadMsgFromArm(); //�ȴ��ظ�

		if (m_ErrorCode == error::eof)
		{
			//�����ж�
			AnsiString error = m_ErrorCode.message();
			tstring s = Format1024(_T("serial %s  break:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
			Pipe->SetLabel(s.c_str());
			return false;
		}
		else if (m_ErrorCode){
			AnsiString error = m_ErrorCode.message();
			tstring s = Format1024(_T("serial %s read fail:%s"),m_PortName.c_str(),UTF8toWS(error).c_str());
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
	Pipe->PushString(s);
	return true;
}

tstring CUArmObject::FormatArmInfo(int InfoID,int Value){
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
bool CUArmObject::OpenPort(ePipeline* Pipe){

	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_STRING)))
	{
		Pipe->SetLabel(_T("Input data type error: require 0x30000000"));
		return false;
	}		
	
	tstring PortName = Pipe->PopString();

	AnsiString Name = WStoUTF8(PortName);
	if(!OpenSerialPort(Name.c_str())){
		AnsiString error = m_ErrorCode.message();
		tstring s = Format1024(_T("Open serial port %s fail:%s"),PortName.c_str(),UTF8toWS(error).c_str());
		Pipe->SetLabel(s.c_str());
		return false;
	};
	return true;
}
bool CUArmObject::Do(Energy* E){
	ePipeline* Pipe = (ePipeline*)E;

	if (!Pipe->HasTypeAB(PARAM_TYPE1(TYPE_STRING))) //��һ�����ݱ���TYPE_STRING
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

		Pipe->SetLabel(_T("Please open serial port first"));
		return false;
	}


	bool ret=true;
	if (ActionName == _T("get_posture"))
	{
		ret = GetArmPosture(Pipe);
	}else if (ActionName == _T("set_posture"))
	{
		ret = SetArmPosture(Pipe);
	}
	else if (ActionName == _T("left"))
	{
		ret = Left(Pipe);
	}else if (ActionName == _T("right"))
	{
		ret = Right(Pipe);
	}
	else if (ActionName == _T("up"))
	{
		ret = Up(Pipe);
	}
	else if (ActionName == _T("down"))
	{
		ret = Down(Pipe);
	}
	else if (ActionName == _T("forward"))
	{
		ret = Forward(Pipe);
	}
	else if (ActionName == _T("back"))
	{
		ret = Back(Pipe);
	}
	else if (ActionName == _T("hand_rot"))
	{	
		ret = HandRot(Pipe);

	}else if (ActionName == _T("hand_grip"))
	{
		ret = HandGrip(Pipe);
	}
	else if(ActionName == _T("hand_state"))
	{
		ret = HandState(Pipe);
	}
	else if (ActionName == _T("set_angle"))
	{
		ret = SetAngle(Pipe);
	}
	else if(ActionName == _T("get_angle"))
	{
		ret = GetAngle(Pipe);
	} 
	else if (ActionName == _T("alert"))
	{
		ret = Alert(Pipe);
	}else if (ActionName == _T("reset"))
	{
		ret = Reset(Pipe);
	}
	else if(ActionName == _T("calibrate"))
	{
		ret = Calibrate(Pipe);
	}else if (ActionName == _T("detach"))
	{
		ret = AttachServo(Pipe,false);
	}else if (ActionName == _T("attach"))
	{
		ret = AttachServo(Pipe,true);
	}
	else if(ActionName == _T("set_id"))
	{
		ret = SetID(Pipe);
	}else if (ActionName == _T("set_speed"))
	{
		ret = SetServoSpeed(Pipe);
	}
	else if(ActionName == _T("get_info"))
	{
		ret = GetInfo(Pipe);
	}
	else if (ActionName == _T("all_info"))
	{
		ret = GetAllInfo(Pipe);
	}
	else if (ActionName == _T("open_port"))
	{
		ret = OpenPort(Pipe);
	}else if (ActionName == _T("close_port"))
	{
		CloseSerialPort();		
	}else if (ActionName == _T("query_port"))
	{
		QueryPort(Pipe);
	}
	return ret;
};

#endif // _COMPILE_UARM_OBJECT