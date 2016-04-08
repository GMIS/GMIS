# Microsoft Developer Studio Project File - Name="PokerRobot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PokerRobot - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PokerRobot.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PokerRobot.mak" CFG="PokerRobot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PokerRobot - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PokerRobot - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PokerRobot - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "d:\project\bin\vc6\release\PokerRobot"
# PROP Intermediate_Dir "d:\project\obj\vc6\release\PokerRobot"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "PokerRobot - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "d:\project\bin\vc6\debug\PokerRobot"
# PROP Intermediate_Dir "d:\project\obj\vc6\debug\PokerRobot"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"d:\project\obj\vc6\debug\PokerRobot/PokerRobot.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "PokerRobot - Win32 Release"
# Name "PokerRobot - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\AddressBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\BitmapEx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\EditWin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\LogDatabase.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\MainFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\MainFrameWork.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\MyObject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\ObjectView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\OcrHost.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\OcrResultView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\OptionPipe.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\OptionView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\PokerDatabase.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\PokerRobot.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\PokerRobot.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\ResultView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\SplitterView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\StatusBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\Tesseract.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\TexasPoker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\TexasPokerEvaluate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\WebEye.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\WebHost.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\WebView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\AddressBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\BitmapEx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\EditWin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\LogDatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\MainFrame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\MyObject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\ObjectView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\OcrHost.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\OcrResultView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\OptionPipe.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\OptionView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\PokerDatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\ResultView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\SplitterView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\StatusBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\Tesseract.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\TexasPoker.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\TexasPokerEvaluate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\WebEye.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\WebHost.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\PokerRobot\WebView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\..\src\PokerRobot\icon1.ico
# End Source File
# Begin Source File

SOURCE=..\..\..\src\PokerRobot\poker.bmp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\WebRobot\poker.bmp
# End Source File
# End Group
# End Target
# End Project
