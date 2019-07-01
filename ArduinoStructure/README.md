# Understanding Arduino files and folders

![Arduino logo](Arduino-logo.png)

In this tutorial, we will try to understand the internal structure of the Arduino IDE and how the build process works. This is useful mainly for debugging, troubleshooting compilation issues and use the build tools with other programs such as VSCode.

This is really an _attempt_ to document the structure of the IDE. As the whole structure is poorly documented (if you find some document that describes the internals of the files and folders, please let me know!!), there is **no guarantee that the information hereafter is fully correct**. This is all based on my experience and observations, mainly under Windows, and a lot of googling. So please do not flood me with questions, I'm no specialist of the Arduino IDE, just a little curious.

This text is a technical article more than a real tutorial, but a couple of tweaks are given at the end.

Here is a list of the software versions used in this tutorial:

- Arduino IDE version 1.8.9
- Arduino SAMD Board (32-bits ARM Cortex-M0+) version 1.8.1
- Gamebuino META Board version 1.2.2
- Gamebuino Library version 1.3.2

Obviously, with other versions, the content of this tutorial may need to be adapted.

## Arduino IDE flavors

To start with, there are several ways to install the Arduino IDE:

1. **Portable Gamebuino pre-configured Arduino IDE**: simplest installation when starting with the Gamebuino, set up by [decompressing the Gamebuino IDE archive into a folder](https://gamebuino.com/academy/workshop/start-with-your-gamebuino/quick-software-setup). As of today, it is based on a [portable](https://www.arduino.cc/en/Guide/PortableIDE) installation of the Arduino IDE in version 1.8.5.  A portable version of a more recent IDE could be setup quite easily. The advantage of such a setup is that everything is stored at the same place.

2. **Arduino IDE installed as an application**: classical installation when working with Arduino, set up by [executing the official Arduino IDE installer](https://gamebuino.com/academy/standalone/arduino-manual-setup). This kind of installation requires administrator rights and hence may not be possible.

3. **Arduino IDE installed in a folder**: classical installation without administrator rights under Windows, set up by choosing the _ZIP file for non admin install_ on the [official Arduino download page](https://www.arduino.cc/en/Main/Software) and decompressing it into a folder.

4. **Arduino IDE app from the Microsoft Store**: [Windows-specific packaging](https://www.microsoft.com/en-us/p/arduino-ide/9nblggh4rsd8) of the Arduino IDE, set up by installing as any other Windows app from the store. It is installed in an obscure folder in the user's home area and is hence difficult to use in other tools, but has the big advantage that board files are stored explicitly in the user's documents.

5. **Web-based Arduino IDE**: new style of IDE for Arduino, requiring the installation of a plugin and allowing to [code in the web-browser](https://create.arduino.cc/projecthub/Arduino_Genuino/getting-started-with-arduino-web-editor-on-various-platforms-4b3e4a). No description of this kind of installation in this tutorial, too far away from the other installations, and not likely to be useable with any other external tool.

## Location of Arduino-related files

Files are stored at different locations depending on the IDE installation. The location is difficult to follow, especially because several copies exist for some files and folders.

We will take the convention of using the slash as path separator for all paths except the Windows-specific ones where the backslash is used.

### Arduino IDE installation folder

In this section we look at the path where the Arduino IDE is installed. We will reference to it as `ARDUINO_IDE_PATH`, located at:

- `C:\Program Files (x86)\Arduino` for a standard Windows installation
- `C:\Program Files\WindowsApps\ArduinoLLC.ArduinoIDE_xxxxxxx` for a Windows installation from the Microsoft Store (where the `x` contain the version number of the Arduino Store app and some unique identifiers)
- `/Applications/Arduino.app` under macOS
- The folder where it was de-compressed in case of:
  - Portable installation
  - User-defined installation into a folder without administrator rights
  - Linux installation (the installer just creates a link to the main executable)

Under this folder, we will find the default boards, libraries and tools:

- `ARDUINO_IDE_PATH/libraries`: the [built-in Arduino _Standard libraries_](https://www.arduino.cc/en/Reference/Libraries), either applicable to the Gamebuino board (e.g. `SD`) or not (e.g. `GSM`)
- `ARDUINO_IDE_PATH/hardware`: built-in Arduino boards and tools, more specifically:
  - `ARDUINO_IDE_PATH/hardware/arduino/avr/cores/arduino`: all [AVR cores](https://www.arduino.cc/en/guide/cores). A "core" contains the functions of the Arduino API for a specific group of chips, i.e. it provides the definition and implementation of classical functions such as `millis()` or `sin()`. It contains, among others, the mandatory include file `Arduino.h`.
  - `ARDUINO_IDE_PATH/hardware/arduino/avr/libraries`: some default [built-in libraries for the AVR cores](https://github.com/arduino/ArduinoCore-avr/tree/master/libraries) such as `SPI` or `HID`.
  - `ARDUINO_IDE_PATH/hardware/tools`: the default tools including the AVR toolchain, where it is particularly difficult to understand what are the relevant files (e.g. many `include` folders in the tree).
- `ARDUINO_IDE_PATH/portable`: only present for a portable installation, containing in turn the sub-folders `packages` and `sketchbook`, described in the next sections

### Hardware `packages` folder in `portable`, `Arduino15`, `ArduinoData` or `.arduino15` folder

In this section we look at the folder named `packages`. This folder contains the **cores and tools from user-downloaded boards** (installed with the _Boards Manager_ in the IDE) and is either under `ARDUINO_IDE_PATH/portable` for a portable installation, or under the mysterious `Arduino15` folder.

The `Arduino15` folder, even if normally hidden, may look familiar to you because it contains the [preferences.txt file](https://www.arduino.cc/en/hacking/preferences) mentioned on the Preferences dialog of the Arduino IDE. The folder contains user preference files, the `staging` folder (used when downloading a library) and the `packages` folder.

Concatenated with the possible locations of its parent folder, the `packages` folder, that we will call unsurprisingly `PACKAGES_PATH`, should be located at:

- `C:\Users\[UserName]\AppData\Local\Arduino15\packages` on Windows with a classical installation
- `C:\Users\[UserName]\Documents\ArduinoData\packages` on Windows with an installation as Store App
- `~/Library/Arduino15/packages` on macOS
- `~/.arduino15/packages` on Linux
- `ARDUINO_IDE_PATH/portable/packages` for a portable installation

Under the `packages` folder, we will find the user-downloaded boards, where each board is in a single sub-folder (`arduino` for the Arduino SAMD 32-bits ARM Cortex-M0+, and `gamebuino` for the Gamebuino, _both are necessary_).

In each board sub-folder, we find _again_ the same kind of files as in the Arduino IDE folder. The following files and folders are part of the Gamebuino META board:

- `PACKAGES_PATH/gamebuino/hardware/samd/1.2.2/platform.txt`: definition of the tools and command options used in the build process
- `PACKAGES_PATH/gamebuino/hardware/samd/1.2.2/cores/arduino`: main folder with the "core" definition of the Gamebuino, containing for example the applicable `Arduino.h` file
- `PACKAGES_PATH/gamebuino/hardware/samd/1.2.2/libraries`: low-level libraries adapted to the Gamebuino board:
  - `SPI`: the [Serial Peripheral Interface](https://www.arduino.cc/en/reference/SPI), used to communicate with the TFT display
  - `HID`: USB [HID library](https://www.arduino.cc/en/Reference/HID), allowing the board to become a [USB Human Interface Device](https://en.wikipedia.org/wiki/USB_human_interface_device_class)
  - `Wire`: the [Wire library](https://www.arduino.cc/en/Reference/Wire), used for communication with I2C / TWI devices
  - `I2S`: communication via the [Inter-IC Sound (I2S) Bus](https://www.arduino.cc/en/Reference/I2S), used for sound
  - `USBHost`: the [USBHost library](https://www.arduino.cc/en/Reference/USBHost), allowing the board to become a USB Host accepting devices such as a mouse or a keyboard on the USB port
  - `SAMD_AnalogCorrection`: [SAMD_AnalogCorrection library](https://github.com/arduino/ArduinoCore-samd/tree/master/libraries/SAMD_AnalogCorrection) to set and enable the digital correction logic of the SAMD Analogic Digital Converter

The SAMD Board folder at `PACKAGES_PATH/arduino` is apparently the basis of the Gamebuino board definition. It contains almost the same files with more board variants, and in addition it provides the tools, including:

- `PACKAGES_PATH/arduino/tools/bossac`: Used for uploading the `.bin` to the board
- `PACKAGES_PATH/arduino/tools/openocd`: Used for on-chip debug (I personally never used it)
- `PACKAGES_PATH/arduino/tools/CMSIS`: Libraries of the vendor-unspecific [Cortex Microcontroller Software Interface Standard (CMSIS)](www.arm.com/cmsis) created by ARM (I have to try the fast DSP math functions)
- `PACKAGES_PATH/arduino/tools/CMSIS-Atmel`: CMSIS vendor-specific files for [ATMEL Smart-ARM (SAM) processors](https://en.wikipedia.org/wiki/Atmel_ARM-based_processors)
- `PACKAGES_PATH/arduino/tools/arm-none-eabi-gcc/7-2017q4`: the [GNU ARM Embedded Toolchain](https://launchpad.net/gcc-arm-embedded) with the [GNU binutils](https://www.gnu.org/software/binutils/) plus [GCC](https://gcc.gnu.org/) and many other libraries and include folders..

This folder requires some deeper analysis. In `PACKAGES_PATH/arduino/tools/arm-none-eabi-gcc/7-2017q4` you will find:

- `bin/arm-none-eabi-g++.exe`: this is the compiler executable, part of the [GNU ARM Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm). The strange name says that the tool generates object files compliant with the Embedded [Application Binary Interface (ABI)](https://developer.arm.com/products/architecture/system-architectures/software-standards/abi). The GCC compiler comes with a very [long list of options on the command line](https://manned.org/arm-none-eabi-g++).
- `lib/gcc/arm-none-eabi/7.2.1/include`: basis include files containing, e.g. `stdint.h` or `stdbool.h`, which may contain `#include_next` lines, saying to the pre-processor to seach the given file in the next include folder
- `arm-none-eabi/include`: next folder with again system includes, e.g. another version of `stdint.h`
- `arm-none-eabi/include/c++/7.2.1`: C++ specific includes e.g. triggered by including [`cstddef`](http://www.cplusplus.com/reference/cstddef/)

As a side note, to conclude this section, be aware that _the `packages` folder does include executables_! When installing a board, it is not clear that additional executables are downloaded. Under Windows, the location in `C:\Users\[UserName]\AppData\Local` may lead to strange effects (interferences with antivirus, difficulties with backup, etc). I would recommend building a portable version of the Arduino IDE if issues arise.

### Sketchbook folder with libraries

In this section we look at the path where sketches are stored a.k.a. sketchbook, that we will name `SKETCHBOOK_PATH`. This is the path set in the _File > Preferences_ of the Arduino IDE in the field _Sketchbook location_. It is at `ARDUINO_IDE_PATH/portable/sketchbook` for a portable installation, or by default in the `Arduino` folder of the user documents folder, or somewhere else if the path was changed in the preferences.

In the sketchbook folder, in addition to the user sketches, we will find the **Arduino libraries downloaded by the user**, i.e. installed with the "Library Manager" in the IDE (menu _Manage libraries.._) at `SKETCHBOOK_PATH/libraries`.

Each library is in a single sub-folder (normally `Gamebuino_META` for the Gamebuino), and in each library folder, the header files are in the `src` sub-folder and examples, if any, are in the `examples` sub-folder.

Please note that the Gamebuino META library _contains_ adaptations of several other libraries otherwise available with the _Library Manager_ (e.g. Display ST7735 from Adafruit). These libraries are stored in `SKETCHBOOK_PATH/libraries/Gamebuino_META/src/utility`. Looking carefully at these libraries, it can be noted that the main include files are always a little different in the original one and the one in `utility`. For instance, the main include file of the Adafruit GFX Library is named originally `Adafruit_GFX.h`, and renamed `Graphics.h` in the Gamebuino folder. Similarly, the Adafruit ST7735 (TFT display of the Gamebuino) uses the `Adafruit_ST7735.h` file and is renamed to `Display-ST7735.h` in `utility`. As a result, no interference is expected by using both versions of a library.

ATTENTION: For some reasons, the folder of a library may be named `arduino_xxxxxx`, where the `x` are numbers, instead of the expected name e.g. `Gamebuino_META`. The compilation will still work, but if an absolute path reference to the library is needed, then I would recommend deleting the `SKETCHBOOK_PATH/libraries` folder and re-install all required libraries.

### Applicable files

As we saw in the previous sections, there are plenty of possibilities to look for header files and tools, and multiple copies exist in the different folders.

The main selection principle is that user-downloaded tools/cores/libraries have priority over the default ones in the Arduino IDE installation folder.

For the Gamebuino, it means that the board definition is in `PACKAGES_PATH/gamebuino/hardware`, the tools come from `PACKAGES_PATH/arduino/tools` and the necessary libraries are in `SKETCHBOOK_PATH/libraries/Gamebuino_META`. Hence, finally, except the `arduino-builder` tool and the editor, nothing is used from the Arduino IDE installation, and it should not be necessary to download other libraries.

### Build process

The [arduino-builder](https://github.com/arduino/arduino-builder) does a pretty good job at identifying the files to include and chooses the correct tools transparently during the build process.

The best way to see how it works, and be sure about the applicable files, is to look at the verbose output of a compilation (see the next section). Alternatively, refer to the [official Arduino wiki page](https://github.com/arduino/Arduino/wiki/Build-Process).

The pre-processing phase of the Arduino build process is very complex compared to a normal compile/link build process. The majority of issues encountered while compiling a sketch are due to this phase. So it is important to note that:

- First of all, the arduino-builder creates a `sketch` folder, or re-use the existing one, in the build folder. This is the folder where the sketch-related files will be compiled.
- Then the `[sketch-name].ino.cpp` file is created in the `sketch` folder, built from the `[sketch-name].ino` as following:
  - The line `#include <Arduino.h>` is added at the beginning
  - Extra `.ino` files are concatenated in alphabetical order at the end
  - Function prototypes from the concatenated file are recognized and added at the beginning
- Additional files of the original sketch folder (such as header `.h` files) are copied to the `sketch` folder in build folder
- Missing include folders are incrementally discovered
- Finally the `[sketch-name].ino.cpp` is compiled

ATTENTION: The build process leads to interesting pitfalls:

- Unlike function prototypes, variable prototypes are not created automatically. As a result, if a new variable is created in an extra `.ino` file (typically the definition of a constant `Image` object), it is necessary to define it at the beginning of the main `.ino` file with `extern`.
- Similarly, there is no data type prototypes at the beginning of the file. Now, if a function prototype uses a sketch-defined type, then the compilation will fail because the type is defined after the prototype.
- It may happen that the include files are not correctly detected, hence include files are not copied in to the `sketch` folder and the compilation fails.

## Tweaks

In this section we look at some simple tweaks.

### Set the build path

There is no way to set the path where temporary files are stored during compilation in the Arduino IDE. The path is chosen as a folder named `arduino_build_xxxxxx` (where `x` are numbers) in the system-wide temporary folder (`C:\Temp` for Windows), plus another folder named `arduino_cache_xxxxxx` to store the compiled core.

The drawback with temporary folders is that the system may delete them, and the path may change.

Instead of the temporary folder you can **set a build folder** in your [preferences.txt file](https://www.arduino.cc/en/hacking/preferences):

- Click on the path mentioning `preferences.txt` at the bottom of the Preferences dialog window of the Arduino IDE to open the folder.
- **Close all windows of the Arduino IDE** before editing the `preferences.txt` file (otherwise your changes will be overwritten when you exit the IDE)
- Edit the file with a text editor and add the line `build.path=[myPath]` to use the path `[myPath]` as build folder. An absolute or relative path can be given (see the remarks below).
- Close the text editor
- Start again the Arduino IDE and compile your sketch

ATTENTION: According to my experience on Windows, if the build path is given as a relative path (e.g. just `build`) then the **build folder is created in the Arduino IDE folder** (so it needs to be writable). Prefer defining an absolute path (but starting with Arduino IDE 1.8.9, I don't know a way to create the build folder automatically in the sketch folder).

To conclude this section, I could not find any description of the parameters of the `preferences.txt` file, just either [an outdated file with comments](https://github.com/arduino/Arduino/blob/master/build/shared/lib/preferences.txt) or the [exhaustive list of parameters](https://github.com/arduino/arduino-builder/blob/master/constants/constants.go) in the `arduino-builder` source code. By the way, there is a foreseen variable to set the build path of the core, named `build.core.path` in `preferences.txt`, but it seems to be buggy. The core is always built into a temporary folder.

### Keep extra assembler and pre-processed source files after build

Normally the build process keeps only .o files (object files, i.e. compiled code before link pass) and .d files (text files listing the dependencies). Additional files can be saved as well, that can be important for debugging. To do so:

- Find the folder of the applicable `platform.txt` file, normally at `PACKAGES_PATH/gamebuino/hardware/samd/1.2.2`. As explained in this file, you can either modify the `platform.txt` file directly or, if you prefer, create a `platform.local.txt` file in the same folder for some settings We choose here the first method for simplicity.
- Edit the `platform.txt` file and change this line as following:
`compiler.cpp.extra_flags=-save-temps=obj`
- Re-compile you sketch and look at the build folder

For every compiled file, you will now find:

- A `.ii` file with the pre-processed code (`.ii` for C++, `.i` for C), which can be used to debug complex issues with macros
- A `.s` file with the assembler code, which can be used to check the result of optimization

### Determine pre-defined macros and include files in the compilation of a sketch

In addition to the different `#define` macros in the source code, some macros are pre-defined by GCC, depending on the environment and the language used. It may be necessary to know the pre-defined macros to better understand what is really applicable in the jungle of include files of the Arduino IDE.

It is possible to list the definition of such macros:

- Go to the folder at `PACKAGES_PATH/arduino/tools/arm-none-eabi-gcc/7-2017q4/bin`
- Create an empty file named with the extension `.cpp`. For the rest of this section we will assume that this file is named `C:\Temp\empty.cpp`, doing that under Windows.
- Execute `arm-none-eabi-g++.exe -dM -E C:\Temp\empty.cpp` to get a list of the pre-defined macros in a non-specific environment. The `-E` option tells GCC to use the pre-processor only, and the `-dM` to display the macros. As explained on the [list of pre-processor options](https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html), it _generates a list of `#define` directives for all the macros defined during the execution of the preprocessor_. But because the file is empty, the output is limited to pre-defined macros.

Note that you can create a file with the output of the command (instead of just displaying it) by redirecting the standard output to a file, e.g. add `> output.txt 2>&1` at the end of the command. The `2>&1` is necessary to re-direct the standard error stream to the standard output (it works _as well under Windows_).

Now, it gives a long list of `#define` instructions, but it does not reflect the complete set of options activated for the compilation of a sketch. The get a more precise view:

- Compile a sketch with verbose output (in the _File > Preferences_ dialog, activate the box _Show verbose output during: compilation_.)
- Search the string _Compiling sketch..._ in the compilation output and look at the long compilation command on the next line.
- Copy the text of the command in a command window and remove the `-o [filename]` option (destination object file), remove the `-I[filename]` options (include files would pollute the output with additional macros), replace the `[sketch_name].ino.cpp` by `C:\Temp\empty.cpp` and add `-E -dM` to list the macros. At the end, under Windows for the user `bfx`, the command will look like this (raw):
`C:\\Users\\bfx\\AppData\\Local\\Arduino15\\packages\\arduino\\tools\\arm-none-eabi-gcc\\7-2017q4/bin/arm-none-eabi-g++" -mcpu=cortex-m0plus -mthumb -c -g -Os -w -std=gnu++11 -ffunction-sections -fdata-sections -fno-threadsafe-statics -nostdlib --param max-inline-insns-single=500 -fno-rtti -fno-exceptions -MMD "-D__SKETCH_NAME__=\"\"\"Fractalino.ino\"\"\"" -DF_CPU=48000000L -DARDUINO=10809 -DARDUINO_SAMD_ZERO -DARDUINO_ARCH_SAMD -save-temps=obj -D__SAMD21G18A__ -DUSB_VID=0x2341 -DUSB_PID=0x804d -DUSBCON "-DUSB_MANUFACTURER=\"Arduino LLC\"" "-DUSB_PRODUCT=\"Arduino Zero\"" C:\Temp\empty.cpp -E -dM`
- Execute the adapted command

Now the command shows the **exhaustive list of macros that are not defined in the source code**, composed of:

- Macros pre-defined by GCC
- Macros defined on the compilation command line by the options `-D[name]=[value]` (like `-DARDUINO=10809` meaning that the Arduino IDE version _1.8.9_ is used)

The same kind of trick can be used to **list include files** for a sketch:

- Again find the compilation command following _Compiling sketch..._ in the verbose output of a sketch compilation
- Re-use the same command after adding `-H` (see previous paragraph)

The list of include files is displayed on the standard error stream, so if you want to re-direct the output to a file, make sure that you use the `2>&1` additional re-direction statement.

The output looks like this (absolute path and extra backslashes removed for readability). The indentation with dots shows the include level:

```
. packages\gamebuino\hardware\samd\1.2.2\cores\arduino/Arduino.h
.. packages\arduino\tools\arm-none-eabi-gcc\7-2017q4\lib\gcc\arm-none-eabi\7.2.1\include\stdbool.h
.. packages\arduino\tools\arm-none-eabi-gcc\7-2017q4\lib\gcc\arm-none-eabi\7.2.1\include\stdint.h
... packages\arduino\tools\arm-none-eabi-gcc\7-2017q4\arm-none-eabi\include\stdint.h
.... packages\arduino\tools\arm-none-eabi-gcc\7-2017q4\arm-none-eabi\include\machine\_default_types.h
..... packages\arduino\tools\arm-none-eabi-gcc\7-2017q4\arm-none-eabi\include\sys\features.h
etc
```

This is what I used to track the otherwise non-configurable includes of GCC. Alternatively, a less exhaustive list of include files can be deduced from `.d` and `.ii` files.

### Optimize sketch compilation for speed instead of size

The standard flags for compilation include the `-Os` option, which triggers optimization for size. This makes sense for Arduino in general,due to the limited flash size.

For games, it may be useful to [optimize the compilation for speed](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html). To do so:

- Edit the applicable `platform.txt` file, normally at `PACKAGES_PATH/gamebuino/hardware/samd/1.2.2`.
- Change all occurrences of `-Os` by `-O3`

If you are curious, the result of the optimization should be visible in the generated assembler code.

## Conclusion

I hope you learned a couple of things in this tutorial. It may evolve as I discover new things or re-fine my understanding.

If you find inconsistencies (and I'm sure there are some) please do not hesitate to report it (here or on Discord)!

That's all folks!
