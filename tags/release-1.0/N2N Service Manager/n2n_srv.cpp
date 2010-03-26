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

extern bool start_supernode;

int main (int argc, char** argv)
{
  if (argc > 1 && strcmp("supernode", argv[1]))
    start_supernode = true;

  SERVICE_TABLE_ENTRY st_entry[] = { { L"n2n_srv", service_main }, { NULL, NULL } };
  if (!StartServiceCtrlDispatcher(st_entry))
  {
    wprintf_s(L"%s:%d (%s) - Failed to launch service.\n", __FILE__, __LINE__, __FUNCTION__);
    return 1;
  }
  return 0;
}