# MSVC2019 code generation bug report

This repository contains all the info related to a bug that I've found in MSVC2019.

## Brief description

The code (see main.cpp in this repo) compiled with MSVC2019 produces `std::bad_cast` when it is not supposed to. I'm 99.9% sure that it is a compiler bug because:
- The only configuration where the bug is present is Release (Debug, RelWithDebugInfo and even MinSizeRel are OK).
- The problem cannot be reproduced with non-MSVC compilers (either on Windows or on other platforms, see "Additional information" section for details).
- Changing one source code line "fixes" the problem, but the change should not have any effect on the observable behavior (see "Additional information" section for details).

A preprocessed source file can be found as [main.i](https://github.com/kovdan01/bug-report-msvc2019-bad-cast/blob/master/main.i) in this repository. I've checked that the preprocessed file still repros the error.

## Reproduction

Initially the bug was found while running unit tests of msgpack library (see [this test case](https://github.com/msgpack/msgpack-c/blob/be4d971c62798eb59f8455dc77a4529748bcd08f/test/user_class.cpp#L178)). Unfortunately, I was not able to reproduce the bug without 3rd-party dependencies, and the sample depends on msgpack library (see CMakeLists.txt and main.cpp in this repository). Still, I'm 99.9% sure that the problem is **NOT** library-related, and this is a compiler bug (I've explained why in "Brief desctiption" section).

### Environment

- Output of `cl /Bv`:

  ```
  C:\>cl /Bv
  Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30038.1 for x86
  Copyright (C) Microsoft Corporation.  All rights reserved.

  Compiler Passes:
   C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\HostX86\x86\cl.exe:        Version 19.29.30038.1
   C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\HostX86\x86\c1.dll:        Version 19.29.30038.1
   C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\HostX86\x86\c1xx.dll:      Version 19.29.30038.1
   C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\HostX86\x86\c2.dll:        Version 19.29.30038.1
   C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\HostX86\x86\c1xx.dll:      Version 19.29.30038.1
   C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\HostX86\x86\link.exe:      Version 14.29.30038.1
   C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\HostX86\x86\mspdb140.dll:  Version 14.29.30038.1
   C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\HostX86\x86\1033\clui.dll: Version 19.29.30038.1

  cl : Command line error D8003 : missing source filename
  ```
- Contents of CL.command.1.tlog:

  ```
  ^D:\PROJECTS\BUG-REPORT-MSVC2019-BAD-CAST\MAIN.CPP
  /c /I"D:\PROJECTS\VCPKG\INSTALLED\X64-WINDOWS\INCLUDE" /nologo /W3 /WX- /diagnostics:column /O2 /Ob2 /D WIN32 /D _WINDOWS /D NDEBUG /D "CMAKE_INTDIR=\"Release\"" /D _MBCS /Gm- /EHsc /MD /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /GR /std:c++17 /Fo"BUG-REPORT-MSVC2019-BAD-CAST.DIR\RELEASE\\" /Fd"BUG-REPORT-MSVC2019-BAD-CAST.DIR\RELEASE\VC142.PDB" /external:env:EXTERNAL_INCLUDE /external:W3 /Gd /TP D:\PROJECTS\BUG-REPORT-MSVC2019-BAD-CAST\MAIN.CPP
  ```

- Info from Visual Studio Help->About:
  - Microsoft Visual Studio Community 2019
  - Version 16.10.3
- msgpack 3.3.0 (installed via vcpkg)
- CMake 3.20.5
- Windows 10.0.19042.928

### Steps to reproduce

1. Install msgpack via vcpkg: `vcpkg.exe install msgpack`.
2. Assuming current folder is this repository root, build the sample with the following commands:
   
   ```
   mkdir build
   cd build
   cmake -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" ..
   cmake --build . --config Release
   ```
   
3. Run the compiled sample: `Release\bug-report-msvc2019-bad-cast.exe`.

### Observable behavior

The output is:

```
[debug] pack double
[debug] double before convert
[debug] Exception while double tests: bad cast
[debug] pack int
[debug] int before convert
[debug] Exception while int tests: bad cast
```

### Expected behavior

The output is:

```
[debug] pack double
[debug] double before convert
[debug] double after convert
[test]  is double: OK
[test]  double == 1.0: OK
[test]  doubles equal: OK
[debug] pack int
[debug] int before convert
[debug] int after convert
[test]  is int: OK
[test]  int == 1: OK
[test]  ints equal: OK
```

## Additional information

- The problem reproduces with all C++ standard supported by the compiler: `/std:c++14`, `/std:c++17`, `/std:c++latest`.
- The problem reproduces on both 32- and 64-bit build (`-A Win32` and `-A x64` options in initial cmake configuration command line).
- There is **NO** problem when the code is compiled with g++ or clang++ under MSYS2 (Windows).
- There is **NO** problem when the code is compiled with g++ or clang++ (Linux) and Apple clang (macOS).
- The code was tested with UB-sanitizer and valgrind under Linux, and no problems were found.
- The problem is **NOT** specific for vcpkg's distribution of msgpack 3.3.0 - the problem reproduces with manually installed library (change `-DCMAKE_TOOLCHAIN_FILE="D:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"` to `-DCMAKE_PREFIX_PATH="D:/path/to/msgpack/prefix"` in initial cmake configuration command line).
- Changing line `msgpack::type::tuple<bool, msgpack::object> tuple(false, msgpack::object());` to `msgpack::type::tuple<bool, msgpack::object> tuple;` "fixes" the problem ([link](https://github.com/kovdan01/bug-report-msvc2019-bad-cast/blob/3c986d660ce2be5e6b96b8f674bee5348cd7f251/main.cpp#L49) to the sources). In that conditions using both default and two-parameters constructors should have the same effect and should not affect the observable behavior.
