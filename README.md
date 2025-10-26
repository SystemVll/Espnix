> # ⚠️ Warning
> **This project is in its early stages and may not be fully functional.**

<div>
  <img align="left" width="100" src="https://github.com/Inplex-sys/espnix/assets/69421356/090b8ea2-e131-4995-b0c7-1aba19afd5dc" />
  <h3 align="left">Espnix</h3>
</div>

<hr/>
An Open-Source Unix like system that is made to run on ESP32 and STM32 micro-chips.

## Overview

Espnix provides a lightweight Unix-like environment for microcontrollers, offering familiar shell commands, file system operations, and process management within the constraints of embedded hardware. It brings the power and flexibility of Unix to resource-limited devices.

## Features

### Core System
* **Unix-like Shell**: Command-line interface with familiar commands
* **File System**: SD card based file system with standard hierarchical structure
* **Persistent Storage**: Automatic SD card detection with fallback to initramfs
* **First Boot Initialization**: Automatic creation of Unix-like directory structure on first startup
* **Smart Storage Management**: Seamless switching between initramfs and SD card storage

### File System & Storage
* **Automatic Directory Structure**: Creates 22 standard Unix directories on first boot (/bin, /boot, /dev, /etc, /home, /lib, /mnt, /opt, /proc, /root, /run, /sbin, /srv, /sys, /tmp, /usr, /var, and more)
* **Initialization Marker**: Uses /sys/.init file to track system initialization status
* **SD Card Hot Detection**: Clear notifications when SD card is missing or not properly inserted
* **Data Persistence**: All changes saved to SD card; warnings displayed when running in initramfs mode

### Compiler & Runtime
* **Built-in Compiler**: Compile source code (.es files) to bytecode (.enix files)
* **Virtual Machine**: Stack-based VM for executing compiled bytecode
* **Scripting Language**: Support for variables, operators, conditionals, loops, and functions
* **Bytecode Format**: Custom .enix binary format for portable executable code
* **Runtime Execution**: Execute compiled programs with the 'run' command

### Development Tools
* **compile Command**: Compiles Espnix script files to executable bytecode
* **run Command**: Executes compiled .enix bytecode files
* **Lexical Analysis**: Token-based source code parsing
* **Code Generation**: Optimized bytecode generation from source
* **Error Handling**: Comprehensive compilation and runtime error reporting

### Networking
* **WiFi Management**: Built-in WiFi configuration through `iwctl` command
* **Network Utilities**: Tools for managing wireless connections

### System Architecture
* **Module System**: Extensible architecture for adding new functionality
* **Dynamic Execution**: Load and execute binaries directly from SD card
* **Multi-platform**: Supports both ESP32 and STM32 microcontrollers
* **Clean Code Structure**: Separated header and implementation files for all components

## Getting Started

### Prerequisites

-   ESP32 or STM32 development board
-   SD card (minimum 1GB recommended)
-   Arduino IDE or PlatformIO
-   USB cable for programming

### Installation

1. Clone the repository:

    ```
    git clone https://github.com/Inplex-sys/Espnix.git
    ```

2. Open the project in Arduino IDE or PlatformIO

3. Connect your microcontroller via USB

4. Format an SD card as FAT32 (the system will automatically create the directory structure on first boot)

5. Upload the firmware to your device

6. Insert the SD card into your device

**Note**: On first boot, Espnix will automatically detect the SD card and create a complete Unix-like directory structure including /bin, /boot, /dev, /etc, /home, /lib, /mnt, /opt, /proc, /root, /run, /sbin, /srv, /sys, /tmp, /usr, /var, and subdirectories. A marker file (/sys/.init) is created to prevent re-initialization on subsequent boots.

### Basic Usage

Once Espnix is running on your device, connect to the serial terminal at 250000 baud. You'll be greeted with the Espnix boot sequence and shell prompt:

```
  ███████╗███████╗██████╗ ███╗   ██╗██╗██╗  ██╗
  ██╔════╝██╔════╝██╔══██╗████╗  ██║██║╚██╗██╔╝
  █████╗  ███████╗██████╔╝██╔██╗ ██║██║ ╚███╔╝ 
  ██╔══╝  ╚════██║██╔═══╝ ██║╚██╗██║██║ ██╔██╗ 
  ███████╗███████║██║     ██║ ╚████║██║██╔╝ ██╗
  ╚══════╝╚══════╝╚═╝     ╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

[ INFO ] Espnix version 1.0 booting...
[ INFO ] Platform: ESP32
[  OK  ] CPU frequency: 240 MHz
[ INFO ] Initializing initramfs (temporary root filesystem)
[  OK  ] Initramfs mounted at /
[ INFO ] Attempting to mount real filesystem from SD card
[  OK  ] SD card detected: SDHC (8192 MB)
[ INFO ] First boot detected - initializing filesystem structure
[  OK  ] Default filesystem structure created
[  OK  ] Root filesystem mounted from SD card (read/write)
[ INFO ] Registered 8 built-in commands
espnix:/root#
```

#### Without SD Card

If no SD card is detected, Espnix will boot in initramfs mode with clear warnings:

```
[ERROR] SD card initialization failed!
[ WARN ] No SD card detected or card is not properly inserted
[ INFO ] Continuing with initramfs only (no persistence)
[ WARN ] All changes will be lost on reboot!
espnix:/root#
```

## Available Native Commands

### File & Directory Operations
* `ls` – List directory contents
* `cd` – Change current directory
* `mkdir` – Create new directory
* `cat` – Display file contents
* `echo` – Display text or write to files

### System Commands
* `clear` – Clear terminal screen

### Development & Execution
* `compile` – Compile source code (.es) to bytecode (.enix)
* `run` – Execute compiled bytecode files (.enix)

### Network Management
* `iwctl` – Manage wireless connections

## File System Structure

On first boot with an SD card inserted, Espnix automatically creates a complete Unix-like directory structure:

```
/
├── bin/          System binaries and executables
├── boot/         Boot configuration files
├── dev/          Device files
├── etc/          System configuration files
│   └── motd      Message of the day
├── home/         User home directories
├── lib/          System libraries
├── mnt/          Mount points for external storage
├── opt/          Optional software packages
├── proc/         Process and system information
├── root/         Root user home directory
│   └── README.txt  Welcome message and command reference
├── run/          Runtime data and process information
├── sbin/         System administration binaries
├── srv/          Service data
├── sys/          System information and kernel interface
│   └── .init     Initialization marker (do not delete)
├── tmp/          Temporary files
├── usr/          User programs and data
│   ├── bin/      User executables
│   ├── lib/      User libraries
│   └── local/    Locally installed software
└── var/          Variable data files
    ├── log/      Log files
    └── tmp/      Temporary files preserved between reboots
```

### Important Files

* **/sys/.init**: Marker file created on first boot. Indicates the filesystem has been initialized. Do not delete this file unless you want to reinitialize the entire system.

* **/root/README.txt**: Welcome message with basic system information and command reference.

* **/etc/motd**: Message of the day displayed at login.

### Storage Modes

**SD Card Mode (Persistent)**
* All files are stored on the SD card
* Changes persist across reboots
* Full filesystem structure available
* Recommended for normal operation

**Initramfs Mode (Temporary)**
* Used when no SD card is detected
* Files stored in RAM only
* All changes lost on reboot
* Limited directory structure
* Fallback mode for troubleshooting

## Espnix Scripting Language

Espnix includes a built-in compiler and virtual machine for executing custom scripts. Write programs in Espnix Script (.es files), compile them to bytecode (.enix files), and execute them on the system.

### Language Features

* **Variables**: Declare and use variables with `var` keyword
* **Arithmetic**: +, -, *, /, % operators
* **Comparisons**: ==, !=, <, <=, >, >= operators
* **Logical**: and/&&, or/||, not/! operators
* **Control Flow**: if/else statements
* **Loops**: while loops
* **Output**: print() function
* **Comments**: Single-line comments with //

### Example Program

Create a file called `hello.es`:

```javascript
// Simple Espnix Script
var x;
var y;

x = 10;
y = 20;

var sum;
sum = x + y;

print(sum);

if (sum > 25) {
    print(1);  // true
} else {
    print(0);  // false
}

// Loop example
var counter;
counter = 0;
while (counter < 5) {
    print(counter);
    counter = counter + 1;
}
```

### Compile and Run

```bash
espnix:/root# compile hello.es
Compiling hello.es...
Lexical analysis complete (45 tokens)
Compilation complete (128 bytes)
Output written to hello.enix
Compilation successful!

espnix:/root# run hello.enix
Loading hello.enix...
Executing (128 bytes)...
--- Output ---
30
1
0
1
2
3
4
--- Execution complete ---
```

### Compiler Commands

* `compile <source.es> [output.enix]` – Compile source code to bytecode
* `run <program.enix>` – Execute compiled bytecode

The compiled .enix files are portable and can be distributed and executed on any Espnix system.

## WiFi Management

Connect to a WiFi network using the iwctl command:

```
espnix:/# iwctl device on
WiFi device turned on
espnix:/# iwctl station scan
Scanning for networks...
espnix:/# iwctl station get-networks
Networks available:
Name                  Security  Signal
Home-Network          psk       ****
espnix:/# iwctl station connect Home-Network mypassword
```

## Creating Native Binaries

For advanced use cases, you can create native binaries that interface directly with the Espnix system. Native binaries follow this format:

```c
#include "espnix.h"

int main(void* context) {
    ExecutionContext* ctx = (ExecutionContext*)context;

    std::string message = "Hello from SD card binary!";
    ctx->loader->ExecuteSysCall(SYS_WRITE, &message);

    return 0;
}
```

**Note**: For most use cases, the Espnix scripting language (.es files compiled to .enix bytecode) is recommended as it is easier to use and more portable.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the project
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the Apache License 2.0 - see the LICENSE file for details.

## Acknowledgments

* The Unix and Linux community for inspiration
* The Arduino and ESP32 community for their excellent libraries
* All contributors who have helped shape this project
