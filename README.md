# ScreenShotByProcessName
ScreenShot implement by C/C++ via VC 6.0

It will capture screen while target process exist.

It will get all process names every 2 second.

In default, only one param follow the command, it can capture 200 pictures then exit.

While capture successfuly, pictures can be saved at the path where ScreenShotByProcessName.exe is.

Pictures' name is the keyword/param you set,like Chrome1.jpg, Chrome2.jpg...

## Why VC6.0

Also, I consider windows XP SP3 can run it well, sometimes, program via VC6.0 can achive my goal.

## Passed test platform

- Windows XP SP3
- Windows 7
- Windows 10

## Usage

    ScreenShotByProcessName.exe <keyword> [<capture number>]

- `<keyword>`: Your target process name's keyword, Case Sensitively.
- `[<capture number>]`: optional param, how many pictures you want capture, default setting is 200

eg:

- Capture 10 pictures if there is/are process name(s) cantain "Chrome"
```
ScreenShotByProcessName.exe Chrome 10
```

- Capture pictures 200
```
ScreenShotByProcessName.exe Chrome
```

## Notice

It can not return any error at all, except stat occur, it means convert from bmp to png error, what the stat number meaning you can reference:
[Status Enumeration](https://docs.microsoft.com/en-us/windows/win32/api/gdiplustypes/ne-gdiplustypes-status), such as `stat = 2` means `InvalidParameter`.

If you want to catch other errors, just debug from source code.