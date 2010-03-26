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

#ifndef _H_N2NEGUI
#define _H_N2NEGUI

bool string_empty(WCHAR* str);
bool validate_options(HWND hwndDlg);
void update_addresses(HWND hwndDlg);
void update_service_status(HWND hwndDlg);
void CALLBACK refresh_screen(HWND hwndDlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void read_options(HWND hwndDlg);
void save_options(HWND hwndDlg);
void handle_command_event(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK dialog_proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void setup_system_menu(HWND hwndDlg);
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);


#endif
