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

#ifndef _H_REGISTRY
#define _H_REGISTRY

int reg_get_dword(HKEY hkey, LPWSTR value_name, LPDWORD ret_dword);
int reg_get_string(HKEY hkey, LPWSTR value_name, LPWSTR ret_str, DWORD buf_size);
int reg_set_dword(HKEY hkey, LPWSTR value_name, DWORD dword_val);
int reg_set_string(HKEY hkey, LPWSTR value_name, LPWSTR str_val);

#endif