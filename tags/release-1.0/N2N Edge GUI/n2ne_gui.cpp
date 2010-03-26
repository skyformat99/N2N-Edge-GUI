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
#include <commctrl.h>
#include <stdlib.h>
#include "n2ne_gui.h"
#include "res.h"
#include "service.h"
#include "net.h"
#include "registry.h"

#pragma comment(lib, "comctl32.lib")

#define is_item_checked(x,y) (SendDlgItemMessage(x, y, BM_GETCHECK, 0, 0) == BST_CHECKED)

WCHAR szClassName[] = L"N2N Edge GUI";
HICON h_icon;
HICON h_icon_sm;

bool string_empty(WCHAR* str)
{
  if (wcslen(str) == 0) return true;
  for (int i = 0; i < (int)wcslen(str); i++)
  {
    if (str[i] != ' ') return false;
  }
  return true;
}

bool validate_options(HWND hwndDlg)
{
  WCHAR tmp_buf[256];
  WCHAR err_str[256];
  int buf_len = 256;
  bool ret_val = true;

  // IP Address
  GetDlgItemText(hwndDlg, IDC_EDT_IPADDRESS, tmp_buf, buf_len);
  if (is_item_checked(hwndDlg, IDC_CHK_IPADDRESS) && !validate_ipv4_address(tmp_buf))
  {
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_IPADDRESS));
    wcscpy_s(err_str, 256, L"Invalid IP address");
    ret_val = false;
  }

  // Subnet mask
  GetDlgItemText(hwndDlg, IDC_EDT_SUBNETMASK, tmp_buf, buf_len);
  if (is_item_checked(hwndDlg, IDC_CHK_SUBNETMASK) && !validate_ipv4_address(tmp_buf))
  {
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_SUBNETMASK));
    wcscpy_s(err_str, 256, L"Invalid subnet mask");
    ret_val = false;
  }

  // Community
  GetDlgItemText(hwndDlg, IDC_EDT_COMMUNITY, tmp_buf, buf_len);
  if (string_empty(tmp_buf))
  {
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_COMMUNITY));
    wcscpy_s(err_str, 256, L"Community is required");
    ret_val = false;
  }

  // Encryption key
  GetDlgItemText(hwndDlg, IDC_EDT_ENCKEY, tmp_buf, buf_len);
  if (is_item_checked(hwndDlg, IDC_CHK_ENCKEY) && string_empty(tmp_buf))
  {
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_ENCKEY));
    wcscpy_s(err_str, 256, L"Encryption key is required");
    ret_val = false;
  }

  // Key file
  GetDlgItemText(hwndDlg, IDC_EDT_KEYFILE, tmp_buf, buf_len);
  if (is_item_checked(hwndDlg, IDC_CHK_KEYFILE) && string_empty(tmp_buf))
  {
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_KEYFILE));
    wcscpy_s(err_str, 256, L"Key file is required");
    ret_val = false;
  }

  // Supernode port
  GetDlgItemText(hwndDlg, IDC_EDT_SUPERNODEPORT, tmp_buf, buf_len);
  if (!validate_number_range(tmp_buf, 1, 65535))
  {
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_SUPERNODEPORT));
    wcscpy_s(err_str, 256, L"Invalid supernode port");
    ret_val = false;
  }

  // MTU
  GetDlgItemText(hwndDlg, IDC_EDT_MTU, tmp_buf, buf_len);
  if (is_item_checked(hwndDlg, IDC_CHK_MTU) && string_empty(tmp_buf))
  {
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_MTU));
    wcscpy_s(err_str, 256, L"MTU is required");
    ret_val = false;
  }

  // Local port
  GetDlgItemText(hwndDlg, IDC_EDT_LOCALPORT, tmp_buf, buf_len);
  if (is_item_checked(hwndDlg, IDC_CHK_LOCALPORT) && !validate_number_range(tmp_buf, 1, 65535))
  {
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_LOCALPORT));
    wcscpy_s(err_str, 256, L"Invalid local port");
    ret_val = false;
  }

  // MAC address
  GetDlgItemText(hwndDlg, IDC_EDT_MACADDRESS, tmp_buf, buf_len);
  if (is_item_checked(hwndDlg, IDC_CHK_MACADDRESS) && !validate_mac_address(tmp_buf))
  {
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_MACADDRESS));
    wcscpy_s(err_str, 256, L"Invalid MAC address");
    ret_val = false;
  }

  // Finished
  if (!ret_val)
  {
    MessageBox(hwndDlg, err_str, L"Error", MB_OK | MB_ICONSTOP);
  }
  return ret_val;
}

void update_addresses(HWND hwndDlg)
{
  if (get_service_status() == SERVICE_RUNNING)
  {
    WCHAR ip_address[16];
    WCHAR mac_address[18];
    get_addresses(ip_address, mac_address);
    SetDlgItemText(hwndDlg, IDC_EDT_CUR_IP, ip_address);
    SetDlgItemText(hwndDlg, IDC_EDT_CUR_MAC, mac_address);
  }
  else
  {
    SetDlgItemText(hwndDlg, IDC_EDT_CUR_IP, L"0.0.0.0");
    SetDlgItemText(hwndDlg, IDC_EDT_CUR_MAC, L"00:00:00:00:00:00");
  }
}

void update_service_status(HWND hwndDlg)
{
  HWND btn_start = GetDlgItem(hwndDlg, IDC_BTN_START);
  HWND btn_stop = GetDlgItem(hwndDlg, IDC_BTN_STOP);
  DWORD service_status = get_service_status();
  switch (service_status)
  {
    case SERVICE_STOPPED:
      SetDlgItemText(hwndDlg, IDC_STC_SRV_STATUS, L"Stopped");
      EnableWindow(btn_start, TRUE);
      EnableWindow(btn_stop, FALSE);
      break;
    case SERVICE_RUNNING:
      SetDlgItemText(hwndDlg, IDC_STC_SRV_STATUS, L"Started");
      EnableWindow(btn_start, FALSE);
      EnableWindow(btn_stop, TRUE);
      break;
    default:
      SetDlgItemText(hwndDlg, IDC_STC_SRV_STATUS, L"Unknown");
      EnableWindow(btn_start, TRUE);
      EnableWindow(btn_stop, TRUE);
      break;
  }
}

void CALLBACK refresh_screen(HWND hwndDlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  update_service_status(hwndDlg);
  update_addresses(hwndDlg);
}

void read_options(HWND hwndDlg)
{
  WCHAR tmp_buf[256];
  DWORD buf_len = 256;
  DWORD dword_buf;
  HKEY hkey;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\N2N Edge GUI\\Parameters", NULL, KEY_READ, &hkey) != ERROR_SUCCESS)
  {
    MessageBox(hwndDlg, L"The registry key could not be opened.", L"Error", MB_OK | MB_ICONSTOP);
    return;
  }
  // Community
  reg_get_string(hkey, L"community", tmp_buf, buf_len);
  SetDlgItemText(hwndDlg, IDC_EDT_COMMUNITY, tmp_buf);

  // Encryption key
  reg_get_string(hkey, L"enckey", tmp_buf, buf_len);
  SetDlgItemText(hwndDlg, IDC_EDT_ENCKEY, tmp_buf);
  SendDlgItemMessage(hwndDlg, IDC_CHK_ENCKEY, BM_SETCHECK, (string_empty(tmp_buf) ? BST_UNCHECKED : BST_CHECKED), 0);
  EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_ENCKEY), !string_empty(tmp_buf));
  EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_KEYFILE), string_empty(tmp_buf));

  // IP address
  reg_get_string(hkey, L"ip_address", tmp_buf, buf_len);
  SetDlgItemText(hwndDlg, IDC_EDT_IPADDRESS, tmp_buf);
  SendDlgItemMessage(hwndDlg, IDC_CHK_IPADDRESS, BM_SETCHECK, (string_empty(tmp_buf) ? BST_UNCHECKED : BST_CHECKED), 0);
  EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_IPADDRESS), !string_empty(tmp_buf));

  // Key file
  reg_get_string(hkey, L"keyfile", tmp_buf, buf_len);
  SetDlgItemText(hwndDlg, IDC_EDT_KEYFILE, tmp_buf);
  SendDlgItemMessage(hwndDlg, IDC_CHK_KEYFILE, BM_SETCHECK, (string_empty(tmp_buf) ? BST_UNCHECKED : BST_CHECKED), 0);
  EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_KEYFILE), !string_empty(tmp_buf));
  EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ENCKEY), string_empty(tmp_buf));

  // Local Port
  reg_get_dword(hkey, L"local_port", &dword_buf);
  SetDlgItemInt(hwndDlg, IDC_EDT_LOCALPORT, dword_buf, FALSE);
  SendDlgItemMessage(hwndDlg, IDC_CHK_LOCALPORT, BM_SETCHECK, (dword_buf == 0 ? BST_UNCHECKED : BST_CHECKED), 0);
  EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_LOCALPORT), !string_empty(tmp_buf));

  // MAC address
  reg_get_string(hkey, L"mac_address", tmp_buf, buf_len);
  SetDlgItemText(hwndDlg, IDC_EDT_MACADDRESS, tmp_buf);
  SendDlgItemMessage(hwndDlg, IDC_CHK_MACADDRESS, BM_SETCHECK, (string_empty(tmp_buf) ? BST_UNCHECKED : BST_CHECKED), 0);
  EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_MACADDRESS), !string_empty(tmp_buf));

  // MTU
  reg_get_dword(hkey, L"mtu", &dword_buf);
  SetDlgItemInt(hwndDlg, IDC_EDT_MTU, dword_buf, FALSE);
  SendDlgItemMessage(hwndDlg, IDC_CHK_MTU, BM_SETCHECK, (dword_buf == 0 ? BST_UNCHECKED : BST_CHECKED), 0);
  EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_MTU), !string_empty(tmp_buf));

  // Multicast
  reg_get_dword(hkey, L"multicast", &dword_buf);
  SendDlgItemMessage(hwndDlg, IDC_CHK_MULTICAST, BM_SETCHECK, (dword_buf == 0 ? BST_UNCHECKED : BST_CHECKED), 0);

  // Packet Forwarding
  reg_get_dword(hkey, L"packet_forwarding", &dword_buf);
  SendDlgItemMessage(hwndDlg, IDC_CHK_PKTFORWARD, BM_SETCHECK, (dword_buf == 0 ? BST_UNCHECKED : BST_CHECKED), 0);

  // Resolve supernode IP
  reg_get_dword(hkey, L"resolve_ip", &dword_buf);
  SendDlgItemMessage(hwndDlg, IDC_CHK_RESOLVE, BM_SETCHECK, (dword_buf == 0 ? BST_UNCHECKED : BST_CHECKED), 0);

  // Subnet mask
  reg_get_string(hkey, L"subnet_mask", tmp_buf, buf_len);
  SetDlgItemText(hwndDlg, IDC_EDT_SUBNETMASK, tmp_buf);
  SendDlgItemMessage(hwndDlg, IDC_CHK_SUBNETMASK, BM_SETCHECK, (string_empty(tmp_buf) ? BST_UNCHECKED : BST_CHECKED), 0);
  EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_SUBNETMASK), !string_empty(tmp_buf));

  // Supernode address
  reg_get_string(hkey, L"supernode_addr", tmp_buf, buf_len);
  SetDlgItemText(hwndDlg, IDC_EDT_SUPERNODEADDR, tmp_buf);

  // Supernode port
  reg_get_dword(hkey, L"supernode_port", &dword_buf);
  SetDlgItemInt(hwndDlg, IDC_EDT_SUPERNODEPORT, dword_buf, FALSE);
}

void save_options(HWND hwndDlg)
{
  if (!validate_options(hwndDlg)) return;
  WCHAR tmp_buf[256];
  DWORD buf_len = 256;
  HKEY hkey;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\N2N Edge GUI\\Parameters", NULL, KEY_READ | KEY_WRITE, &hkey) != ERROR_SUCCESS)
  {
    MessageBox(hwndDlg, L"The registry key could not be opened.", L"Error", MB_OK | MB_ICONSTOP);
    return;
  }
  // Community
  GetDlgItemText(hwndDlg, IDC_EDT_COMMUNITY, tmp_buf, buf_len);
  reg_set_string(hkey, L"community", tmp_buf);

  // Encryption key
  if (is_item_checked(hwndDlg, IDC_CHK_ENCKEY))
  {
    GetDlgItemText(hwndDlg, IDC_EDT_ENCKEY, tmp_buf, buf_len);
    reg_set_string(hkey, L"enckey", tmp_buf);
  }
  else
  {
    reg_set_string(hkey, L"enckey", L"");
  }

  // IP Address
  if (is_item_checked(hwndDlg, IDC_CHK_IPADDRESS))
  {
    GetDlgItemText(hwndDlg, IDC_EDT_IPADDRESS, tmp_buf, buf_len);
    reg_set_string(hkey, L"ip_address", tmp_buf);
  }
  else
  {
    reg_set_string(hkey, L"ip_address", L"");
  }

  // Key file
  if (is_item_checked(hwndDlg, IDC_CHK_KEYFILE))
  {
    GetDlgItemText(hwndDlg, IDC_EDT_KEYFILE, tmp_buf, buf_len);
    reg_set_string(hkey, L"keyfile", tmp_buf);
  }
  else
  {
    reg_set_string(hkey, L"keyfile", L"");
  }

  // Local port
  if (is_item_checked(hwndDlg, IDC_CHK_LOCALPORT))
  {
    GetDlgItemText(hwndDlg, IDC_EDT_LOCALPORT, tmp_buf, buf_len);
    reg_set_dword(hkey, L"local_port", (DWORD)_wtoi(tmp_buf));
  }
  else
  {
    reg_set_dword(hkey, L"local_port", 0);
  }

  // MAC address
  if (is_item_checked(hwndDlg, IDC_CHK_MACADDRESS))
  {
    GetDlgItemText(hwndDlg, IDC_EDT_MACADDRESS, tmp_buf, buf_len);
    reg_set_string(hkey, L"mac_address", tmp_buf);
  }
  else
  {
    reg_set_string(hkey, L"mac_address", L"");
  }

  // MTU
  if (is_item_checked(hwndDlg, IDC_CHK_MTU))
  {
    GetDlgItemText(hwndDlg, IDC_EDT_MTU, tmp_buf, buf_len);
    reg_set_dword(hkey, L"mtu", (DWORD)_wtoi(tmp_buf));
  }
  else
  {
    reg_set_dword(hkey, L"mtu", 0);
  }

  // Multicast
  reg_set_dword(hkey, L"multicast", (is_item_checked(hwndDlg, IDC_CHK_MULTICAST) ? 1 : 0));

  // Packet Forwarding
  reg_set_dword(hkey, L"packet_forwarding", (is_item_checked(hwndDlg, IDC_CHK_PKTFORWARD) ? 1 : 0));

  // Resolve supernode IP
  reg_set_dword(hkey, L"resolve_ip", (is_item_checked(hwndDlg, IDC_CHK_RESOLVE) ? 1 : 0));

  // Subnet mask
  if (is_item_checked(hwndDlg, IDC_CHK_SUBNETMASK))
  {
    GetDlgItemText(hwndDlg, IDC_EDT_SUBNETMASK, tmp_buf, buf_len);
    reg_set_string(hkey, L"subnet_mask", tmp_buf);
  }
  else
  {
    reg_set_string(hkey, L"subnet_mask", L"");
  }

  // Supernode address
  GetDlgItemText(hwndDlg, IDC_EDT_SUPERNODEADDR, tmp_buf, buf_len);
  reg_set_string(hkey, L"supernode_addr", tmp_buf);

  // Supernode port
  GetDlgItemText(hwndDlg, IDC_EDT_SUPERNODEPORT, tmp_buf, buf_len);
  reg_set_dword(hkey, L"supernode_port", (DWORD)_wtoi(tmp_buf));

  // Finished
  RegCloseKey(hkey);

  if (MessageBox(NULL, L"Settings saved.  (Re)start service?", L"Restart service", MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
    stop_service();
    Sleep(500);
    start_service();
    update_service_status(hwndDlg);
  }
}

void handle_command_event(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (HIWORD(wParam != BN_CLICKED)) return;
  switch (LOWORD(wParam))
  {
    case IDC_BTN_START:
      start_service();
      update_service_status(hwndDlg);
      update_addresses(hwndDlg);
      break;
    case IDC_BTN_STOP:
      stop_service();
      update_service_status(hwndDlg);
      update_addresses(hwndDlg);
      break;
    case IDC_BTN_SAVE:
      save_options(hwndDlg);
      break;
    case IDC_BTN_READ:
      read_options(hwndDlg);
      break;
    case IDC_BTN_EXIT:
      EndDialog(hwndDlg, NULL);
      break;
    case IDC_CHK_IPADDRESS:
    {
      bool checked = is_item_checked(hwndDlg, IDC_CHK_IPADDRESS);
      EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_IPADDRESS), checked);
      SendDlgItemMessage(hwndDlg, IDC_CHK_PKTFORWARD, BM_SETCHECK, (checked ? BST_UNCHECKED : BST_CHECKED), 0);
      break;
    }
    case IDC_CHK_SUBNETMASK:
      EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_SUBNETMASK), is_item_checked(hwndDlg, IDC_CHK_SUBNETMASK));
      break;
    case IDC_CHK_ENCKEY:
    {
      bool checked = is_item_checked(hwndDlg, IDC_CHK_ENCKEY);
      EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_ENCKEY), checked);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_KEYFILE), !checked);
      break;
    }
    case IDC_CHK_KEYFILE:
    {
      bool checked = is_item_checked(hwndDlg, IDC_CHK_KEYFILE);
      EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_KEYFILE), checked);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ENCKEY), !checked);
      break;
    }
    case IDC_CHK_MTU:
      EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_MTU), is_item_checked(hwndDlg, IDC_CHK_MTU));
      break;
    case IDC_CHK_LOCALPORT:
      EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_LOCALPORT), is_item_checked(hwndDlg, IDC_CHK_LOCALPORT));
      break;
    case IDC_CHK_MACADDRESS:
      EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_MACADDRESS), is_item_checked(hwndDlg, IDC_CHK_MACADDRESS));
      break;
  }
}

void setup_system_menu(HWND hwndDlg)
{
  HMENU sys_menu = GetSystemMenu(hwndDlg, FALSE);
  AppendMenu(sys_menu, MF_SEPARATOR, NULL, NULL);
  AppendMenu(sys_menu, MF_STRING, IDM_ABOUT, L"About N2N Edge GUI..");
}

INT_PTR CALLBACK dialog_proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_CLOSE:
      KillTimer(hwndDlg, IDT_POLL);
      DestroyWindow(hwndDlg);
      break;
    case WM_INITDIALOG:
    {
      setup_system_menu(hwndDlg);
      LOGFONT lfont;
      HWND hwnd_ip, hwnd_mac;
      hwnd_ip = GetDlgItem(hwndDlg, IDC_EDT_CUR_IP);
      hwnd_mac = GetDlgItem(hwndDlg, IDC_EDT_CUR_MAC);
      HFONT hfont = (HFONT)SendMessage(hwnd_ip, WM_GETFONT, 0, 0);
      GetObject(hfont, sizeof(lfont), &lfont);
      lfont.lfWeight = FW_BOLD;
      hfont = CreateFontIndirect(&lfont);
      SendMessage(hwnd_ip, WM_SETFONT, (WPARAM)hfont, 0);
      SendMessage(hwnd_mac, WM_SETFONT, (WPARAM)hfont, 0);
      SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)h_icon);
      SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)h_icon_sm);
      SetTimer(hwndDlg, IDT_POLL, 5000, refresh_screen);
      update_service_status(hwndDlg);
      update_addresses(hwndDlg);
      read_options(hwndDlg);
      break;
    }
    case WM_COMMAND:
      handle_command_event(hwndDlg, uMsg, wParam, lParam);
      break;
    case WM_SYSCOMMAND:
      if (wParam == IDM_ABOUT)
      {
        MessageBox(hwndDlg, L"N2N Edge GUI $Rev$", L"About N2N Edge GUI", MB_OK | MB_ICONINFORMATION);
        break;
      }
      return FALSE;
      break;
    default:
      return FALSE;
  }
  return TRUE;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Initialize
  INITCOMMONCONTROLSEX icx;
  icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
  InitCommonControlsEx(&icx);

  h_icon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON32), IMAGE_ICON, 32, 32, 0);
  h_icon_sm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON32), IMAGE_ICON, 16, 16, 0);

  if (!init_service_control())
  {
    return 1;
  }

  // Run GUI window
  INT_PTR res = DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, dialog_proc);

  // Finalize and close handles
  close_service_control();

  return 0;
}
