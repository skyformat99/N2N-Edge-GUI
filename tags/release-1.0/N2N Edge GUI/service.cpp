/*
  N2N Edge GUI - GUI Configuration utility for n2n edge
  Copyright (C) 2010  Ryan M. Dorn
  https://sourceforge.net/projects/n2nedgegui/
  $Id$

  This file is part of N2N Edge GUI.

  N2N Edge GUI is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  N2N Edge GUI is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with N2N Edge GUI.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include "service.h"

SC_HANDLE sc_manager, sc_handle;

bool init_service_control()
{
  sc_manager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
  if (!sc_manager)
  {
    MessageBox(NULL, L"An error occurred connecting to the service manager.", L"Error", MB_OK | MB_ICONSTOP);
    return false;
  }
  sc_handle = OpenService(sc_manager, L"N2NEdge", SERVICE_ALL_ACCESS);
  if (!sc_handle)
  {
    MessageBox(NULL, L"An error occurred connecting to the 'N2NEdge' service.", L"Error", MB_OK | MB_ICONSTOP);
    CloseServiceHandle(sc_manager);
    return false;
  }
  return true;
}

void close_service_control()
{
  CloseServiceHandle(sc_handle);
  CloseServiceHandle(sc_manager);
}

DWORD get_service_status()
{
  SERVICE_STATUS_PROCESS ssp;
  if (get_service_info(&ssp))
  {
    return ssp.dwCurrentState;
  }
  return 0;
}

bool get_service_info(SERVICE_STATUS_PROCESS* ssp)
{
  ZeroMemory(ssp, sizeof(SERVICE_STATUS_PROCESS));
  DWORD dw_bytes_needed;
  if (QueryServiceStatusEx(sc_handle, SC_STATUS_PROCESS_INFO, (LPBYTE)ssp,
                           sizeof(SERVICE_STATUS_PROCESS), &dw_bytes_needed))
  {
    return true;
  }
  return false;
}

bool service_wait(DWORD status)
{
  DWORD dw_start, dw_check, dw_wait;
  SERVICE_STATUS_PROCESS ssp;

  get_service_info(&ssp);

  dw_start = GetTickCount();
  dw_check = ssp.dwCheckPoint;
  while (ssp.dwCurrentState != status)
  {
    dw_wait = ssp.dwWaitHint / 10;
    if (dw_wait < 1000)
      dw_wait = 1000;
    else if (dw_wait > 10000)
      dw_wait = 10000;

    Sleep(dw_wait);

    get_service_info(&ssp);

    if (ssp.dwCheckPoint > dw_check)
    {
      dw_start = GetTickCount();
      dw_check = ssp.dwCheckPoint;
    }
    else
    {
      if (GetTickCount() - dw_start > ssp.dwWaitHint)
      {
        break;
      }
    }
  }
  return (ssp.dwCurrentState == status);
}

void start_service()
{
  StartService(sc_handle, 0, NULL);

  if (!service_wait(SERVICE_RUNNING))
  {
    MessageBox(NULL, L"Unable to start service.", L"Error", MB_OK | MB_ICONSTOP);
  }
}

void stop_service()
{
  SERVICE_STATUS_PROCESS ssp;
  get_service_info(&ssp);
  
  ControlService(sc_handle, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp);

  if (!service_wait(SERVICE_STOPPED))
  {
    MessageBox(NULL, L"Unable to stop service.", L"Error", MB_OK | MB_ICONSTOP);
  }
}