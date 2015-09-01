// Win32Tool.cpp: implementation of the Win32Tool class.
//
//////////////////////////////////////////////////////////////////////

#include "Win32Tool.h"

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
{
  THREADNAME_INFO info;
  {
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
  }
  __try
  {
    RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
  }
  __except (EXCEPTION_CONTINUE_EXECUTION)
  {
  }
}
int CDECL MBPrintf (TCHAR * szCaption, TCHAR * szFormat, ...)
{
	TCHAR   szBuffer [1024] ;
	va_list pArgList ;

	va_start (pArgList, szFormat) ;
	_vsntprintf (	szBuffer, sizeof (szBuffer) / sizeof (TCHAR), 
			szFormat, pArgList) ;
	va_end (pArgList) ;
	return MessageBox (NULL, szBuffer, szCaption, 0) ;
} 

BOOL CenterWindow(HWND hwndChild, HWND hwndParent) { 
    RECT    rcChild, rcParent;
    int     cxChild, cyChild, cxParent, cyParent;
    int     cxScreen, cyScreen, xNew, yNew;
    HDC     hdc;

    // Get the Height and Width of the child window
    GetWindowRect(hwndChild, &rcChild);
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;


    if (hwndParent)
    {
    // Get the Height and Width of the parent window
       GetWindowRect(hwndParent, &rcParent);
       cxParent = rcParent.right - rcParent.left;
       cyParent = rcParent.bottom - rcParent.top;
    }

    else
    {
       cxParent = GetSystemMetrics (SM_CXSCREEN);
       cyParent = GetSystemMetrics (SM_CYSCREEN);
       rcParent.left = 0;
       rcParent.top  = 0;
       rcParent.right = cxParent;
       rcParent.bottom= cyParent;
    }

    // Get the display limits
    hdc = GetDC(hwndChild);
    cxScreen = GetDeviceCaps(hdc, HORZRES);
    cyScreen = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(hwndChild, hdc);

    // Calculate new X position, then adjust for screen
    xNew = rcParent.left + ((cxParent - cxChild) / 2);
    if (xNew < 0)
    {
        xNew = 0;
    }
    else if ((xNew + cxChild) > cxScreen)
    {
        xNew = cxScreen - cxChild;
    }

    // Calculate new Y position, then adjust for screen
    yNew = rcParent.top  + ((cyParent - cyChild) / 2);
    if (yNew < 0)
    {
        yNew = 0;
    }
    else if ((yNew + cyChild) > cyScreen)
    {
        yNew = cyScreen - cyChild;
    }

    // Set it, and return
    return SetWindowPos(hwndChild,
                        NULL,
                        xNew, yNew,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER);
};



//Function to convert unsigned char to string of length 2
void Char2Hex(unsigned char ch, char* szHex)
{
	unsigned char byte[2];
	byte[0] = ch/16;
	byte[1] = ch%16;
	for(int i=0; i<2; i++)
	{
		if(byte[i] >= 0 && byte[i] <= 9)
			szHex[i] = '0' + byte[i];
		else
			szHex[i] = 'A' + byte[i] - 10;
	}
	szHex[2] = 0;
}

//Function to convert string of length 2 to unsigned char
void Hex2Char(char const* szHex, unsigned char& rch)
{
	rch = 0;
	for(int i=0; i<2; i++)
	{
		if(*(szHex + i) >='0' && *(szHex + i) <= '9')
			rch = (rch << 4) + (*(szHex + i) - '0');
		else if(*(szHex + i) >='A' && *(szHex + i) <= 'F')
			rch = (rch << 4) + (*(szHex + i) - 'A' + 10);
		else
			break;
	}
}    

//Function to convert string of unsigned chars to string of chars
void CharStr2HexStr(unsigned char const* pucCharStr, char* pszHexStr, int iSize)
{
	int i;
	char szHex[3];
	pszHexStr[0] = 0;
	for(i=0; i<iSize; i++)
	{
		Char2Hex(pucCharStr[i], szHex);
		strcat(pszHexStr, szHex);
	}
}

//Function to convert string of chars to string of unsigned chars
void HexStr2CharStr(char const* pszHexStr, unsigned char* pucCharStr, int iSize)
{
	int i;
	unsigned char ch;
	for(i=0; i<iSize; i++)
	{
		Hex2Char(pszHexStr+2*i, ch);
		pucCharStr[i] = ch;
	}
}

BOOL DeleteDir( CONST TCHAR* lpcszSrcPath )
{
		SHFILEOPSTRUCT    sfo;
		
		memset(&sfo, '/0', sizeof(sfo));
		sfo.hwnd = NULL;
		sfo.wFunc = FO_DELETE;
		sfo.pFrom = lpcszSrcPath;
		sfo.pTo   = NULL;
		sfo.fFlags = FOF_NOERRORUI | FOF_NOCONFIRMATION;

		int ret = SHFileOperation(&sfo); 
		if (!ret)
		{
			return FALSE;
		}

		return TRUE;
}