# Microsoft Developer Studio Project File - Name="TheorySpace" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TheorySpace - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TheorySpace.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TheorySpace.mak" CFG="TheorySpace - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TheorySpace - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TheorySpace - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TheorySpace - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /ZI /D "NDEBUG" /D "WIN32" /D "_UNICODE" /D "UNICODE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TheorySpace - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bin\VC6\debug"
# PROP Intermediate_Dir "obj\VC6\debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_UNICODE" /D "UNICODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"d:\project\obj\vc6\debug\TheorySpace/TheorySpace.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"bin\vc6\debug\TheorySpace_Debug.lib"

!ENDIF 

# Begin Target

# Name "TheorySpace - Win32 Release"
# Name "TheorySpace - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\AbstractSpace.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AbstractSpacePool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BaseEnergy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ConvertUTF.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FORMAT.CPP
# End Source File
# Begin Source File

SOURCE=.\src\LinkerPipe.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Model.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Msg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Pipeline.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SpaceMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\System.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TimeStamp.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\ABMutex.h
# End Source File
# Begin Source File

SOURCE=.\src\AbstractSpace.h
# End Source File
# Begin Source File

SOURCE=.\src\AbstractSpacePool.h
# End Source File
# Begin Source File

SOURCE=.\src\BaseEnergy.h
# End Source File
# Begin Source File

SOURCE=.\src\ConvertUTF.h
# End Source File
# Begin Source File

SOURCE=.\src\FORMAT.H
# End Source File
# Begin Source File

SOURCE=.\src\LinkerPipe.h
# End Source File
# Begin Source File

SOURCE=.\src\LockPipe.h
# End Source File
# Begin Source File

SOURCE=.\src\Model.h
# End Source File
# Begin Source File

SOURCE=.\src\Msg.h
# End Source File
# Begin Source File

SOURCE=.\src\Object.h
# End Source File
# Begin Source File

SOURCE=.\src\Pipeline.h
# End Source File
# Begin Source File

SOURCE=.\src\SpaceMutex.h
# End Source File
# Begin Source File

SOURCE=.\src\System.h
# End Source File
# Begin Source File

SOURCE=.\src\TheorySpace.h
# End Source File
# Begin Source File

SOURCE=.\src\TimeStamp.h
# End Source File
# Begin Source File

SOURCE=.\src\Typedef.h
# End Source File
# End Group
# End Target
# End Project
