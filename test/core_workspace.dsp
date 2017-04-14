# Microsoft Developer Studio Project File - Name="core_test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=core_test - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "core_test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "core_test.mak" CFG="core_test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "core_test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "core_test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "core_test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../core/config/" /I "../core/engine/" /I "../core/system/" /I "../core/tracer/" /I "../data/materials/" /I "../data/objects/" /I "../data/textures/" /I "scenes/" /D "RT_WIN32" /D "RT_X86" /D RT_128=1+2+4+8 /D RT_256=1+2 /D RT_512=1+2 /D RT_POINTER=32 /D RT_ADDRESS=32 /D RT_ELEMENT=32 /D RT_ENDIAN=0 /D RT_DEBUG=0 /D RT_PATH="../" /D RT_EMBED_STDOUT=0 /D RT_EMBED_FILEIO=0 /D RT_EMBED_TEX=1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /Zm500 /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib /nologo /subsystem:console /machine:I386 /out:"core_test.exe"

!ELSEIF  "$(CFG)" == "core_test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "../core/config/" /I "../core/engine/" /I "../core/system/" /I "../core/tracer/" /I "../data/materials/" /I "../data/objects/" /I "../data/textures/" /I "scenes/" /D "RT_WIN32" /D "RT_X86" /D RT_128=1+2+4+8 /D RT_256=1+2 /D RT_512=1+2 /D RT_POINTER=32 /D RT_ADDRESS=32 /D RT_ELEMENT=32 /D RT_ENDIAN=0 /D RT_DEBUG=1 /D RT_PATH="../" /D RT_EMBED_STDOUT=0 /D RT_EMBED_FILEIO=0 /D RT_EMBED_TEX=1 /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /Zm500 /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"core_test.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "core_test - Win32 Release"
# Name "core_test - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\core_test.cpp
# End Source File
# End Group
# Begin Group "core"

# PROP Default_Filter ""
# Begin Group "config"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\config\rtarch.h
# End Source File
# Begin Source File

SOURCE=..\core\config\rtarch_x86.h
# End Source File
# Begin Source File

SOURCE=..\core\config\rtarch_x86_128x1v4.h
# End Source File
# Begin Source File

SOURCE=..\core\config\rtarch_x86_128x1v8.h
# End Source File
# Begin Source File

SOURCE=..\core\config\rtarch_x86_256x1v2.h
# End Source File
# Begin Source File

SOURCE=..\core\config\rtarch_x86_512x1v2.h
# End Source File
# Begin Source File

SOURCE=..\core\config\rtbase.h
# End Source File
# Begin Source File

SOURCE=..\core\config\rtconf.h
# End Source File
# Begin Source File

SOURCE=..\core\config\rtzero.h
# End Source File
# End Group
# Begin Group "engine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\engine\engine.cpp
# End Source File
# Begin Source File

SOURCE=..\core\engine\engine.h
# End Source File
# Begin Source File

SOURCE=..\core\engine\format.h
# End Source File
# Begin Source File

SOURCE=..\core\engine\object.cpp
# End Source File
# Begin Source File

SOURCE=..\core\engine\object.h
# End Source File
# Begin Source File

SOURCE=..\core\engine\rtgeom.cpp
# End Source File
# Begin Source File

SOURCE=..\core\engine\rtgeom.h
# End Source File
# Begin Source File

SOURCE=..\core\engine\rtimag.cpp
# End Source File
# Begin Source File

SOURCE=..\core\engine\rtimag.h
# End Source File
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\system\system.cpp
# End Source File
# Begin Source File

SOURCE=..\core\system\system.h
# End Source File
# End Group
# Begin Group "tracer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\tracer\tracer.cpp
# End Source File
# Begin Source File

SOURCE=..\core\tracer\tracer_128v1.cpp
# End Source File
# Begin Source File

SOURCE=..\core\tracer\tracer_128v2.cpp
# End Source File
# Begin Source File

SOURCE=..\core\tracer\tracer_128v4.cpp
# End Source File
# Begin Source File

SOURCE=..\core\tracer\tracer_128v8.cpp
# End Source File
# Begin Source File

SOURCE=..\core\tracer\tracer_256v1.cpp
# End Source File
# Begin Source File

SOURCE=..\core\tracer\tracer_256v2.cpp
# End Source File
# Begin Source File

SOURCE=..\core\tracer\tracer_512v1.cpp
# End Source File
# Begin Source File

SOURCE=..\core\tracer\tracer_512v2.cpp
# End Source File
# Begin Source File

SOURCE=..\core\tracer\tracer.h
# End Source File
# End Group
# End Group
# Begin Group "data"

# PROP Default_Filter ""
# Begin Group "materials"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\data\materials\all_mat.h
# End Source File
# End Group
# Begin Group "objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\data\objects\all_obj.h
# End Source File
# Begin Source File

SOURCE=..\data\objects\obj_aliencube.h
# End Source File
# Begin Source File

SOURCE=..\data\objects\obj_frametable.h
# End Source File
# End Group
# Begin Group "textures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\data\textures\all_tex.h
# End Source File
# Begin Source File

SOURCE=..\data\textures\tex_crate01.h
# End Source File
# End Group
# End Group
# Begin Group "test"

# PROP Default_Filter ""
# Begin Group "scenes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\scenes\scn_test01.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test02.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test03.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test04.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test05.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test06.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test07.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test08.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test09.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test10.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test11.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test12.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test13.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test14.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test15.h
# End Source File
# Begin Source File

SOURCE=.\scenes\scn_test16.h
# End Source File
# End Group
# End Group
# End Target
# End Project
