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
#include "Global.hpp"

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
			if (IsWindow(hwnd) && IsWindowVisible(hwnd) && // Visible
				(GetParent(hwnd) == GetWindow(hwnd, GW_OWNER) || GetParent(hwnd) == dsk) && // Level
				(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE) && // IsTaskbarWindow
				(!(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
			) {
				memset(wcs, 0, 255);
				// setlocale(LC_ALL, "");
				int captionlen = GetWindowTextLength(hwnd);
				GetWindowText(hwnd, (LPWSTR) wcs, captionlen + 1);
				QString caption = QString::fromStdWString(wcs);

				// 过滤无标题窗口
				if (captionlen > 0 && caption != Global::WND_TITLE) {
					DWORD pid;
					GetWindowThreadProcessId(hwnd, &pid);
					QString imgName = getImageName(pid);

					Wnd w(hwnd, caption, pid, imgName);
					wnds.push_back(w);
				}
			}
			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}
		return wnds;
	}

private:

	// https://blog.csdn.net/kingkee/article/details/98115433
	// 进程信息
	struct ProcessInfo {
		DWORD pid;
		QString image;
		ProcessInfo(DWORD pid, QString image) 
			: pid(pid), image(image) {}
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
			ProcessInfo pi(ps.th32ProcessID, QString::fromStdWString(ps.szExeFile));
			pis.push_back(pi);
		} while (Process32Next(hSnapshot, &ps));

		// 关闭快照句柄
		CloseHandle(hSnapshot);
		return pis;
	}

	// 通过 pid 获取进程名
	static QString getImageName(int pid) {
		vector<ProcessInfo> pis = GetProcessInfo();
		foreach (ProcessInfo pi, pis) {
			if (pi.pid == pid) {
				// pi.image.replace(QString(".exe"), QString(""));
				return pi.image;
			}
		}
		return "Unknown";
	}

	/////////////////////////////////////

public:

	// 设为活动窗口
	static void SetFGWindow(HWND hwnd) {
		HWND me = GetActiveWindow();
		if (IsIconic(hwnd)) ShowWindow(hwnd, SW_SHOWNOACTIVATE);
		SetForegroundWindow(hwnd);
		SetForegroundWindow(me);
	}

	// 修改窗口标题
	static void SetWindowName(HWND hwnd, QString newTitle) {
		SetWindowText(hwnd, newTitle.toStdWString().c_str());
	}	
	
	// 最小化窗口 状态栏可见
	static void MinimizeWindow(HWND hwnd) {
		ShowWindowAsync(hwnd, SW_MINIMIZE);
	}

	// 隐藏窗口 状态栏不可见
	static void HideWindow(HWND hwnd) {
		ShowWindowAsync(hwnd, SW_HIDE);
	}

	// 显示窗口
	static void UnHideWindow(HWND hwnd) {
		ShowWindowAsync(hwnd, SW_SHOW);
	}
};

#endif // UITLS_HPP
