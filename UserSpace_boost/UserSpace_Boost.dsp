# Microsoft Developer Studio Project File - Name="UserSpace_Boost" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=UserSpace_Boost - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "UserSpace_Boost.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UserSpace_Boost.mak" CFG="UserSpace_Boost - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UserSpace_Boost - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "UserSpace_Boost - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "UserSpace_Boost - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "d:\project\bin\vc6\release\UserSpace_boost"
# PROP Intermediate_Dir "d:\project\obj\vc6\release\UserSpace_boost"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_UNICODE" /D "UNICODE" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "UserSpace_Boost - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "d:\project\bin\vc6\debug\UserSpace_Boost"
# PROP Intermediate_Dir "d:\project\obj\vc6\debug\UserSpace_Boost"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_UNICODE" /D "UNICODE" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"d:\project\obj\vc6\debug\UserSpace_Boost/UserSpace_Boost.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "UserSpace_Boost - Win32 Release"
# Name "UserSpace_Boost - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserLinkerPipe.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserMutex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserObject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserSpaceMutex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserSpacePool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserTimeStamp.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserLinkerPipe.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserModel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserObject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserSpaceMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserSpacePool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserSystem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\UserSpace_Boost\UserTimeStamp.h
# End Source File
# End Group
# End Target
# End Project
