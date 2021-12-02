# UEFI File Manager (UFM)
A dual pane file manager with a text based user interface for UEFI environment. Distributed under the BSD-2-Clause Plus Patent License.

_Requires EFI Shell v2.0 and above for correct work_

## Installation:
1) Move all files from Library to ShellPkg/Library

2) Add the following line to ShellPkg/ShellPkg.dsc, **section "Components"**:

```
ShellPkg/Library/UefiShellUfmCommandLib/UefiShellUfmCommandLib.inf
```

### If you want to integrate UFM in Shell:
1) Add the following line to ShellPkg/ShellPkg.dsc, **section "LibraryClasses"**, in build of any version of the shell:

```
ShellPkg/Application/Shell/Shell.inf {
	...
	<LibraryClasses>
		...
		NULL|ShellPkg/Library/UefiShellUfmCommandLib/UefiShellUfmCommandLib.inf # <- add this line
}
```

### If you want standalone application:
1) Move all files from Application, Include to ShellPkg/Application and ShellPkg/Include

2) Add the following lines to ShellPkg/ShellPkg.dsc:

**Section LibraryClasses.common:**
```
UfmCommandLib|ShellPkg/Library/UefiShellUfmCommandLib/UefiShellUfmCommandLib.inf
```

**Section Components:**
```
ShellPkg/Application/UfmApp/UfmApp.inf
```

3) Add the following line to ShellPkg/ShellPkg.dec, **section "LibraryClasses"**:

```
UfmCommandLib|Include/Library/UfmCommandLib.h
```
