#ifndef UITLS_HPP
#define UITLS_HPP

#include <Windows.h>
#include <vector>
#include "Wnd.h"

using namespace std;

class Utils {
public:
	// 获得当前所有窗口的句柄和标题
	static vector<Wnd> getAllWindows() {
		vector<Wnd> wnds;
		HWND dsk = GetDesktopWindow();
		HWND hwnd = GetWindow(dsk, GW_CHILD);
		wchar_t wcs[526] = {0};

		while (hwnd != nullptr) {
			memset(wcs, 0, 255);
			int len = GetWindowTextLength(hwnd) ;
			GetWindowTextW(hwnd, wcs, 255);

			wstring wst(wcs);
			string str = wstring2string(wst);

			if (IsWindowVisible(hwnd) && len > 0  && str != "Program Manager") {
				DWORD pid; string pname;
				GetWindowThreadProcessId(hwnd, &pid);

				Wnd w(hwnd, wstring2string(wst), pid);

				wnds.push_back(w);
			}

			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}
		return wnds;
	}

 	static string wstring2string(wstring wstr) {  
		string result;  
		int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);  
		char* buffer = new char[len + 1];  
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);  
		buffer[len] = '\0';
		result.append(buffer);  
		delete[] buffer;  
		return result;
	}
};

#endif // UITLS_HPP
