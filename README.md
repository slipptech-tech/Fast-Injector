<p align="center">
  <img width="805" height="244" alt="FastInject-removebg-preview" src="https://github.com/user-attachments/assets/3fc74f23-58fb-4064-99c9-b25ed3ca7fca">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white">
  <img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white">
  <img src="https://img.shields.io/badge/Build-Release-2EA44F?style=for-the-badge">
  <img src="https://img.shields.io/badge/Visual%20Studio-2022-5C2D91?style=for-the-badge&logo=visualstudio&logoColor=white">
</p>


# Fast Injector

Fast Injector is a lightweight Windows DLL injection utility written in C++.

The project is designed as a simple, fast and minimal injector for development, testing and educational purposes. The source code is available for users who want to inspect, modify and build the project themselves.

## Features

* Native C++ application
* x64 support
* Simple and lightweight interface
* DLL selection
* Process selection
* Fast operation
* Open-source code
* Built with Visual Studio 2022
* DirectX 11 / ImGui-based interface

## Requirements

* Windows 10 or Windows 11
* Visual Studio 2022
* Desktop development with C++
* Windows 10/11 SDK
* MSVC C++ build tools
* Git (optional)

## Building from Source

### 1. Install Visual Studio 2022

Install Visual Studio 2022 with the following workload:

**Desktop development with C++**

Make sure the following components are installed:

* MSVC v143 C++ build tools
* Windows 10/11 SDK
* C++ CMake tools for Windows (if required by the project)

### 2. Open the project

Clone or download the repository and open the `.vcxproj` file in Visual Studio 2022.

Example project structure:

```text
Fast Injector/
├── deps/
│   └── imgui/
├── src/
│   └── FastInjector/
│       └── FastInjector/
│           └── FastInjector.vcxproj
└── ...
```

### 3. Configure the project

In Visual Studio, open:

**Project → Properties**

Set:

**Configuration**

```text
All Configurations
```

**Platform**

```text
x64
```

Then configure:

**C/C++ → Language → C++ Language Standard**

```text
ISO C++20 Standard (/std:c++20)
```

**Advanced → Character Set**

```text
Use Unicode Character Set
```

For a standalone executable, the runtime can be configured as:

```text
C/C++ → Code Generation → Runtime Library
Multi-threaded (/MT)
```

### 4. Configure dependencies

If the project uses Dear ImGui, make sure the `deps` directory is located relative to the project as expected.

For example:

```text
C:\Users\YourName\Downloads\Fast Injector\deps\imgui
```

If the project uses:

```text
$(ProjectDir)..\..\deps
```

Visual Studio resolves this path relative to the `.vcxproj` directory.

Check:

**C/C++ → General → Additional Include Directories**

and

**Linker → General → Additional Library Directories**

if the project requires them.

### 5. Build

At the top of Visual Studio select:

```text
Release
x64
```

Then select:

**Build → Build Solution**

or press:

```text
Ctrl + Shift + B
```

If the build succeeds, Visual Studio will place the executable in a directory similar to:

```text
x64\Release\
```

or:

```text
bin\x64\Release\
```

depending on the project's configuration.

## Troubleshooting

### `cannot open source file`

Check that all dependencies are present and that the **Additional Include Directories** point to the correct folders.

### `cannot open file .lib`

Check:

**Linker → General → Additional Library Directories**

and:

**Linker → Input → Additional Dependencies**

### `imgui.h: No such file or directory`

Make sure the ImGui directory is present and its path is included under:

```text
C/C++ → General → Additional Include Directories
```

### No EXE appears after building

Check the **Output** and **Error List** windows in Visual Studio. Also verify that you selected:

```text
Release
x64
```


you want to build Fast Injector yourself, you can clone the repository using Git.

Open PowerShell or Command Prompt and run:

git clone https://github.com/YOUR-USERNAME/YOUR-REPOSITORY.git

Then enter the project directory:

cd YOUR-REPOSITORY


## Disclaimer

Fast Injector is provided for educational, development and testing purposes. Use it only with software and processes you own or have permission to modify. The authors are not responsible for misuse of the software.

## License

This project is open source. See the repository's license file for the applicable license terms.
