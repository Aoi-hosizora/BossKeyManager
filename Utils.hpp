#ifndef UITLS_HPP
#define UITLS_HPP

#ifdef UNICODE
#define GetWindowText GetWindowTextW
#else
#define GetWindowText GetWindowTextA
#endif 

#include <vector>
#include <algorithm>
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <QtCore/QtCore>
#include <locale>
#include "Wnd.h"

using namespace std;

class Utils {

public:

	// 获得当前所有窗口的信息
	static vector<Wnd> getAllWindows() {
		vector<Wnd> wnds;

		HWND dsk = GetDesktopWindow();
		HWND hwnd = GetWindow(dsk, GW_CHILD);

		TCHAR wcs[256] = {0};
		while (hwnd != nullptr) {
			// 过滤非窗口和不可见窗口
			if (IsWindow(hwnd) && IsWindowVisible(hwnd) && 
				(GetParent(hwnd) == GetWindow(hwnd, GW_OWNER) || GetParent(hwnd) == dsk)
			) {
				memset(wcs, 0, 255);
				// setlocale(LC_ALL, "");
				int captionlen = GetWindowTextLength(hwnd);
				GetWindowText(hwnd, (LPWSTR) wcs, captionlen + 1);
				string caption = wstring2string(wcs);

				// 过滤无标题窗口和 PM
				if (captionlen > 0 && caption != "Program Manager") {
					DWORD pid;
					GetWindowThreadProcessId(hwnd, &pid);
					string imgName = getImageName(pid);

					// 过滤侧边栏 TODO
					if (imgName != "sidebar") {
						Wnd w(hwnd, caption, pid, imgName);
						wnds.push_back(w);
					}
				}
			}
			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}
		return wnds;
	}

	// hwnd -> string
	static string hwnd2string(HWND hwnd) {
		// 0x12F0FF8
		return QString("0x%1").arg((int) hwnd, 6, 16, QLatin1Char('0')).toStdString();
	}

	static string wstring2string(wstring wstr) {
		return QString::fromStdWString(wstr).toStdString();
	}

private:

	// https://blog.csdn.net/kingkee/article/details/98115433
	// 进程信息
	struct ProcessInfo {
		DWORD pid;
		string image;
		ProcessInfo(DWORD pid, string image) : pid(pid), image(image) {}
	};

	// 获取进程信息
	static vector<ProcessInfo> GetProcessInfo() {
		STARTUPINFO st;
		PROCESS_INFORMATION pi;
		PROCESSENTRY32 ps;
		HANDLE hSnapshot;
		vector<ProcessInfo> pis;

		ZeroMemory(&st, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		st.cb = sizeof(STARTUPINFO);
		ZeroMemory(&ps, sizeof(PROCESSENTRY32));
		ps.dwSize = sizeof(PROCESSENTRY32);

		// 拍摄进程快照
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		// 快照拍摄失败
		if (hSnapshot == INVALID_HANDLE_VALUE)
			return pis;
		if (!Process32First(hSnapshot, &ps)) 
			return pis;

		do {
			ProcessInfo pi(ps.th32ProcessID, wstring2string(ps.szExeFile));
			pis.push_back(pi);
		} while (Process32Next(hSnapshot, &ps));

		// 关闭快照句柄
		CloseHandle(hSnapshot);
		return pis;
	}

	// 通过 pid 获取进程名
	static string getImageName(int pid) {
		vector<ProcessInfo> pis = GetProcessInfo();
		foreach (ProcessInfo pi, pis) {
			if (pi.pid == pid) {
				int pos = pi.image.find(".exe");
				return pi.image.replace(pos, 4, "");
			}
		}
		return "";
	}
};

#endif // UITLS_HPP
