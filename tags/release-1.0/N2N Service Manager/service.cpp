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

#include <windows.h>
#include <stdio.h>
#include "service.h"

SERVICE_STATUS srv_status;
SERVICE_STATUS_HANDLE srv_handle;
HANDLE wait_handle;
HANDLE pid;
WCHAR exe_path[MAX_PATH];
WCHAR command_line[1024];
bool start_supernode = false;

void log_event(unsigned short type, WCHAR* format, ...)
{
  WCHAR message[4096];
  WCHAR* strings[2];
  int n, size;
  va_list arg;

  // Construct the message
  size = sizeof(message);
  va_start(arg, format);
  n = _vsnwprintf_s(message, size, size, format, arg);
  va_end(arg);

  // Check success
  if (n < 0 || n >= size) return;

  // Construct strings array
  strings[0] = message;
  strings[1] = 0;

  // Open event log
  HANDLE handle = RegisterEventSource(0, L"n2n_srv");
  if (!handle) return;

  // Log the message
  if (!ReportEvent(handle, type, 0, 0, 0, 1, 0, (const WCHAR**)strings, 0))
  {
    wprintf_s(L"%s:%d (%s) - ReportEvent() failed.\n", __FILE__, __LINE__, __FUNCTION__);
  }

  // Close event log
  DeregisterEventSource(handle);
}

int build_exe_path(WCHAR* exe_path, int buf_len)
{
  DWORD exe_buf_len = buf_len * sizeof(WCHAR);

  // Open registry key
  HKEY hkey;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\N2N Edge GUI", NULL, KEY_READ, &hkey) != ERROR_SUCCESS)
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Error opening registry key.\n", __FILE__, __LINE__, __FUNCTION__);
    return 0;
  }

  // Get executable path
  if (RegQueryValueEx(hkey, L"Path", NULL, NULL, (LPBYTE)exe_path, &exe_buf_len) != ERROR_SUCCESS)
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Unable to read 'Path' registry value.\n", __FILE__, __LINE__, __FUNCTION__);
    RegCloseKey(hkey);
    return 0;
  }
  RegCloseKey(hkey);
  return 1;
}

int build_command_line_edge(WCHAR* exe_path, WCHAR* command_line, int buf_len)
{
  command_line[0] = 0;
  WCHAR ret_val[512];
  DWORD ret_dword = 0;

  // Use 'ptr' to append to the end of the command line
  WCHAR* ptr = command_line;
  ptr += swprintf_s(command_line, buf_len, L"\"%s\\edge.exe\"", exe_path);

  // Open registry key
  HKEY hkey;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\N2N Edge GUI\\Parameters", NULL, KEY_READ, &hkey) != ERROR_SUCCESS)
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Error opening registry key.\n", __FILE__, __LINE__, __FUNCTION__);
    return 0;
  }

  // Community
  if (!reg_get_string(hkey, L"community", ret_val, 512)) return 0;
  ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -c %s", ret_val);

  // Encryption key
  if (!reg_get_string(hkey, L"enckey", ret_val, 512)) return 0;
  if (wcslen(ret_val) != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -k %s", ret_val);
  }

  // IP address
  if (!reg_get_string(hkey, L"ip_address", ret_val, 512)) return 0;
  if (wcslen(ret_val) != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -a %s", ret_val);
  }
  else
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -a dhcp:0.0.0.0");
  }

  // Encryption key file
  if (!reg_get_string(hkey, L"keyfile", ret_val, 512)) return 0;
  if (wcslen(ret_val) != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -K %s", ret_val);
  }

  // Local Port
  if (!reg_get_dword(hkey, L"local_port", &ret_dword)) return 0;
  if (ret_dword != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -p %d", ret_dword);
  }

  // MAC address
  if (!reg_get_string(hkey, L"mac_address", ret_val, 512)) return 0;
  if (wcslen(ret_val) != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -m %s", ret_val);
  }

  // MTU
  if (!reg_get_dword(hkey, L"mtu", &ret_dword)) return 0;
  if (ret_dword != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -M %d", ret_dword);
  }

  // Multicast
  if (!reg_get_dword(hkey, L"multicast", &ret_dword)) return 0;
  if (ret_dword != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -E");
  }

  // Packet forwarding
  if (!reg_get_dword(hkey, L"packet_forwarding", &ret_dword)) return 0;
  if (ret_dword != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -r");
  }

  // Resolve supernode IP
  if (!reg_get_dword(hkey, L"resolve_ip", &ret_dword)) return 0;
  if (ret_dword != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -b");
  }

  // Subnet mask
  if (!reg_get_string(hkey, L"subnet_mask", ret_val, 512)) return 0;
  if (wcslen(ret_val) != 0)
  {
    ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -s %s", ret_val);
  }
  // Supernode address
  if (!reg_get_string(hkey, L"supernode_addr", ret_val, 512)) return 0;
  ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -l %s", ret_val);

  // Supernode port
  if (!reg_get_dword(hkey, L"supernode_port", &ret_dword)) return 0;
  ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L":%d", ret_dword);

  return 1;
}

int build_command_line_supernode(WCHAR* exe_path, WCHAR* command_line, int buf_len)
{
  command_line[0] = 0;
  DWORD ret_dword = 0;

  // Use 'ptr' to append to the end of the command line
  WCHAR* ptr = command_line;
  ptr += swprintf_s(command_line, buf_len, L"\"%s\\supernode.exe\"", exe_path);

  // Open registry key
  HKEY hkey;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\N2N Edge GUI\\Parameters", NULL, KEY_READ, &hkey) != ERROR_SUCCESS)
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Error opening registry key.\n", __FILE__, __LINE__, __FUNCTION__);
    return 0;
  }

  // Supernode server port
  if (!reg_get_dword(hkey, L"supernode_server_port", &ret_dword)) return 0;
  ptr += swprintf_s(ptr, buf_len - (ptr - command_line), L" -l %d", ret_dword);

  return 1;
}

int reg_get_dword(HKEY hkey, LPWSTR value_name, LPDWORD ret_dword)
{
  // Fetch DWORD value from registry
  DWORD buf_size = sizeof(DWORD);
  if (RegQueryValueEx(hkey, value_name, NULL, NULL, (LPBYTE)ret_dword, &buf_size) != ERROR_SUCCESS)
  {
    *ret_dword = 0;
    return 0;
  }
  return 1;
}

int reg_get_string(HKEY hkey, LPWSTR value_name, LPWSTR ret_str, DWORD buf_size)
{
  // Fetch string value from registry
  if (RegQueryValueEx(hkey, value_name, NULL, NULL, (LPBYTE)ret_str, &buf_size) != ERROR_SUCCESS)
  {
    return 0;
  }
  return 1;
}

int start_service()
{
  if (pid) return 0;

  // Allocate a STARTUPINFO structure for a new process
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(STARTUPINFO);

  // Allocate a PROCESSINFO structure for the process
  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  // Launch executable
  if (!CreateProcess(NULL, command_line, NULL, NULL, NULL, NULL, NULL, exe_path, &si, &pi))
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Unable to launch supernode process.\n", __FILE__, __LINE__, __FUNCTION__);
    return stop_service(3);
  }
  pid = pi.hProcess;

  // Signal successful start
  srv_status.dwCurrentState = SERVICE_RUNNING;
  SetServiceStatus(srv_handle, &srv_status);

  return 0;
}


int stop_service(unsigned long exitcode)
{
  // Signal we are stopping
  srv_status.dwCurrentState = SERVICE_STOP_PENDING;
  SetServiceStatus(srv_handle, &srv_status);

  // Do nothing if the server isn't running
  if (pid)
  {
    TerminateProcess(pid, 0);
    pid = 0;
  }

  // Signal we stopped
  srv_status.dwCurrentState = SERVICE_STOPPED;
  if (exitcode)
  {
    srv_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    srv_status.dwServiceSpecificExitCode = exitcode;
  }
  else
  {
    srv_status.dwWin32ExitCode = NO_ERROR;
    srv_status.dwServiceSpecificExitCode = 0;
  }
  SetServiceStatus(srv_handle, &srv_status);

  return exitcode;
}

VOID CALLBACK end_service(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
  // check exit code
  unsigned long ret = 0;
  GetExitCodeProcess(pid, &ret);

  pid = 0;

  log_event(EVENTLOG_INFORMATION_TYPE, L"%s:%d (%s) - Process exited with return code %u.\n", __FILE__, __LINE__, __FUNCTION__, ret);

  // Wait 5 seconds, then restart process
  Sleep(5000);
  while (monitor_service())
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Failed to restart service.\n", __FILE__, __LINE__, __FUNCTION__);
    Sleep(30000);
  }
}

int monitor_service()
{
  // Set service status to started
  int ret = start_service();
  if (ret)
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Unable to start service.\n", __FILE__, __LINE__, __FUNCTION__);
    return ret;
  }

  // Monitor service
  if (!RegisterWaitForSingleObject(&wait_handle, pid, end_service, 0, INFINITE, WT_EXECUTEONLYONCE | WT_EXECUTELONGFUNCTION))
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Unable to call RegisterWaitForSingleObject.\n", __FILE__, __LINE__, __FUNCTION__);
  }
  return 0;
}

DWORD WINAPI service_ctrl_handler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
  switch (dwControl)
  {
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
      stop_service(0);
      return NO_ERROR;
  }
  return ERROR_CALL_NOT_IMPLEMENTED;
}

VOID WINAPI service_main(DWORD dwNumServiceArgs, LPWSTR *lpServiceArgVectors)
{
  ZeroMemory(&srv_status, sizeof(SERVICE_STATUS));
  srv_status.dwCheckPoint = 0;
  srv_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  srv_status.dwCurrentState = SERVICE_RUNNING;
  srv_status.dwServiceSpecificExitCode = 0;
  srv_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
  srv_status.dwWaitHint = 1000;
  srv_status.dwWin32ExitCode = NO_ERROR;

  // Build path and command line parameters
  if (!build_exe_path(exe_path, MAX_PATH))
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Error building executable path.\n", __FILE__, __LINE__, __FUNCTION__);
    return;
  }
  int ret = 0;
  if (start_supernode)
  {
    ret= build_command_line_supernode(exe_path, command_line, 1024);
  }
  else
  {
    ret =build_command_line_edge(exe_path, command_line, 1024);
  }
  if (!ret)
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Error building command line.\n", __FILE__, __LINE__, __FUNCTION__);
    return;
  }

  srv_handle = RegisterServiceCtrlHandlerEx(L"n2n_srv", service_ctrl_handler, 0);
  if (!srv_handle)
  {
    log_event(EVENTLOG_ERROR_TYPE, L"%s:%d (%s) - Unable to register service control handler.\n", __FILE__, __LINE__, __FUNCTION__);
    return;
  }

  pid = NULL;
  monitor_service();
}