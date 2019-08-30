# ScreenShotByProcessName
ScreenShot implement by C/C++ via VC 6.0

It will capture screen while target process exist.

It will get all process names every 2 second.

In default, only one param follow the command, it can capture 200 pictures then exit.

While capture successfuly, pictures can be saved at the path where ScreenShotByProcessName.exe is.

Pictures' name is the keyword/param you set,like Chrome1.jpg, Chrome2.jpg...

## Why VC6.0

Also, I consider windows XP SP3 can run it well, sometimes, program via VC6.0 can achieve my goal.

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

error stat number meaning you can reference:
[Status Enumeration](https://docs.microsoft.com/en-us/windows/win32/api/gdiplustypes/ne-gdiplustypes-status), such as `stat = 2` means `InvalidParameter`.

## ChangeLog

- add judgment for args, without keyword follow, the program will exit directly.
- add judgment for `CaptureImage()`, if none bmp file captured, `Convert2png()` will exit directly.
- add error print for `CaptureImage()` while `WriteFile()`