# svarstore

Simple Variable Store - A command-line tool for managing EFI variables in a simple variable store format.

**Only for specified Radxa devices**

## Overview

`svarstore` is a lightweight and efficient EFI variable store management tool that provides comprehensive functionality for handling EFI variables. It supports listing, modifying, and managing EFI variables, as well as boot device order configuration.

## Features

- **Variable Management**
  - List all EFI variables in the store
  - Get and set individual EFI variables
  - Flush changes to the EFI variable store
  - Reload EFI variables from the store
  - Erase all EFI variables in the store

- **Boot Order Management**
  - List current boot device order
  - Set new boot device order
  - Replace individual boot devices at specific positions

- **Daemon Mode**
  - Monitor and sync EFI variables automatically
  - Background operation support

- **Storage Format**
  - Custom simple variable store format
  - Signature: `SVARSTOR` (0x524f545352415653)
  - Version: 0x00010000
  - Minimum store size: 8KB (0x2000 bytes)

## Installation

### Build from Source

#### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential meson ninja-build libc6-dev debhelper
```

**Arch Linux:**
```bash
sudo pacman -S base-devel meson ninja
```

#### Building

```bash
# Clone the repository
git clone https://github.com/BigfootACA/svarstore.git
cd svarstore

# Configure build
meson setup build

# Compile
meson compile -C build

# Install (optional)
sudo meson install -C build
```

### Install from Debian Package

```bash
# Build Debian package
dpkg-buildpackage -us -uc -b

# Install the package
sudo dpkg -i ../svarstore_0.1_*.deb
```

### Install from Arch Linux Package

```bash
# Build Arch Linux package
makepkg -si

# Install the package
sudo pacman -U svarstore-0.1-*.pkg.tar.zst
```

## Usage

### Basic Commands

```bash
# List all EFI variables
svarstore --list

# Flush changes to store
svarstore --flush

# Reload variables from store
svarstore --reload

# Erase all variables
svarstore --erase

# Start daemon mode
svarstore --daemon
```

### Boot Order Management

```bash
# List current boot order
svarstore --order-list

# Set new boot order (comma-separated)
svarstore --order-set usb,emmc,ufs,nvme

# Replace device at position 1 with eMMC
svarstore --order-replace 1=emmc

# Replace device at position 3 with UFS
svarstore --order-replace 3=hdd
```

### Available Boot Device Types

- `CDROM`
- `eMMC`
- `Floppy`
- `IDE`
- `Net`
- `NVMe`
- `SATA`
- `SCSI`
- `SD`
- `SPINOR`
- `UFS`
- `USB`

## License

This project is licensed under the GNU General Public License v2.0 or later (GPL-2+).

See the [LICENSE](LICENSE) file for details.
