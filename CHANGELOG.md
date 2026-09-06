# Changelog

All notable changes to the VNET client and server.

---

## [9.5.0] - 2026-09-05

### 🎉 Major Release: "Cold Signal" Update

This release marks a complete transformation of VNET from a 2D terminal game into a full desktop environment with proper window management, workspace support, and a GNOME-style interface.

---

### 🚀 Added

#### Desktop Environment (GNOME-Style)

- **Complete desktop mode** with `F12` toggle between desktop and fullscreen browser UI
- **GNOME-style top bar** with Activities button, workspace indicator, clock, and VCOIN display
- **Desktop icons** for launching apps: Browser, Terminal, Profile, Settings, System Feed
- **Application grid** (Super key / Windows key) showing all available apps
- **Workspace management** with `Ctrl+1`, `Ctrl+2`, `Ctrl+3` to switch between Main, Work, and Chat workspaces

#### Window Management

- **Draggable windows** with title bars
- **Window controls**: Close (✕), Minimize (─), Maximize (□)
- **Z-index ordering**: Clicking a window brings it to the front
- **Window content focus**: Clicking any part of a window (title bar or content) brings it to top

#### Desktop Apps

- **VNET Browser** – Full VNET page rendering inside a desktop window
- **Terminal** – CLI overlay with scrolling and command input
- **User Profile** – Display player stats (handle, port, VCOIN, trace, ICE, sites)
- **Settings** – Theme, audio, display, security, and system configuration panels
- **System Feed** – Real-time network activity monitor

#### Network & Multiplayer

- **UDP socket system** with `VNetLib` abstraction (Windows/Linux compatible)
- **Client-server architecture** with headless server (`vnet_server.exe`)
- **KEY_SYNC protocol** – Server assigns 20 sites (9 mutual + 11 random) to each client
- **Heartbeat system** (`PING`) for peer registration (3s interval)
- **Network commands**: `CHAT`, `WHISPER`, `DOS`, `SPIKE`, `OVERLOAD`, `REDIRECT`, `SCAN`, `SATSCAN`, `SNIFFER`, `PATCH`, `DECOY`, `PROXY`, `ION`

#### Commands (40+ CLI Commands)

- **Victory**: `win <k1>...<k8>` (root breach), `takeover` (economic control)
- **Navigation**: `connect`, `goto`, `home`, `back`, `bounty`
- **Communication**: `chat`, `whisper`, `pm`, `say`
- **Economy**: `mine`, `wallet`, `buy ice`, `vektrapay`
- **Exploits**: `dos`, `spike`, `overload`, `redirect`, `snoop`, `probe`, `crack`, `ion`
- **Cyberwarfare**: `sniffer`, `satscan`, `patch`, `decoy`, `proxy`, `netscan`
- **Utility**: `scan`, `history`, `clear`, `theme`, `status`, `info`, `trace`, `flush`, `calm`, `meds`, `inspect`
- **VFS/Decryption**: `shift`, `vdec`, `cat`, `ls`

#### UI Enhancements

- **Enhanced jitter/shake system** with multi-frequency noise, CRT wobble, and glitch spikes
- **CRT distortion effects**: vignette, scanline interference, chromatic aberration
- **VCR_OSD_MONO font** support with global font scaling
- **Fullscreen toggle** with `F11`
- **FPS overlay** toggle with `F1`
- **Debug overlay** toggle with `F2`

#### Build System

- **Recursive source discovery** in build script
- **Color-coded build output** with progress bars
- **Client and server build scripts** (`build_client.ps1`, `build_server.ps1`)
- **CMake support** for Visual Studio and MinGW
- **PowerShell build script** with dependency checking

---

### 🐛 Fixed

- **Duplicate `main.cpp` compilation** – Added unique filter to source discovery
- **Desktop.cpp path** – Moved from `src/client/` to `src/client/desktop/`
- **Desktop icons not clickable** – Moved click detection from `Draw()` to `Update()`
- **Desktop icons not visible** – Added background color to icons
- **Window controls not working** – Fixed button detection and click handling
- **Z-index / click-through issues** – Windows now properly stack and bring to front
- **Scrolling in desktop mode** – Mouse wheel now works in both browser and terminal
- **Terminal input in desktop mode** – Terminal window now accepts keyboard input
- **Network `[NET]` spam** – Fixed packet parsing to ignore IP/port metadata
- **`StartsWith` function conflict** – Consolidated to single definition
- **`fmodf` and math functions** – Added `#include <cmath>` to desktop.cpp
- **Multi-character character constant** – Fixed Unicode bullet in workspace indicator
- **`TriggerJitter` overload conflict** – Removed duplicate default argument
- **`SX`/`SY` declaration conflict** – Removed `static` from implementation
- **Missing `#include <string>`** – Added to `render.h`

---

### 🔧 Changed

- **Project structure** – Reorganized into `client/`, `server/`, `shared/`, `lib/` folders
- **Desktop.cpp split** – Separated into modular files for apps, settings, and VDEC
- **Build script** – Updated to use recursive source discovery for all `.cpp` files
- **UI scaling** – Added `g_fontScale` and global UI scaling with `Ctrl++`/`Ctrl+-`
- **Jitter system** – Upgraded from simple sine wave to multi-dimensional noise with glitch spikes

---

### 📁 Project Structure

```

src/
├── client/
│ ├── desktop/
│ │ ├── apps/ # Browser, Terminal, Profile, Settings, Feed, Hellroom, VDEC
│ │ ├── settings/ # Theme, Audio, Display, Security, System
│ │ └── desktop.cpp/h
│ ├── game.cpp/h
│ ├── render.cpp/h
│ ├── player.cpp/h
│ └── vnet_client.cpp/h
├── server/
│ ├── main.cpp # Headless entry
│ └── server_core.cpp # Server game loop
├── shared/
│ ├── vnet.cpp/h # Shared game logic
│ ├── vnet_protocol.h # Protocol definitions
│ ├── vnet_sites.cpp/h # 50+ site data
│ └── utils.cpp/h
└── lib/
└── vnet_lib.cpp/h # Low-level UDP socket wrapper

```

---

### 🎮 Controls

| Key               | Action                      |
| ----------------- | --------------------------- |
| `F1`              | Toggle FPS Display          |
| `F2`              | Toggle Debug Info           |
| `F10`             | Toggle Desktop Mode         |
| `F11`             | Toggle Fullscreen           |
| `F12`             | Toggle Desktop Mode (alias) |
| `TAB`             | Toggle Terminal Overlay     |
| `Ctrl+1/2/3`      | Switch Workspaces           |
| `Super` (Win key) | Open App Grid               |
| `Ctrl++`          | Increase UI Scale           |
| `Ctrl+-`          | Decrease UI Scale           |

---

### 🏆 Victory Conditions

| Path                  | Requirement                                                           |
| --------------------- | --------------------------------------------------------------------- |
| **Root Breach**       | Collect 8 keys → `win <k1>...<k8>` at terminal.vnet                   |
| **Grid Blackout**     | Overload all 5 core nodes (market, vault, terminal, crypto, hellroom) |
| **Economic Takeover** | Reach 25.0 VCOIN → `takeover`                                         |

---

### 📦 Assets

- **Font**: VCR_OSD_MONO_1.001.ttf
- **Icons**: Emoji-based desktop icons (Browser 🌐, Terminal 💻, Profile 👤, Settings ⚙, Feed 📊)
- **Audio**: Music player with visualizer (VAudio integration)

---

### 🚀 Known Issues

- Hellroom IRC messages may duplicate locally before server broadcast
- Scan command uses offline generation; server response path needs debugging
- Some UI tags (`[INPUT]`, `[BTN]`, `[SCANNER]`) not fully implemented in markup renderer

---

### 🙏 Contributors

- Project Lead: @sh4d0w_net
- Network Architecture: VNET Development Team
- UI/UX Design: VEKTRAOS Design Bureau

---

### 🕯️

> _"The network is alive. It is drinking your heat."_

---
