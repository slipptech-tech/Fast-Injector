# Troubleshooting

This page contains solutions to common Fast Injector problems.

## Application Does Not Start

Try the following:

1. Make sure you are using a supported Windows version.
2. Download the latest release.
3. Check whether Windows Security or another antivirus blocked the executable.
4. If you built the application yourself, rebuild it using `Release | x64`.
5. Check Windows Event Viewer for application errors.

## Process Is Not Listed

Make sure:

* The target process is currently running.
* Fast Injector has the required permissions for the operation.
* The application and target process use compatible architectures.

For example, an x64-only build may not work with a 32-bit target process.

## DLL Cannot Be Selected

Check that:

* The file has a `.dll` extension.
* The DLL is not corrupted.
* The DLL matches the target process architecture.
* The file is accessible from your Windows account.

## Antivirus Detection

Security software may flag injector-related applications because of the techniques they use.

Do not disable antivirus protection simply because an application reports a detection.

If you built Fast Injector yourself, review the source code and dependencies before deciding whether to allow the executable.

## Build Errors

If Visual Studio reports missing headers or libraries:

1. Check the `deps` directory.
2. Check **Additional Include Directories**.
3. Check **Additional Library Directories**.
4. Verify that the required Visual Studio workload is installed.
5. Clean and rebuild the solution.

## ImGui Errors

If you see errors such as:

```text
cannot open source file "imgui.h"
```

verify that the ImGui source directory is included in:

```text
C/C++ → General → Additional Include Directories
```

## Still Having Problems?

When opening an issue, provide:

* Fast Injector version.
* Windows version.
* Visual Studio version if building from source.
* Build configuration.
* Exact error message.
* Steps to reproduce the problem.

Do not post passwords, tokens, personal information or other sensitive data.
