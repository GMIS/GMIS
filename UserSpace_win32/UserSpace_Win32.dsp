# Microsoft Developer Studio Project File - Name="UserSpace_Win32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=UserSpace_Win32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "UserSpace_Win32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UserSpace_Win32.mak" CFG="UserSpace_Win32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UserSpace_Win32 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "UserSpace_Win32 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "UserSpace_Win32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bin\VC6\release"
# PROP Intermediate_Dir "obj\VC6\release"
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

!ELSEIF  "$(CFG)" == "UserSpace_Win32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bin\VC6\release"
# PROP Intermediate_Dir "obj\VC6\release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_UNICODE" /D "UNICODE" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"d:\project\obj\vc6\debug\UserSpace_Win32/UserSpace_Win32.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"bin\VC6\debug\UserSpace_Win32_Debug.lib"

!ENDIF 

# Begin Target

# Name "UserSpace_Win32 - Win32 Release"
# Name "UserSpace_Win32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\UserLinkerPipe.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UserModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UserMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UserObject.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UserSpaceMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UserSpacePool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UserSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UserTimeStamp.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\UserLinkerPipe.h
# End Source File
# Begin Source File

SOURCE=.\src\UserModel.h
# End Source File
# Begin Source File

SOURCE=.\src\UserMutex.h
# End Source File
# Begin Source File

SOURCE=.\src\UserObject.h
# End Source File
# Begin Source File

SOURCE=.\src\UserSpaceMutex.h
# End Source File
# Begin Source File

SOURCE=.\src\UserSpacePool.h
# End Source File
# Begin Source File

SOURCE=.\src\UserSystem.h
# End Source File
# Begin Source File

SOURCE=.\src\UserTimeStamp.h
# End Source File
# End Group
# End Target
# End Project
