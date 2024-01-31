
## Setup for MacOSX

You can follow these steps to setup the required tools an libraries on MacOSX (Tested on M1 hardware):

1. If needed install CLang C compiler tools with XCode CLI tools\
`xcode-select install`
2. Install cmake using homebrew. The arch statment is only needed on ARM silicon.\
`<arch -arm64> brew install cmake`
3. Install Visual Studio Code and setup the extensions as explained below.
You will need to download Visual Sudo Code from Microsofts webside under:
4. Install SDL2 and related libraries  
`<arch -arm64> brew install sdl2`\
`<arch -arm64> brew install sdl2-image`\
`<arch -arm64> brew install sdl2-ttf`
5. Install the YAML libreires\
`<arch -arm64> brew install libyaml`

### Setup Visual Studio code to use the LINUX toolchain:

1. Add the C/C++ extension from Microsft https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools
2. Add the CMake-Tools extension. https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
3. Alternatively you can install the C/C++ Extension Pack which includes both of the above and some other helpfull extensions. https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack