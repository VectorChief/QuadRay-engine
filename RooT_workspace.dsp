# Microsoft Developer Studio Project File - Name="RooT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=RooT - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RooT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RooT.mak" CFG="RooT - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RooT - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "RooT - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RooT - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "core/config/" /I "core/engine/" /I "core/system/" /I "core/tracer/" /I "data/materials/" /I "data/objects/" /I "data/scenes/" /I "data/textures/" /D "RT_WIN32" /D "RT_X86" /D RT_DEBUG=0 /D RT_FULLSCREEN=0 /D RT_EMBED_STDOUT=1 /D RT_EMBED_FILEIO=0 /D RT_EMBED_TEX=1 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "NDEBUG" /FR /YX /FD /Zm500 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /machine:I386 /out:"RooT.exe"

!ELSEIF  "$(CFG)" == "RooT - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "core/config/" /I "core/engine/" /I "core/system/" /I "core/tracer/" /I "data/materials/" /I "data/objects/" /I "data/scenes/" /I "data/textures/" /D "RT_WIN32" /D "RT_X86" /D RT_DEBUG=1 /D RT_FULLSCREEN=0 /D RT_EMBED_STDOUT=1 /D RT_EMBED_FILEIO=0 /D RT_EMBED_TEX=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /Zm500 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"RooT.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "RooT - Win32 Release"
# Name "RooT - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\RooT_win32.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\RooT.h
# End Source File
# End Group
# Begin Group "core"

# PROP Default_Filter ""
# Begin Group "config"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\core\config\rtarch.h
# End Source File
# Begin Source File

SOURCE=.\core\config\rtarch_arm.h
# End Source File
# Begin Source File

SOURCE=.\core\config\rtarch_arm_mpe.h
# End Source File
# Begin Source File

SOURCE=.\core\config\rtarch_x86.h
# End Source File
# Begin Source File

SOURCE=.\core\config\rtarch_x86_sse.h
# End Source File
# Begin Source File

SOURCE=.\core\config\rtbase.h
# End Source File
# Begin Source File

SOURCE=.\core\config\rtconf.h
# End Source File
# End Group
# Begin Group "engine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\core\engine\engine.cpp
# End Source File
# Begin Source File

SOURCE=.\core\engine\engine.h
# End Source File
# Begin Source File

SOURCE=.\core\engine\format.h
# End Source File
# Begin Source File

SOURCE=.\core\engine\object.cpp
# End Source File
# Begin Source File

SOURCE=.\core\engine\object.h
# End Source File
# Begin Source File

SOURCE=.\core\engine\rtgeom.cpp
# End Source File
# Begin Source File

SOURCE=.\core\engine\rtgeom.h
# End Source File
# Begin Source File

SOURCE=.\core\engine\rtimag.cpp
# End Source File
# Begin Source File

SOURCE=.\core\engine\rtimag.h
# End Source File
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\core\system\system.cpp
# End Source File
# Begin Source File

SOURCE=.\core\system\system.h
# End Source File
# End Group
# Begin Group "tracer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\core\tracer\tracer.cpp
# End Source File
# Begin Source File

SOURCE=.\core\tracer\tracer.h
# End Source File
# End Group
# End Group
# Begin Group "data"

# PROP Default_Filter ""
# Begin Group "materials"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\data\materials\all_mat.h
# End Source File
# End Group
# Begin Group "objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\data\objects\all_obj.h
# End Source File
# End Group
# Begin Group "scenes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\data\scenes\all_scn.h
# End Source File
# Begin Source File

SOURCE=.\data\scenes\scn_demo01.h
# End Source File
# End Group
# Begin Group "textures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\data\textures\all_tex.h
# End Source File
# Begin Source File

SOURCE=.\data\textures\tex_crate01.h
# End Source File
# End Group
# End Group
# End Target
# End Project
