#ifndef _BRAIN_WEBSOCKET_H
#define _BRAIN_WEBSOCKET_H

#include "LinkerPipe.h"
#include "SpaceMutex.h"
#include "System.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Format.h"
#include <iostream>


using namespace PHYSIC;

using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::ThreadPool;
using Poco::Net::Socket;

class CBrain;

class WebSocketRequestHandler: public HTTPRequestHandler
	/// Handle a WebSocket connection.
{
	CBrain*  m_Brain;
public:
	WebSocketRequestHandler(CBrain* Brain):m_Brain(Brain){

	};

	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
};


class BrainRequestHandlerFactory: public HTTPRequestHandlerFactory
{
	CBrain*  m_Brain;
public:
	BrainRequestHandlerFactory(CBrain* Brain): m_Brain(Brain){

	};

	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request);
};


class CWebsocketLinkerPipe:public CLinkerPipe
{
	SUPPORT_ABSTRACT_SAPCE_POOL(CWebsocketLinkerPipe);

private:
	bool          m_bDeleteMutex;
public:

#if defined(USING_POCO)
	WebSocket     m_Socket;
#endif

public:
	CWebsocketLinkerPipe(CSpaceMutex* Mutex,System* Parent,int64 SourceID,const Socket& socket);
	virtual ~CWebsocketLinkerPipe();

	virtual bool   IsValid();
	virtual void   Close();

private:
	bool  PhysicalRev(char* Buf,uint32 BufSize, uint32& RevLen, uint32 flag=0);
	bool  PhysicalSend(char* Buf,uint32 BufSize, uint32& SendLen, uint32 flag=0);
};
#endif