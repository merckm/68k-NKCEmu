
## Setup for Linux

You can follow these steps to setup the required tools an libraries on Linux (UBUNTU):

1. If needed install GCC or CLang
2. Install cmake using\
`sudo apt install cmake`
3. Install Visual Studio Code and setup the extensions as explained below.
You will need to download Visual Sudo Code from Microsofts webside under:
4. Install SDL2 and related libraries  
`sudo apt install libsadl2-dev`\
`sudo apt install libsadl2-image-dev`\
`sudo apt install libsadl2-ttf-dev`
5. Install the YAML libreires\
`sudo apt install libyaml-dev`

### Setup Visual Studio code to use the LINUX toolchain:

1. Add the C/C++ extension from Microsft https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools
2. Add the CMake-Tools extension. https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
3. Alternatively you can install the C/C++ Extension Pack which includes both of the above and some other helpfull extensions. https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack