// curlobject.cpp: implementation of the WinAPIObject class.
//
//////////////////////////////////////////////////////////////////////

#include "curlobject.h"


#if defined _COMPILE_CURL_OBJECT

#include <assert.h>
#include <shellapi.h>
#include <algorithm>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Wldap32.lib")
#pragma comment(lib,"libcurld.lib")



size_t write_data(void *ptr, size_t size, size_t nmemb, void *User)  
{
	CCurlObject* ob = (CCurlObject*)User;
	size_t bytes = size * nmemb;
	ob->m_buf = (char*)realloc(ob->m_buf,ob->m_bufsize+bytes+1);
	if (ob->m_buf==NULL)
	{
		ob->m_bufsize=0;
		return 0;
	}
	
	memcpy(ob->m_buf+ob->m_bufsize, ptr, bytes);	
	ob->m_bufsize += bytes;
	ob->m_buf[ob->m_bufsize] = 0;
	return size * nmemb;
}

//////////////////////////////////////////////////////////////////////////



const TCHAR* CCurlObject::UserManual = __TEXT(\
		"\
		1 cmd：open\n\
		Function：打开一个程序或链接\n\
		Input：字符串，程序名或链接地址\n\
		Output：null\n\
		"\
		);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCurlObject::CCurlObject(tstring Name,int64 ID)
	:m_Name(Name),Mass(ID),m_buf(NULL),m_bufsize(0){
	curl_global_init(CURL_GLOBAL_ALL);
	m_Curl = curl_easy_init();
	m_Headers = NULL;
 	m_Headers = curl_slist_append(m_Headers, "application/json");
};

CCurlObject::~CCurlObject(){
	if(m_Headers){
		curl_slist_free_all(m_Headers);
	};
	if(m_Curl){
		curl_easy_cleanup(m_Curl);
	};
};

bool CCurlObject::CmdLine(ePipeline* Pipe){
	eElectron e;
	Pipe->Pop(&e);

	if (e.EnergyType() != TYPE_STRING)
	{
		Pipe->SetLabel(_T("Error: Illegal param"));
	}
	tstring s = *(tstring*)e.Value();
	AnsiString CmdLine = WStoUTF8(s);

	SECURITY_ATTRIBUTES sa; 
	HANDLE hOutputRead,hOutputWrite; 

	sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
	sa.lpSecurityDescriptor = NULL; 
	sa.bInheritHandle = TRUE; 
	if (!CreatePipe(&hOutputRead,&hOutputWrite,&sa,0)) 
	{ 
		Pipe->SetLabel(_T("Error: can't create pipe"));
		return false; 
	} 

	STARTUPINFOA si;

	PROCESS_INFORMATION  pi;	
	::ZeroMemory(&si, sizeof(si));	
	si.cb = sizeof(si);
//	si.hStdError = hOutputWrite; 
	si.hStdOutput = hOutputWrite; 
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	
	AnsiString ExecuterFile = "curl.exe " + CmdLine;

	char CmdLineBuf[512];
	strcpy(CmdLineBuf,ExecuterFile.c_str());

	BOOL started = ::CreateProcessA(
		NULL,//ExecuterFile.c_str(),        // command is part of input string
		CmdLineBuf,  // (writeable) command string
		NULL,        // process security
		NULL,        // thread security
		TRUE,       // inherit handles flag
		0,           // flags
		NULL,        // inherit environment
		NULL,        // inherit directory
		&si,    // STARTUPINFO
		&pi);   // PROCESS_INFORMATION

	if(!started){
		Pipe->SetLabel(_T("Error: can't open curl.exe "));
		return false;
	}

	//WaitForSingleObject(pi.hProcess,INFINITE);

	CloseHandle(hOutputWrite);

	char buffer[4096] = {0}; 
	DWORD bytesRead; 
	AnsiString result;
	while (true)
	{
		if(ReadFile(hOutputRead,buffer,4095,&bytesRead,NULL) == NULL) 
		{
			break;
		}
		buffer[bytesRead]=0;
		result += buffer;
		Sleep(250);
	}
	CloseHandle(hOutputRead);

	Pipe->PushString(result);

	return true;
}
bool CCurlObject::Header(ePipeline* Pipe){
	eElectron e;
	Pipe->Pop(&e);

	if (e.EnergyType() != TYPE_STRING)
	{
		Pipe->SetLabel(_T("Error: Illegal param"));
	}
	tstring s = *(tstring*)e.Value();

	AnsiString param = WStoUTF8(s);
	curl_slist_append(m_Headers,param.c_str());
	return true;
}
bool CCurlObject::Set(ePipeline* Pipe){
	eElectron e;
	Pipe->Pop(&e);

	if (e.EnergyType() != TYPE_STRING)
	{
		Pipe->SetLabel(_T("Error: Illegal param"));
	}
	tstring op = *(tstring*)e.Value();
	if (op == _T("CURLOPT_URL"))
	{
		
		Pipe->Pop(&e);
		if (e.EnergyType() != TYPE_STRING)
		{
			Pipe->SetLabel(_T("Error: Illegal param"));
		}
		tstring s = *(tstring*)e.Value();
		AnsiString url = WStoUTF8(s);
		curl_easy_setopt(m_Curl, CURLOPT_URL,url.c_str());
	}else if (op == _T("CURLOPT_HTTPHEADER"))
	{
		curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER,m_Headers);
	}
	else if (op == _T("CURLOPT_USERNAME"))
	{
		Pipe->Pop(&e);
		if (e.EnergyType() != TYPE_STRING)
		{
			Pipe->SetLabel(_T("Error: Illegal param"));
		}
		tstring s = *(tstring*)e.Value();
		AnsiString name = WStoUTF8(s);
		curl_easy_setopt(m_Curl, CURLOPT_USERNAME,name.c_str());
	}
	else if (op == _T("CURLOPT_PASSWORD"))
	{
		Pipe->Pop(&e);
		if (e.EnergyType() != TYPE_STRING)
		{
			Pipe->SetLabel(_T("Error: Illegal param"));
		}
		tstring s = *(tstring*)e.Value();
		AnsiString password = WStoUTF8(s);
		curl_easy_setopt(m_Curl, CURLOPT_PASSWORD,password.c_str());
	}
	else if (op == _T("CURLOPT_VERBOSE"))
	{
		Pipe->Pop(&e);
		if (e.EnergyType() != TYPE_STRING)
		{
			Pipe->SetLabel(_T("Error: Illegal param"));
		}
		tstring s = *(tstring*)e.Value();
		if (s == _T("1"))
		{
			curl_easy_setopt(m_Curl, CURLOPT_VERBOSE,1L);
		} 
		else
		{
			curl_easy_setopt(m_Curl, CURLOPT_VERBOSE,0L);
		}	
	}
	else if (op == _T("CURLOPT_POSTFIELDS"))
	{
		Pipe->Pop(&e);
		if (e.EnergyType() != TYPE_STRING)
		{
			Pipe->SetLabel(_T("Error: Illegal param"));
		}
		tstring s = *(tstring*)e.Value();
		AnsiString postfield = WStoUTF8(s);
		curl_easy_setopt(m_Curl, CURLOPT_POSTFIELDS,postfield.c_str());
		 
	}
	else if (op == _T("CURLOPT_HTTPGET"))
	{
		Pipe->Pop(&e);
		if (e.EnergyType() != TYPE_STRING)
		{
			Pipe->SetLabel(_T("Error: Illegal param"));
		}
		tstring s = *(tstring*)e.Value();
		if (s == _T("1"))
		{
			curl_easy_setopt(m_Curl,CURLOPT_HTTPGET,1L);
		} 
	}
	else if (op == _T("CURLOPT_POST"))
	{
		Pipe->Pop(&e);
		if (e.EnergyType() != TYPE_STRING)
		{
			Pipe->SetLabel(_T("Error: Illegal param"));
		}
		tstring s = *(tstring*)e.Value();
		if (s == _T("1"))
		{
			curl_easy_setopt(m_Curl,CURLOPT_POST,1L);
		} 
	}else if (op == _T("CURLOPT_USERAGENT"))
	{
		Pipe->Pop(&e);
		if (e.EnergyType() != TYPE_STRING)
		{
			Pipe->SetLabel(_T("Error: Illegal param"));
		}
		tstring s = *(tstring*)e.Value();
		AnsiString useragent = WStoUTF8(s); //"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.7.8) Gecko/20050511 Firefox/1.0.4"
		curl_easy_setopt(m_Curl, CURLOPT_USERAGENT,useragent.c_str());

	}
	else 
	{
		tstring info = Format1024(_T("Do not support %s"),op.c_str());
		Pipe->SetLabel(info.c_str());
		return false;
	}

	return true;
}

bool CCurlObject::Reset(ePipeline* Pipe){
	if(m_Curl){
		curl_easy_cleanup(m_Curl);
		curl_slist_free_all(m_Headers);
		m_Headers = NULL; 
	};
	m_Curl = curl_easy_init();
	m_Headers = curl_slist_append(m_Headers, "Accept:application/json");
	return true;
};
bool CCurlObject::Run(ePipeline* Pipe){
	curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, m_Headers);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, write_data);  
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA,this);
	CURLcode code = curl_easy_perform(m_Curl);   
	if (code == CURLE_OK) {
		AnsiString result;
		if(m_bufsize){
			result = m_buf;
			free(m_buf);
			m_bufsize = 0;
		}
		Pipe->PushString(result);
	}
	return true;	
};
bool CCurlObject::Do(Energy* E)
{
	ePipeline* Pipe = (ePipeline*)E;

	Pipe->SetLabel(_T(""));

	if (Pipe->Size()==0)
	{
		return true;
	}

    ENERGY_TYPE Type = Pipe->GetDataType(0);
	if (Type != TYPE_STRING)
	{
		return false;
	}

	tstring Cmd = Pipe->PopString();
    transform(Cmd.begin(),Cmd.end(),Cmd.begin(), ::tolower);

    if (Cmd == _T("header"))
    {
		return Header(Pipe);

    }else if(Cmd == _T("option")){
		
		return Set(Pipe);

	}else if (Cmd == _T("run"))
	{
		return Run(Pipe);
	}else if (Cmd == _T("reset"))
	{
		return Reset(Pipe);
	}
	else if (Cmd == _T("command"))
    {
		return CmdLine(Pipe);
	};

	Pipe->SetLabel(_T("Illegal command"));
//	Pipe->SetID(RETURN_ERROR);
	return false;
}

#endif //_COMPILE_WIMAPI_OBJECT