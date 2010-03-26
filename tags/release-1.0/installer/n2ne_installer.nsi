!include "MUI2.nsh"
!include "StrFunc.nsh"

${StrLoc}

Name "N2N Edge GUI"

OutFile "n2ne_gui_install.exe"

RequestExecutionLevel admin

BrandingText "N2N Edge GUI Installer $$Rev$$"

InstallDir "$PROGRAMFILES\N2N Edge GUI"
InstallDirRegKey HKLM "Software\N2N Edge GUI" "Path"

!define MUI_FINISHPAGE_RUN "$INSTDIR\n2ne_gui.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch N2N Edge GUI"
!insertmacro MUI_PAGE_LICENSE "../COPYING"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Var is64bit
Var skip_service
Var restart_service

Icon "..\N2N Edge GUI\n2n.ico"

Section "N2N Edge GUI"
  SectionIn RO
  SetOutPath $INSTDIR

  SimpleSC::ServiceIsRunning "N2NEdge"
  Pop $0
  Pop $1
  StrCmp $0 "1" 0 start_main
  SimpleSC::StopService "N2NEdge"
  StrCpy $restart_service "1"

  start_main:
  CreateDirectory "$SMPROGRAMS\N2N Edge GUI"
  File "edge.exe"
  File "..\N2N Edge GUI\n2n.ico"

  WriteUninstaller "n2ne_gui_uninst.exe"
  WriteRegStr HKLM "SOFTWARE\N2N Edge GUI" "Path" '$INSTDIR'
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\N2N Edge GUI" "DisplayName" "N2N Edge GUI"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\N2N Edge GUI" "UninstallString" '"$INSTDIR\n2ne_gui_uninst.exe"'
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\N2N Edge GUI" "QuietUninstallString" '"$INSTDIR\n2ne_gui_uninst.exe" /S'
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\N2N Edge GUI" "InstallLocation" '"$INSTDIR"'
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\N2N Edge GUI" "DisplayIcon" '"$INSTDIR\n2n.ico"'

; --------------------------------------------------------
; TAP DRIVER
; --------------------------------------------------------

  SetOutPath "$INSTDIR\drv"

  StrCmp $is64bit "0" is32bit is64bit

  is32bit:
  File "..\tap-win32-bin\x86\tapinstall.exe"
  File "..\tap-win32-bin\x86\OemWin2k.inf"
  File "..\tap-win32-bin\x86\tap0901.cat"
  File "..\tap-win32-bin\x86\tap0901.sys"
  goto install_driver

  is64bit:
  File "..\tap-win32-bin\x64\tapinstall.exe"
  File "..\tap-win32-bin\x64\OemWin2k.inf"
  File "..\tap-win32-bin\x64\tap0901.cat"
  File "..\tap-win32-bin\x64\tap0901.sys"
  goto install_driver

  install_driver:
  nsExec::ExecToStack '"$INSTDIR\drv\tapinstall" find TAP0901'
  Pop $1
  Pop $2
  ${StrLoc} $0 $2 "No matching devices" 0
  StrCmp $0 "0" 0 start_service
  nsExec::ExecToLog '"$INSTDIR\drv\tapinstall" remove TAP0901'
  nsExec::ExecToLog '"$INSTDIR\drv\tapinstall" install OemWin2k.inf TAP0901'

; --------------------------------------------------------
; SERVICE
; --------------------------------------------------------
  start_service:

  StrCmp $skip_service "1" start_gui 0

  SetOutPath $INSTDIR

  File "..\Release\n2n_srv.exe"
  SimpleSC::InstallService "N2NEdge" "N2N Edge" "16" "2" '"$INSTDIR\n2n_srv.exe"' "" "" ""
  SimpleSC::SetServiceDescription "N2NEdge" "Provides N2N VPN Access"
  WriteRegStr HKLM "SOFTWARE\N2N Edge GUI\Parameters" "community" "community"
  WriteRegStr HKLM "SOFTWARE\N2N Edge GUI\Parameters" "enckey" "enckey"
  WriteRegStr HKLM "SOFTWARE\N2N Edge GUI\Parameters" "ip_address" ""
  WriteRegStr HKLM "SOFTWARE\N2N Edge GUI\Parameters" "keyfile" ""
  WriteRegDWORD HKLM "SOFTWARE\N2N Edge GUI\Parameters" "local_port" 0x00000000
  WriteRegStr HKLM "SOFTWARE\N2N Edge GUI\Parameters" "mac_address" ""
  WriteRegDWORD HKLM "SOFTWARE\N2N Edge GUI\Parameters" "mtu" 0x00000000
  WriteRegDWORD HKLM "SOFTWARE\N2N Edge GUI\Parameters" "multicast" 0x00000000
  WriteRegDWORD HKLM "SOFTWARE\N2N Edge GUI\Parameters" "packet_forwarding" 0x00000001
  WriteRegDWORD HKLM "SOFTWARE\N2N Edge GUI\Parameters" "resolve_ip" 0x00000000
  WriteRegStr HKLM "SOFTWARE\N2N Edge GUI\Parameters" "subnet_mask" ""
  WriteRegStr HKLM "SOFTWARE\N2N Edge GUI\Parameters" "supernode_addr" "supernode"
  WriteRegDWORD HKLM "SOFTWARE\N2N Edge GUI\Parameters" "supernode_port" 0x00001de6

; --------------------------------------------------------
; GUI TOOL
; --------------------------------------------------------
  start_gui:

  SetOutPath $INSTDIR

  CreateShortCut "$SMPrograms\N2N Edge GUI\N2N Edge GUI.lnk" "$INSTDIR\n2ne_gui.exe"
  File "..\Release\n2ne_gui.exe"

; --------------------------------------------------------
; FINAL
; --------------------------------------------------------
  CreateShortCut "$SMPROGRAMS\N2N Edge GUI\Uninstall N2N Edge GUI.lnk" "$INSTDIR\n2ne_gui_uninst.exe"
  StrCmp $restart_service "1" 0 done_final
  SimpleSC::StartService "N2NEdge" ""
  done_final:
SectionEnd

Function .onInit
  SimpleSC::ServiceIsRunning "N2NEdge"
  Pop $0
  Pop $1
  StrCmp $0 "1" 0 get_system_arch
  StrCpy $skip_service "1"

  get_system_arch:
  System::Call "kernel32::GetCurrentProcess() i.s"
  System::Call "kernel32::IsWow64Process(is, *i.s)"
  Pop $is64bit
FunctionEnd

UninstallText "This will uninstall N2N Edge GUI.  Click 'Uninstall' to continue."

Section "Uninstall"
  SimpleSC::StopService "N2NEdge"
  SimpleSC::RemoveService "N2NEdge"
  nsExec::ExecToLog '"$INSTDIR\drv\tapinstall" remove TAP0901'
  Delete "$INSTDIR\drv\*.*"
  Delete "$INSTDIR\*.*"
  Delete "$SMPROGRAMS\N2N Edge GUI\*.*"
  RMDir "$SMPROGRAMS\N2N Edge GUI"
  RMDir "$INSTDIR\drv"
  RMDir "$INSTDIR"
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\N2N Edge GUI"
  DeleteRegKey HKLM "SOFTWARE\N2N Edge GUI"
SectionEnd