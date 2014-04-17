# Microsoft Developer Studio Generated NMAKE File, Based on TheorySpace.dsp
!IF "$(CFG)" == ""
CFG=TheorySpace - Win32 Debug
!MESSAGE No configuration specified. Defaulting to TheorySpace - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "TheorySpace - Win32 Release" && "$(CFG)" != "TheorySpace - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TheorySpace - Win32 Release"

OUTDIR=.\temp\TheorySpace\Release6
INTDIR=.\temp\TheorySpace\Release6

ALL : ".\bin\Release6\TheorySpace.lib"


CLEAN :
	-@erase "$(INTDIR)\AbstractSpace.obj"
	-@erase "$(INTDIR)\AbstractSpacePool.obj"
	-@erase "$(INTDIR)\BaseEnergy.obj"
	-@erase "$(INTDIR)\ConvertUTF.obj"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\FORMAT.OBJ"
	-@erase "$(INTDIR)\LinkerPipe.obj"
	-@erase "$(INTDIR)\Model.obj"
	-@erase "$(INTDIR)\Msg.obj"
	-@erase "$(INTDIR)\Pipeline.obj"
	-@erase "$(INTDIR)\SpaceMutex.obj"
	-@erase "$(INTDIR)\System.obj"
	-@erase "$(INTDIR)\TimeStamp.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase ".\bin\Release6\TheorySpace.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /ZI /D "NDEBUG" /D "WIN32" /D "_MBCS" /Fp"$(INTDIR)\TheorySpace.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\TheorySpace.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"bin\Release6\TheorySpace.lib" 
LIB32_OBJS= \
	"$(INTDIR)\AbstractSpace.obj" \
	"$(INTDIR)\AbstractSpacePool.obj" \
	"$(INTDIR)\BaseEnergy.obj" \
	"$(INTDIR)\ConvertUTF.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\FORMAT.OBJ" \
	"$(INTDIR)\LinkerPipe.obj" \
	"$(INTDIR)\Model.obj" \
	"$(INTDIR)\Msg.obj" \
	"$(INTDIR)\Pipeline.obj" \
	"$(INTDIR)\SpaceMutex.obj" \
	"$(INTDIR)\System.obj" \
	"$(INTDIR)\TimeStamp.obj"

".\bin\Release6\TheorySpace.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "TheorySpace - Win32 Debug"

OUTDIR=.\..\..\..\bin\vc6\debug\TheorySpace
INTDIR=.\..\..\..\obj\vc6\debug\TheorySpace
# Begin Custom Macros
OutDir=.\..\..\..\bin\vc6\debug\TheorySpace
# End Custom Macros

ALL : "$(OUTDIR)\TheorySpaceD.lib"


CLEAN :
	-@erase "$(INTDIR)\AbstractSpace.obj"
	-@erase "$(INTDIR)\AbstractSpacePool.obj"
	-@erase "$(INTDIR)\BaseEnergy.obj"
	-@erase "$(INTDIR)\ConvertUTF.obj"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\FORMAT.OBJ"
	-@erase "$(INTDIR)\LinkerPipe.obj"
	-@erase "$(INTDIR)\Model.obj"
	-@erase "$(INTDIR)\Msg.obj"
	-@erase "$(INTDIR)\Pipeline.obj"
	-@erase "$(INTDIR)\SpaceMutex.obj"
	-@erase "$(INTDIR)\System.obj"
	-@erase "$(INTDIR)\TimeStamp.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\TheorySpaceD.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_UNICODE" /D "UNICODE" /Fp"$(INTDIR)\TheorySpace.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\TheorySpace.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\TheorySpaceD.lib" 
LIB32_OBJS= \
	"$(INTDIR)\AbstractSpace.obj" \
	"$(INTDIR)\AbstractSpacePool.obj" \
	"$(INTDIR)\BaseEnergy.obj" \
	"$(INTDIR)\ConvertUTF.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\FORMAT.OBJ" \
	"$(INTDIR)\LinkerPipe.obj" \
	"$(INTDIR)\Model.obj" \
	"$(INTDIR)\Msg.obj" \
	"$(INTDIR)\Pipeline.obj" \
	"$(INTDIR)\SpaceMutex.obj" \
	"$(INTDIR)\System.obj" \
	"$(INTDIR)\TimeStamp.obj"

"$(OUTDIR)\TheorySpaceD.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("TheorySpace.dep")
!INCLUDE "TheorySpace.dep"
!ELSE 
!MESSAGE Warning: cannot find "TheorySpace.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "TheorySpace - Win32 Release" || "$(CFG)" == "TheorySpace - Win32 Debug"
SOURCE=..\..\..\src\TheorySpace\AbstractSpace.cpp

"$(INTDIR)\AbstractSpace.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\AbstractSpacePool.cpp

"$(INTDIR)\AbstractSpacePool.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\BaseEnergy.cpp

"$(INTDIR)\BaseEnergy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\ConvertUTF.cpp

"$(INTDIR)\ConvertUTF.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\crc32.cpp

"$(INTDIR)\crc32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\FORMAT.CPP

"$(INTDIR)\FORMAT.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\LinkerPipe.cpp

"$(INTDIR)\LinkerPipe.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\Model.cpp

"$(INTDIR)\Model.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\Msg.cpp

"$(INTDIR)\Msg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\Pipeline.cpp

"$(INTDIR)\Pipeline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\SpaceMutex.cpp

"$(INTDIR)\SpaceMutex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\System.cpp

"$(INTDIR)\System.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\src\TheorySpace\TimeStamp.cpp

"$(INTDIR)\TimeStamp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

