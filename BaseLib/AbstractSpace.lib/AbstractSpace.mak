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

!IF  "$(CFG)" == "TheorySpace - Win32 Release"

OUTDIR=.\bin\VC6\release
INTDIR=.\obj\VC6\release
# Begin Custom Macros
OutDir=.\bin\VC6\release
# End Custom Macros

ALL : "$(OUTDIR)\TheorySpace.lib"


CLEAN :
	-@erase "$(INTDIR)\AbstractSpace.obj"
	-@erase "$(INTDIR)\AbstractSpacePool.obj"
	-@erase "$(INTDIR)\BaseEnergy.obj"
	-@erase "$(INTDIR)\ConvertUTF.obj"
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
	-@erase "$(OUTDIR)\TheorySpace.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /ZI /D "NDEBUG" /D "WIN32" /D "_UNICODE" /D "UNICODE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\TheorySpace.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\TheorySpace.lib" 
LIB32_OBJS= \
	"$(INTDIR)\AbstractSpace.obj" \
	"$(INTDIR)\AbstractSpacePool.obj" \
	"$(INTDIR)\BaseEnergy.obj" \
	"$(INTDIR)\ConvertUTF.obj" \
	"$(INTDIR)\FORMAT.OBJ" \
	"$(INTDIR)\LinkerPipe.obj" \
	"$(INTDIR)\Model.obj" \
	"$(INTDIR)\Msg.obj" \
	"$(INTDIR)\Pipeline.obj" \
	"$(INTDIR)\SpaceMutex.obj" \
	"$(INTDIR)\System.obj" \
	"$(INTDIR)\TimeStamp.obj"

"$(OUTDIR)\TheorySpace.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "TheorySpace - Win32 Debug"

OUTDIR=.\bin\VC6\debug
INTDIR=.\obj\VC6\debug
# Begin Custom Macros
OutDir=.\bin\VC6\debug
# End Custom Macros

ALL : "$(OUTDIR)\TheorySpace_Debug.lib"


CLEAN :
	-@erase "$(INTDIR)\AbstractSpace.obj"
	-@erase "$(INTDIR)\AbstractSpacePool.obj"
	-@erase "$(INTDIR)\BaseEnergy.obj"
	-@erase "$(INTDIR)\ConvertUTF.obj"
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
	-@erase "$(OUTDIR)\TheorySpace_Debug.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_UNICODE" /D "UNICODE" /Fp"$(INTDIR)\TheorySpace.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"d:\project\obj\vc6\debug\TheorySpace/TheorySpace.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\TheorySpace_Debug.lib" 
LIB32_OBJS= \
	"$(INTDIR)\AbstractSpace.obj" \
	"$(INTDIR)\AbstractSpacePool.obj" \
	"$(INTDIR)\BaseEnergy.obj" \
	"$(INTDIR)\ConvertUTF.obj" \
	"$(INTDIR)\FORMAT.OBJ" \
	"$(INTDIR)\LinkerPipe.obj" \
	"$(INTDIR)\Model.obj" \
	"$(INTDIR)\Msg.obj" \
	"$(INTDIR)\Pipeline.obj" \
	"$(INTDIR)\SpaceMutex.obj" \
	"$(INTDIR)\System.obj" \
	"$(INTDIR)\TimeStamp.obj"

"$(OUTDIR)\TheorySpace_Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("TheorySpace.dep")
!INCLUDE "TheorySpace.dep"
!ELSE 
!MESSAGE Warning: cannot find "TheorySpace.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "TheorySpace - Win32 Release" || "$(CFG)" == "TheorySpace - Win32 Debug"
SOURCE=.\src\AbstractSpace.cpp

"$(INTDIR)\AbstractSpace.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\AbstractSpacePool.cpp

"$(INTDIR)\AbstractSpacePool.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\BaseEnergy.cpp

"$(INTDIR)\BaseEnergy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\ConvertUTF.cpp

"$(INTDIR)\ConvertUTF.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\FORMAT.CPP

"$(INTDIR)\FORMAT.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\LinkerPipe.cpp

"$(INTDIR)\LinkerPipe.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\Model.cpp

"$(INTDIR)\Model.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\Msg.cpp

"$(INTDIR)\Msg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\Pipeline.cpp

"$(INTDIR)\Pipeline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\SpaceMutex.cpp

"$(INTDIR)\SpaceMutex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\System.cpp

"$(INTDIR)\System.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\TimeStamp.cpp

"$(INTDIR)\TimeStamp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

