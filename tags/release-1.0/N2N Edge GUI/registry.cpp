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
#include "registry.h"

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

int reg_set_dword(HKEY hkey, LPWSTR value_name, DWORD dword_val)
{
  // Set DWORD value in registry
  if (RegSetValueEx(hkey, value_name, NULL, REG_DWORD, (LPBYTE)&dword_val, sizeof(DWORD)) != ERROR_SUCCESS)
  {
    return 0;
  }
  return 1;
}

int reg_set_string(HKEY hkey, LPWSTR value_name, LPWSTR str_val)
{
  DWORD data_len = (wcslen(str_val) + 1) * sizeof(WCHAR);
  // Set string value in registry
  if (RegSetValueEx(hkey, value_name, NULL, REG_SZ, (LPBYTE)str_val, data_len) != ERROR_SUCCESS)
  {
    return 0;
  }
  return 1;
}