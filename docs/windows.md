### Setup for Windows 10/11
The following instructions give some hints how to setup a compile environment for Windows.
These examples use the UCRT toolchain but it schould also be possiblew to use the CLANG64 toolchain.

1. Install MSYS2 from https://www.msys2.org/
2. Install the GCC compiler and UCRT runtime\  
`pacman -Sy mingw-w64-ucrt-x86_64-gcc`
3. Install cmake and other build tools \
`pacman -Sy mingw-w64-ucrt-x86_64-cmake`
4. Install SDL2 for your toolchain  \
`pacman -Sy mingw-w64-ucrt-x86_64-SDL2`\
`pacman -Sy mingw-w64-ucrt-x86_64-SDL2_image`\
`pacman -Sy mingw-w64-ucrt-x86_64-SDL2_ttf`
4. Install ibyaml for your toolchain  \
 `pacman -Sy mingw-w64-ucrt-x86_64-libyaml`

#### Setup Visual Studio code to use the MSYS2 toolchain:

1. Add the C/C++ extension from Microsft https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools
2. Add the CMake-Tools extension. https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
3. Alternatively you can install the C/C++ Extension Pack which includes both of the above and some other helpfull extensions. https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack
4. Install the MSYS2 extension for VS Code. https://github.com/okhlybov/vscode-msys2

You can install all required extension by pasting the following code into the search field\  
`ms-vscode.cmake-tools ms-vscode.cpptools fougas.msys2`  
or  \
`ms-vscode.cpptools-extension-pack fougas.msys2`  
to use the extension pack.

#### Setup the toolchains in VS-Code

I recommend to configure the following setting by editing the ++ settings.json ++ file.
Mein looks as follows:
```
{
    "cmake.generator": "Ninja Multi-Config",
    "cmake.cmakePath": "${command:cmake.buildkit.cmake.exe}",
    "cmake.preferredGenerators": ["Unix Makefiles"],
    "cmake.configureSettings": {
        "CMAKE_MAKE_PROGRAM": "${command:cmake.buildkit.generator.exe}",
        "CMAKE_VERBOSE_MAKEFILE": false
    },
    "cmake.configureOnOpen": true,
    "[cmake]": {},
    "git.confirmSync": false,
    "git.autofetch": true,
    "window.zoomLevel": 1,
    "cmake.cacheInit": null,
    "terminal.integrated.profiles.windows": {
        "PowerShell": {
            "source": "PowerShell",
            "icon": "terminal-powershell"
        },
        "Command Prompt": {
            "path": [
                "${env:windir}\\Sysnative\\cmd.exe",
                "${env:windir}\\System32\\cmd.exe"
            ],
            "args": [],
            "icon": "terminal-cmd"
        },
        "Git Bash": {
            "source": "Git Bash"
        },
        "bash (MSYS2)": {
            "path": "C:\\msys64\\usr\\bin\\bash.exe",
            "args": [
                "--login",
                "-i"
            ],
            "env": {
                "MSYSTEM": "MINGW64",
                "CHERE_INVOKING": "1"
            }
        },
        "MSYS2 UCRT": {
            "path": "cmd.exe",
            "args": [
                "/c",
                "C:\\msys64\\msys2_shell.cmd -defterm -here -no-start -ucrt64"
            ]
        }
    },
    "terminal.integrated.defaultProfile.windows": "bash (MSYS2)"
}
```

Finally we need to setup the CMakeTools integration following the instructions in the MSYS2 extrension.

Issue the command Ctrl+Shift+P |> CMake: Edit User-Local CMake Kits and paste the configuration below:
```
[
  {
    "name": "MinGW32",
    "preferredGenerator": {"name": "MinGW Makefiles"},
    "environmentVariables": {"PATH": "${command:mingw32.path}"},
    "cmakeSettings": {
      "CMAKE_C_COMPILER": "${command:mingw32.cc.exe}",
      "CMAKE_CXX_COMPILER": "${command:mingw32.cxx.exe}",
      "CMAKE_Fortran_COMPILER": "${command:mingw32.fc.exe}",
      "MPI_C_COMPILER": "${command:mingw32.mpicc.exe}",
      "MPI_CXX_COMPILER": "${command:mingw32.mpicxx.exe}",
      "MPI_Fortran_COMPILER": "${command:mingw32.mpifort.exe}"
    },
    "keep": true
  },
  {
    "name": "MinGW64",
    "preferredGenerator": {"name": "MinGW Makefiles"},
    "environmentVariables": {"PATH": "${command:mingw64.path}"},
    "cmakeSettings": {
      "CMAKE_C_COMPILER": "${command:mingw64.cc.exe}",
      "CMAKE_CXX_COMPILER": "${command:mingw64.cxx.exe}",
      "CMAKE_Fortran_COMPILER": "${command:mingw64.fc.exe}",
      "MPI_C_COMPILER": "${command:mingw64.mpicc.exe}",
      "MPI_CXX_COMPILER": "${command:mingw64.mpicxx.exe}",
      "MPI_Fortran_COMPILER": "${command:mingw64.mpifort.exe}"
    },
    "keep": true
  },
  {
    "name": "UCRT64",
    "preferredGenerator": {"name": "MinGW Makefiles"},
    "environmentVariables": {"PATH": "${command:ucrt64.path}"},
    "cmakeSettings": {
      "CMAKE_C_COMPILER": "${command:ucrt64.cc.exe}",
      "CMAKE_CXX_COMPILER": "${command:ucrt64.cxx.exe}",
      "CMAKE_Fortran_COMPILER": "${command:ucrt64.fc.exe}",
      "MPI_C_COMPILER": "${command:ucrt64.mpicc.exe}",
      "MPI_CXX_COMPILER": "${command:ucrt64.mpicxx.exe}",
      "MPI_Fortran_COMPILER": "${command:ucrt64.mpifort.exe}"
    },
    "keep": true
  },
  {
    "name": "Clang32",
    "preferredGenerator": {"name": "MinGW Makefiles"},
    "environmentVariables": {"PATH": "${command:clang32.path}"},
    "cmakeSettings": {
      "CMAKE_C_COMPILER": "${command:clang32.cc.exe}",
      "CMAKE_CXX_COMPILER": "${command:clang32.cxx.exe}",
      "CMAKE_Fortran_COMPILER": "${command:clang32.fc.exe}",
      "MPI_C_COMPILER": "${command:clang32.mpicc.exe}",
      "MPI_CXX_COMPILER": "${command:clang32.mpicxx.exe}",
      "MPI_Fortran_COMPILER": "${command:clang32.mpifort.exe}"
    },
    "keep": true
  },
  {
    "name": "Clang64",
    "preferredGenerator": {"name": "MinGW Makefiles"},
    "environmentVariables": {"PATH": "${command:clang64.path}"},
    "cmakeSettings": {
      "CMAKE_C_COMPILER": "${command:clang64.cc.exe}",
      "CMAKE_CXX_COMPILER": "${command:clang64.cxx.exe}",
      "CMAKE_Fortran_COMPILER": "${command:clang64.fc.exe}",
      "MPI_C_COMPILER": "${command:clang64.mpicc.exe}",
      "MPI_CXX_COMPILER": "${command:clang64.mpicxx.exe}",
      "MPI_Fortran_COMPILER": "${command:clang64.mpifort.exe}"
    },
    "keep": true
  },
  {
    "name": "MSYS2",
    "preferredGenerator": {"name": "Unix Makefiles"},
    "environmentVariables": {"PATH": "${command:msys2.path}"},
    "cmakeSettings": {
      "CMAKE_C_COMPILER": "${command:msys2.cc.exe}",
      "CMAKE_CXX_COMPILER": "${command:msys2.cxx.exe}",
      "CMAKE_Fortran_COMPILER": "${command:msys2.fc.exe}",
    },
    "keep": true
  },
  {
    "name": "Cygwin32",
    "preferredGenerator": {"name": "Unix Makefiles"},
    "environmentVariables": {"PATH": "${command:cygwin32.path}"},
    "cmakeSettings": {
      "CMAKE_C_COMPILER": "${command:cygwin32.cc.exe}",
      "CMAKE_CXX_COMPILER": "${command:cygwin32.cxx.exe}",
      "CMAKE_Fortran_COMPILER": "${command:cygwin32.fc.exe}",
    },
    "keep": true
  },
  {
    "name": "Cygwin64",
    "preferredGenerator": {"name": "Unix Makefiles"},
    "environmentVariables": {"PATH": "${command:cygwin64.path}"},
    "cmakeSettings": {
      "CMAKE_C_COMPILER": "${command:cygwin64.cc.exe}",
      "CMAKE_CXX_COMPILER": "${command:cygwin64.cxx.exe}",
      "CMAKE_Fortran_COMPILER": "${command:cygwin64.fc.exe}",
    },
    "keep": true
  }
]
```



YAML
pacman -S mingw-w64-ucrt-x86_64-libyaml
