/*
  N2N Service Manager - Utility to run n2n applications as a service
  Copyright (C) 2010  Ryan M. Dorn
  https://sourceforge.net/projects/n2nedgegui/
  $Id$

  This file is part of N2N Edge GUI.

  N2N Service Manager is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  N2N Service Manager is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with N2N Service Manager.  If not, see <http://www.gnu.org/licenses/>.
  ---------------------------------------------------------------
  Code based on NSSM - the Non-Sucking Service Manager
  Copyright (C) 2010 Iain Patterson
  http://tterson.net/src/nssm
*/


#ifndef _H_SERVICE
#define _H_SERVICE

void log_event(unsigned short type, WCHAR* format, ...);
int build_exe_path(WCHAR* exe_path, int buf_len);
int build_command_line_edge(WCHAR* exe_path, WCHAR* command_line, int buf_len);
int build_command_line_supernode(WCHAR* exe_path, WCHAR* command_line, int buf_len);
int reg_get_dword(HKEY hkey, LPWSTR value_name, LPDWORD ret_dword);
int reg_get_string(HKEY hkey, LPWSTR value_name, LPWSTR ret_str, DWORD buf_size);
int start_service();
int stop_service(unsigned long exitcode);
VOID CALLBACK end_service(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
int monitor_service();
DWORD WINAPI service_ctrl_handler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
VOID WINAPI service_main(DWORD dwNumServiceArgs, LPWSTR *lpServiceArgVectors);

#endif
