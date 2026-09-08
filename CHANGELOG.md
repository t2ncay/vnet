# Changelog

All notable changes to the VNET client and server.

---

## [Unreleased]

### Added

- **VDEC (VNET Decryption Toolkit)** – full cryptographic suite
  - Key Ring with 8 key slots and progress tracking
  - Caesar cipher encrypt/decrypt with server integration
  - Hash calculator (MD5‑style)
  - Decryption minigame to unlock keys
  - Immersive cyberpunk UI with scanlines, glows and animations
- **Hellroom IRC app** – full P2P chat client
  - Nickname changer
  - Message input with send button
  - User list with online status
  - Chat history with timestamps
  - Commands: `/nick`, `/me`, `/w`, `/clear`, `/help`, `/status`
  - Scrollable chat log with colour‑coded messages
- **VEKTRA OS login screen** – replaces old connection menu
  - Animated grid background with floating nodes
  - Glowing VEKTRA logo with pulse effect
  - Cyberpunk‑styled IP and handle inputs
  - System status panel (network, encryption, uptime, port)
  - CRT scanline and vignette overlays
  - Loading screen with rotating hexagon gear and progress bar
- **macOS‑style window controls**
  - Red / Yellow / Green circles (close, minimize, maximize)
  - Left‑aligned (configurable order)
- **Enhanced wallpaper** – VEKTRA intelligence agency style
  - Detailed eagle silhouette
  - Crisp, scaled VEKTRA text with glow
  - Ambient floating particles
  - Subtle vignette and scanline effects
- **Redesigned music player** – VEKTRA OS aesthetic
  - CRT scanlines and corner reticles
  - Gradient visualizer with glow peaks
  - Track list with current track highlight
  - Glowing progress bar with handle
  - VEKTRA OS branding and status indicator
- **.vex site file system** – all 54 sites now external, moddable `.vex` files
  - Full VEX parser with metadata and content blocks
  - Dynamic placeholder replacement (`ICE_COUNT`, `VCOIN_BALANCE`, etc.)
  - Support for `[ID]` tag to match URLs
- **Federal E‑Raid cinematic sequence**
  - Glitch → Flash → Hex flood → Blackout → Active raid
  - 3D operator face (wireframe head) with dialogue
  - Desktop lockdown, terminal forced open
  - Success/failure outcomes with visual feedback
  - Integrated minigame: EVADE, ESCAPE, BURN
- **Enhanced browser** – full redesign
  - Multi‑tab browsing with favicons
  - Bookmarks bar with quick links
  - History sidebar
  - Security indicator (🔒 / ⚠️)
  - Status bar with real‑time stats
  - Loading overlay with progress bar
  - Keyboard shortcuts (`Ctrl+T`, `Ctrl+W`, etc.)
- **VEKTRA OS‑style connection overlay** for browser navigation
- **Federal raid commands**: `raid`, `raid 1|2|3`, `decoy`, `patch`, `purge`

### Changed

- **Desktop split** – monolithic `desktop.cpp` (3000+ lines) refactored into 20+ modular files
  - Apps: `browser/`, `terminal/`, `profile/`, `settings/`, `feed/`, `hellroom/`, `vdec/`
  - Settings: `theme/`, `audio/`, `display/`, `security/`, `system/`
- **Login screen** – replaced old connection menu with VEKTRA OS interface
- **Music player** – redesigned to match VEKTRA OS wallpaper aesthetic
- **Wallpaper** – improved resolution and detail
- **Build system** – recursive source discovery, duplicate removal, colour output
- **Browser** – now self‑contained class, draws itself via `GetBrowser().Draw(win)`
- **Font scaling** – global `g_fontScale` and UI scaling for better readability
- **Windows** – now use macOS‑style circles, title centred (or left‑aligned, configurable)

### Fixed

- Duplicate `main.cpp` compilation (added `Select-Object -Unique`)
- Desktop icons not clickable/visible
- Window controls not responding to clicks
- Z‑index ordering and click‑through issues
- Scrolling in desktop mode (both terminal and browser)
- Terminal input in desktop mode
- Hellroom messages not being sent/received (now uses system feed)
- Local message duplication in Hellroom (removed redundant local push)
- `DrawBrowserConnectionOverlay` undefined (moved to Desktop public)
- `DrawVDEC*` declarations missing in `desktop.h`
- Compilation warnings: multi‑character constants, narrowing conversions, unused variables
- Missing includes: `<cstring>`, `<cmath>`, `vnet_protocol.h`

### Removed

- Old connection menu (replaced by VEKTRA OS login screen)
- Old square window buttons (✕, −, □) – replaced by macOS circles
- Old browser implementation (replaced by `Browser` class)

---

## [9.5.0] – 2026‑09‑05

### Added – Initial Feature Set

#### Desktop Environment (GNOME‑Style)

- Complete desktop mode with `F12` toggle
- GNOME‑style top bar with Activities, workspace indicator, clock, VCOIN display
- Desktop icons for apps: Browser, Terminal, Profile, Settings, System Feed
- Application grid (Super key)
- Workspace management (`Ctrl+1/2/3`)

#### Window Management

- Draggable windows with title bars
- Window controls: close, minimise, maximise
- Z‑index ordering and focus on click

#### Desktop Apps

- **VNET Browser** – full site rendering inside a desktop window
- **Terminal** – CLI overlay with scrolling and command input
- **User Profile** – player stats display
- **Settings** – theme, audio, display, security, system panels
- **System Feed** – real‑time network activity monitor

#### Network & Multiplayer

- UDP socket system (`VNetLib`) – Windows/Linux compatible
- Client–server architecture with headless server (`vnet_server.exe`)
- `KEY_SYNC` protocol – server assigns 20 sites (9 mutual + 11 random)
- Heartbeat system (`PING`) for peer registration (3s interval)
- Network commands: `CHAT`, `WHISPER`, `DOS`, `SPIKE`, `OVERLOAD`, `REDIRECT`, `SCAN`, `SATSCAN`, `SNIFFER`, `PATCH`, `DECOY`, `PROXY`, `ION`

#### Commands (40+ CLI)

- **Victory**: `win <k1>...<k8>` (root breach), `takeover` (economic control)
- **Navigation**: `connect`, `goto`, `home`, `back`, `bounty`
- **Communication**: `chat`, `whisper`, `pm`, `say`
- **Economy**: `mine`, `wallet`, `buy ice`, `vektrapay`
- **Exploits**: `dos`, `spike`, `overload`, `redirect`, `snoop`, `probe`, `crack`, `ion`
- **Cyberwarfare**: `sniffer`, `satscan`, `patch`, `decoy`, `proxy`, `netscan`
- **Utility**: `scan`, `history`, `clear`, `theme`, `status`, `info`, `trace`, `flush`, `calm`, `meds`, `inspect`
- **VFS/Decryption**: `shift`, `vdec`, `cat`, `ls`

#### UI Enhancements

- Enhanced jitter/shake system with multi‑frequency noise, CRT wobble, glitch spikes
- CRT distortion effects: vignette, scanline interference, chromatic aberration
- VCR_OSD_MONO font support with global font scaling
- Fullscreen toggle (`F11`), FPS overlay (`F1`), debug overlay (`F2`)

#### Build System

- Recursive source discovery in build script
- Colour‑coded build output with progress bars
- Client and server build scripts (`build_client.ps1`, `build_server.ps1`)
- CMake support for Visual Studio and MinGW

### Changed – Initial Release

- Project structure reorganised into `client/`, `server/`, `shared/`, `lib/`
- Desktop.cpp split into modular files for apps, settings, and VDEC
- Build script updated for recursive source discovery
- UI scaling with `g_fontScale` and `Ctrl++`/`Ctrl+-`
- Jitter system upgraded from sine wave to multi‑dimensional noise

### Fixed – Initial Release

- Duplicate `main.cpp` compilation – added unique filter
- Desktop.cpp path moved to `desktop/` folder
- Desktop icons clickable and visible
- Window controls working
- Z‑index / click‑through issues resolved
- Scrolling in desktop mode (browser and terminal)
- Terminal input in desktop mode
- Network `[NET]` spam – fixed packet parsing
- `StartsWith` function conflict – consolidated
- `fmodf` and math functions – added `<cmath>`
- Multi‑character character constant – fixed Unicode bullet
- `TriggerJitter` overload conflict – removed duplicate default argument
- `SX`/`SY` declaration conflict – removed `static`
- Missing `#include <string>` – added to `render.h`

### Removed – Initial Release

- Old fullscreen browser UI (replaced by desktop mode)

---

## Project Structure

```
src/
├── client/
│   ├── connection/
│   │   └── login_screen.cpp/h
│   ├── desktop/
│   │   ├── apps/
│   │   │   ├── browser.cpp/h
│   │   │   ├── feed.cpp
│   │   │   ├── hellroom.cpp
│   │   │   ├── profile.cpp
│   │   │   ├── settings.cpp
│   │   │   ├── terminal.cpp
│   │   │   └── vdec/
│   │   │       ├── vdec.cpp
│   │   │       ├── keyring.cpp
│   │   │       ├── decrypt.cpp
│   │   │       ├── encrypt.cpp
│   │   │       ├── hash.cpp
│   │   │       └── minigame.cpp
│   │   ├── settings/
│   │   │   ├── theme.cpp
│   │   │   ├── audio.cpp
│   │   │   ├── display.cpp
│   │   │   ├── security.cpp
│   │   │   └── system.cpp
│   │   ├── desktop.cpp
│   │   ├── desktop.h
│   │   └── desktop_icons.cpp
│   ├── game.cpp/h
│   ├── main.cpp
│   ├── music_player.cpp/h
│   ├── player.cpp/h
│   ├── render.cpp/h
│   ├── vnet_client.cpp/h
│   └── wallpaper.cpp/h
├── server/
│   ├── main.cpp
│   └── server_core.cpp
├── shared/
│   ├── vnet.cpp/h
│   ├── vnet_protocol.h
│   ├── vnet_sites.cpp/h
│   ├── utils.cpp/h
│   └── vex_parser.cpp/h
└── lib/
    └── vnet_lib.cpp/h
```

---

## Keyboard Shortcuts

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

## Victory Conditions

| Path                  | Requirement                                                           |
| --------------------- | --------------------------------------------------------------------- |
| **Root Breach**       | Collect 8 keys → `win <k1>...<k8>` at terminal.vnet                   |
| **Grid Blackout**     | Overload all 5 core nodes (market, vault, terminal, crypto, hellroom) |
| **Economic Takeover** | Reach 25.0 VCOIN → `takeover`                                         |

---

## Known Issues

- Hellroom IRC messages may duplicate locally before server broadcast
- Scan command uses offline generation; server response path needs debugging
- Some UI tags (`[INPUT]`, `[BTN]`, `[SCANNER]`) not fully implemented in markup renderer

---

## Contributors

- Project Lead: @sh4d0w_net
- Network Architecture: VNET Development Team
- UI/UX Design: VEKTRAOS Design Bureau

---

> _"The network is alive. It is drinking your heat."_

---
