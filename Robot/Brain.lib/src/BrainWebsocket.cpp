#include "BrainWebsocket.h"
#include "Brain.h"

void WebSocketRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

	try
	{
	
	
		WebSocket ws(request, response);
		CLinker Linker;
		int64 SourceID = AbstractSpace::CreateTimeStamp();
		m_Brain->m_WebsocketClientList.CreateLinker(Linker,m_Brain,SourceID,ws);
		if(!Linker.IsValid()){
			   
				AnsiString Add = request.clientAddress().toString();
				tstring  AddW = UTF8toWS(Add);

				tstring s = Format1024(_T("websocket accept %s fail (can't create new WebsocketLinker)"),AddW.c_str());

				ePipeline Info;
				Info.PushString(s);
				m_Brain->NotifyLinkerState(SourceID,LINKER_CREATE_ERROR,NORMAL_LEVEL,Info);
				ws.close();
				return;
		}


		CLinkerPipe* LinkerPtr = Linker.Release();
		LinkerPtr->SetStateOutputLevel(WEIGHT_LEVEL);
		m_Brain->m_WebsocketClientList.AddLinker(LinkerPtr);

		CMsg Msg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_WHO_ARE_YOU,DEFAULT_DIALOG,0);
		LinkerPtr->PushMsgToSend(Msg);

		m_Brain->CreateWebsokectWorkerStrategy();
	}
	catch (WebSocketException& exc)
	{
		AnsiString error = exc.what();
		tstring wInfo = Format1024(_T("webcoket exception:%s"),UTF8toWS(error).c_str());
		ePipeline Info;
		Info.PushInt(-1);
		Info.PushString(wInfo);
		m_Brain->NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_LISTEN_FAIL,&Info);

		switch (exc.code())
		{
		case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
			response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
			// fallthrough
		case WebSocket::WS_ERR_NO_HANDSHAKE:
		case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
		case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
			response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
			response.setContentLength(0);
			response.send();
			break;
		}
	}
}


HTTPRequestHandler* BrainRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	AnsiString info = "Request from " 
		+ request.clientAddress().toString()
		+ ": "
		+ request.getMethod()
		+ " "
		+ request.getURI()
		+ " "
		+ request.getVersion();
	tstring wInfo = UTF8toWS(info);
	//m_Brain->OutSysInfo(wInfo);

	for (HTTPServerRequest::ConstIterator it = request.begin(); it != request.end(); ++it)
	{
		info = it->first + ": " + it->second;
		wInfo = UTF8toWS(info);
		//m_Brain->OutSysInfo(wInfo);
	}
	if(request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
		return new WebSocketRequestHandler(m_Brain);
	else
		return NULL;
}


CWebsocketLinkerPipe::CWebsocketLinkerPipe(CSpaceMutex* Mutex,System* Parent,int64 SourceID,const Socket& socket)
	:CLinkerPipe(Mutex,Parent,WEBSOCKET_LINKER,SourceID,_T("Child")),m_Socket(socket)
{
	assert(Mutex);
	m_bDeleteMutex = FALSE;
#ifdef _DEBUG
	SetStateOutputLevel(LIGHT_LEVEL);
#endif
}

CWebsocketLinkerPipe::~CWebsocketLinkerPipe(){

	if (m_Mutex && m_bDeleteMutex)
	{
		delete m_Mutex;
		m_Mutex = NULL;
	}
}



bool   CWebsocketLinkerPipe::IsValid(){
	bool ret = false;
#if defined(USING_POCO)
	SocketImpl* imp = m_Socket.impl();
	ret = imp->initialized();
#else
   #error "Current impletement only support poco c++ lib"
#endif
	return ret;
}

void   CWebsocketLinkerPipe::Close(){

#if defined(USING_POCO)
	m_Socket.shutdown();
	m_Socket.close();
#else
	#error "Current impletement only support poco c++ lib"
#endif
	CLinkerPipe::Close();
}

bool  CWebsocketLinkerPipe::PhysicalRev(char* Buf,uint32 BufSize, uint32& RevLen, uint32 flag){

	RevLen = 0;	

#if defined(USING_POCO)
	Poco::Timespan timeout(50);
	bool ret = m_Socket.poll(timeout,Socket::SELECT_READ);
	if (ret)
	{
		int32 flag=0;
		RevLen = m_Socket.receiveFrame(Buf,BufSize,flag);
		if((flag & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_CLOSE){
			Poco::Net::NetException e("websocket closed by remote");
			throw e;
		}
		Buf[RevLen] = '\0';
			
	}
#else
#error "Current implemention only support poco c++ lib";
#endif
	return TRUE;
};

bool  CWebsocketLinkerPipe::PhysicalSend(char* Buf,uint32 BufSize, uint32& SendLen, uint32 flag){
	SendLen = 0;
#if defined(USING_POCO)
	Poco::Timespan timeout(50);

	bool ret = m_Socket.poll(timeout,Socket::SELECT_WRITE);
	if(ret){
		SendLen = m_Socket.sendFrame(Buf,BufSize,WebSocket::FRAME_BINARY);
	}

#else
	#error "Current implemention only support poco c++ lib";
#endif

	return TRUE;

};