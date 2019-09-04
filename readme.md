# BossKeyManager
+ A boss key manager for the programs shown in the taskbar

### Support actions
+ Minimize the window directly and the icon in taskbar will not hide
	+ `ShowWindowAsync(hwnd, SW_MINIMIZE)`

+ Hide the window directly and the icon in taskbar also
	+ `ShowWindowAsync(hwnd, SW_HIDE)`

### Screenshot

![Screenshot](./assets/Screenshot.jpg)

### References
+ [Limit QKeySequence/QKeySequenceEdit to only one shortcut](https://stackoverflow.com/questions/30006562/limit-qkeysequence-qkeysequenceedit-to-only-one-shortcut)
+ [cmdow](https://github.com/ritchielawrence/cmdow)
+ [getting-a-list-of-all-open-windows-in-c-and-storing-them](https://stackoverflow.com/questions/42589496/getting-a-list-of-all-open-windows-in-c-and-storing-them)
+ [获取 Windows 操作系统当前所有进程PID、进程名称](https://blog.csdn.net/kingkee/article/details/98115433)
+ [qxtglobalshortcut5](https://github.com/ddqd/qxtglobalshortcut5)