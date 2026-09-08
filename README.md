# 🖥️ VNET - Cyberwarfare Engine

**A Dark, Immersive P2P Network Simulation & Cyberpunk RPG**

[![Version](https://img.shields.io/badge/version-9.5.0-red.svg)](https://github.com/yourusername/vnet)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

---

## 📡 Overview

**VNET** is a fully-featured cyberpunk network simulation and multiplayer strategy game where you navigate a dark, interconnected web of nodes, exploit vulnerabilities, mine cryptocurrency, and compete to achieve dominance over the **VNET backbone**. Built from scratch with **C++** and **Raylib**, it combines real-time networking, immersive UI/UX, and deep RPG mechanics.

**🔗 Official Server:** [vnet://hellroom.vnet]  
**💀 Core Objective:** Overload the 5 mutual core nodes, breach the root vault with 8 cryptographic keys, or amass 25.0 VCOIN for economic control.

---

## 🎮 Gameplay & Mechanics

### 🏆 **Victory Conditions**

Three distinct paths to win:

| **Path**              | **Requirement**                                                                                         | **Reward**              |
| --------------------- | ------------------------------------------------------------------------------------------------------- | ----------------------- |
| **Grid Blackout**     | Overload all 5 core nodes: `market.vnet`, `vault.vnet`, `terminal.vnet`, `crypto.vnet`, `hellroom.vnet` | `🔓 SYSTEM OVERRIDE`    |
| **Root Breach**       | Collect all 8 cryptographic keys and execute `win <k1> ... <k8>` at `terminal.vnet`                     | `🔑 VAULT ACCESS`       |
| **Economic Takeover** | Reach 25.0 VCOIN and type `takeover`                                                                    | `💰 ECONOMIC DOMINANCE` |

### 🧠 **Core Systems**

#### **1. Cryptographic Key System**

- **8 Master Keys** scattered across the network
- Each key is hidden behind a unique challenge at a specific `.vnet` node
- Use **VDEC (VNET Decryption Toolkit)** to decrypt, hash, and brute-force keys
- Keys are synchronized with the server via **KEY_SYNC** protocol

#### **2. VDEC – VNET Decryption Toolkit**

- **Key Ring** – Track 8 cryptographic keys with visual progress
- **Encrypt / Decrypt** – Caesar cipher operations with server-side engine
- **Hash Calculator** – MD5‑style cryptographic digests
- **Decryption Minigame** – Challenge-based key unlocking
- **Bit‑shift Offset Control** – Fine‑tune decryption parameters
- **Immersive Cyberpunk UI** – Scanlines, glows, and matrix‑style animations

#### **3. Mining & Economy**

- Mine blocks at `crypto.vnet` using the `mine <block_id>` command
- Each block yields **0.20–0.60 VCOIN** with a **20-second cooldown**
- Mining increases **trace threat** by +18% and **CRT heat** by +3.5°C
- Spend VCOIN on:
  - **ICE Shields** (0.30 VCOIN) – Absorb DOS/Trace attacks
  - **Trace Flush** (0.10 VCOIN) – Reduce trace by 30%
  - **Neural Calm** (0.15 VCOIN) – Reduce paranoia
  - **Exploits** – DOS, Spike, Redirect, Overload, Ion Cannon

#### **4. Exploit & Cyberwarfare Kit**

| Command                 | Cost       | Cooldown | Effect                             |
| ----------------------- | ---------- | -------- | ---------------------------------- |
| `dos <port>`            | 0.25 VCOIN | 15s      | Freezes target peer for 8s         |
| `spike <port>`          | 0.20 VCOIN | 12s      | +35% trace on target               |
| `overload <url>`        | 1.50 VCOIN | 25s      | Overloads a site for 45s           |
| `redirect <port> <url>` | 0.15 VCOIN | 10s      | BGP hijack target                  |
| `ion <target>`          | 2.00 VCOIN | 180s     | Orbital strike (at `orbital.vnet`) |
| `proxy <url> <node>`    | 0.40 VCOIN | 120s     | Route through proxy                |

#### **5. Federal E‑Raid System**

- **Cinematic Sequence** – Glitch → Flash → Hex Flood → Blackout → Active Raid
- **3D Operator Face** – Wireframe head with dynamic dialogue
- **Desktop Lockdown** – Background black, only terminal and intruder detector remain
- **Minigame Options**:
  - **EVADE** – Deploy decoy nodes to misdirect federal scanners
  - **ESCAPE** – Migrate to a new port before trace locks
  - **BURN** – Scorched earth: purge logs and kill processes
- **Consequences** – Trace spike, ICE loss, VCOIN confiscation, site burning
- **Rewards** – VCOIN bonuses, trace reduction, ICE restoration

#### **6. Threat & Paranoia System**

- **Trace Level (0–100%):** Increases with every action. High trace attracts **federal e-raids**, **bot stalkers**, and **ICE lockouts**.
- **CRT Heat (35–100°C):** Overheating causes glitching and system instability.
- **Neural Paranoia (0–100%):** Accumulates from exposure to horror nodes. High paranoia affects UI perception and triggers **Netman hallucinations**.
- **ICE Shields (0–3):** Absorb DOS, Spike, and Redirect attacks. Purchase at `market.vnet`.

---

## 🖥️ Desktop Environment

A fully functional **VEKTRA OS‑style desktop** built from scratch:

### 🪟 **Window Management**

- **Draggable, resizable windows** with macOS‑style title bars
- **Red / Yellow / Green window controls** (close, minimize, maximize)
- **Workspace switching** (Ctrl+1/2/3)
- **Application grid** (Super key or click "Activities")
- **Z‑index ordering** – click to bring windows to front

### 🔐 **Login & Loading Experience**

- **VEKTRA OS login screen** – replaces the old connection menu
- **Animated grid background** with floating nodes and connection lines
- **Glowing VEKTRA logo** with pulse effect
- **Cyberpunk‑styled IP and handle inputs**
- **System status panel** – network, encryption, uptime, port
- **CRT scanline and vignette overlays**
- **Loading screen** with rotating hexagon gear, progress bar, and dynamic status messages

### 📱 **Built-in Applications**

| App                 | Description                                                                                                 |
| ------------------- | ----------------------------------------------------------------------------------------------------------- |
| **🌐 Browser**      | VNET page renderer with multi‑tab support, bookmarks, history, and security indicators                      |
| **💻 Terminal**     | Full-featured CLI with scrolling, command history, and syntax highlighting                                  |
| **👤 Profile**      | Player stats, site directory, VCOIN wallet, ICE/trace status                                                |
| **⚙️ Settings**     | Theming (15+ themes), audio control, display options, security                                              |
| **📡 Feed**         | Real-time system feed with threat radar and activity ticker                                                 |
| **🔥 Hellroom**     | P2P IRC chatroom with nick changes, whispers, commands (`/nick`, `/me`, `/w`, `/clear`, `/help`, `/status`) |
| **🔐 VDEC**         | Decryption toolkit with Key Ring, Encrypt/Decrypt, Hash, and Minigame                                       |
| **🎵 Music Player** | VEKTRA OS‑styled player with visualizer, playlist, and CRT effects                                          |

### 🎨 **Themes (15)**

```
classic • tokyo • redroom • amber • matrix • cyberpunk • nord
dracula • synthwave • cobalt • monokai • gruvbox • abyss • solaris • ghost
```

---

## 🌐 Network Architecture

### **Client-Server Model**

- **UDP protocol** with custom packet framing
- **Non-blocking I/O** for smooth real-time interaction
- **P2P broadcast** for chat and exploit propagation
- **Heartbeat system** (3s interval) for peer registry

### **Protocol Commands (VNetCmd)**

| Command         | Purpose                       |
| --------------- | ----------------------------- |
| `PING`          | Peer registration & heartbeat |
| `CHAT`          | Global broadcast message      |
| `WHISPER`       | Private direct message        |
| `GET`           | Page request                  |
| `DOS`           | Denial-of-service attack      |
| `SPIKE`         | Trace spike injection         |
| `OVERLOAD`      | Site overloading              |
| `WIN`           | Key submission                |
| `TAKEOVER`      | Economic victory              |
| `SCAN`          | Site discovery                |
| `SATSCAN`       | Satellite surveillance        |
| `NETSCAN`       | Peer discovery                |
| `VDEC_DECRYPT`  | Decryption operation          |
| `VDEC_ENCRYPT`  | Encryption operation          |
| `VDEC_HASH`     | Hash calculation              |
| `VDEC_MINIGAME` | Decryption challenge          |

### **Server Features**

- **Dynamic site assignment** (9 mutual + 11 random per player)
- **Key generation** with scrambling & location mapping
- **Mining block rotation** (8 new blocks every 30s)
- **Decoy system** with tripwire traps
- **Federal e-raids** with cinematic sequence
- **Bot stalker** (every 120s)
- **Site overloading** with 45s cooldown
- **VDEC engine** – server-side encryption, decryption, and hashing

---

## 🖌️ UI/UX Design

### **Markup Language**

VNET uses a custom **tag-based markup language** for page rendering:

```markdown
[TITLE] Page Title
[SUBTITLE] Section Header
[BADGE:text:color] Visual Badge
[GAUGE:75:Label] Progress Bar
[LINK:url] >> Link Text
[BOX] +-- Box Content --+
[CODE] Monospace code
[HR] Horizontal Rule
[ART:key] ASCII Art
[IMG:key] Image Placeholder
[VIDEO:key:title] Video Player
[HEX_STREAM:addr:len:rate] Hex Dump
[SCANNER:var:alias] Retinal Scanner
[INPUT:id:placeholder] Text Input
[BTN:action:label] Interactive Button
```

### **Visual Effects**

- **CRT jitter/shake** system with weighted multi-frequency noise
- **Scanline overlay** with subtle CRT wobble
- **Glitch spikes** for network events
- **Dynamic color palettes** per theme
- **Animated backgrounds** with floating grid nodes
- **CRT distortion effects** – vignette, chromatic aberration, scanline interference

### **Responsive Scaling**

- **Letterbox scaling** with reference resolution (1920x1080)
- **DPI-aware rendering** for all UI elements
- **Scaled primitives** (rect, text, lines) for consistent UX

---

## 🛠️ Technical Stack

### **Core Libraries**

| Library                      | Purpose                      |
| ---------------------------- | ---------------------------- |
| **Raylib**                   | Rendering, input, audio      |
| **Winsock2 / POSIX Sockets** | Networking                   |
| **STL**                      | Containers, strings, vectors |

### **Build Requirements**

- **C++17** or higher
- **Raylib 5.0+** (with audio support)
- **CMake 3.10+** (optional)

### **Directory Structure**

```
src/
├── client/
│   ├── connection/          # VEKTRA OS login screen
│   ├── desktop/
│   │   ├── apps/
│   │   │   ├── browser.cpp/h
│   │   │   ├── feed.cpp
│   │   │   ├── hellroom.cpp
│   │   │   ├── profile.cpp
│   │   │   ├── settings.cpp
│   │   │   ├── terminal.cpp
│   │   │   └── vdec/         # VDEC modules (6 files)
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
│   ├── vnet_sites.cpp/h
│   ├── vnet_protocol.h
│   ├── utils.cpp/h
│   └── vex_parser.cpp/h      # .vex file parser
└── lib/
    └── vnet_lib.cpp/h
```

---

## 🚀 Getting Started

### **1. Clone & Build**

```bash
git clone https://github.com/yourusername/vnet.git
cd vnet
mkdir build && cd build
cmake .. && make
```

### **2. Run the Server**

```bash
./bin/vnet_server 8000
```

### **3. Run the Client**

```bash
./bin/vnet_client
```

- Enter server IP (default: `127.0.0.1`)
- Press **Enter** or click **"INITIALIZE UPLINK"**

### **4. First Steps**

- Press **TAB** to open the terminal overlay
- Type `scan` to discover new sites
- Type `help` for full command reference
- Press **Super key** (Windows/Command) for the app grid

### **5. Quickstart Commands**

```bash
# Navigation
connect <url>     # Navigate to a site
home              # Return to vnet.dir
back              # Previous page

# Chat
chat <message>    # Global broadcast
whisper <h> <msg> # Private message

# Mining
mine              # Show available blocks
mine <block_id>   # Mine a specific block

# Exploits
dos <port>        # DOS attack
spike <port>      # Trace spike
overload <url>    # Overload site

# VDEC
vdec decrypt      # Open decryption toolkit
vdec encrypt      # Open encryption toolkit
vdec hash         # Open hash calculator

# System
status            # Show network status
info              # Detailed system info
trace             # Show trace level
flush             # Reduce trace (0.10 VCOIN)
theme <name>      # Change color theme
```

---

## 🧬 Game World: 55 VNET Sites

### **Core Nodes (5)**

| Site            | Description                     |
| --------------- | ------------------------------- |
| `market.vnet`   | Black market & ICE vendor       |
| `vault.vnet`    | Corrupted data vault / VFS root |
| `terminal.vnet` | Master decryption gateway       |
| `crypto.vnet`   | Mining rig & black tumbler      |
| `hellroom.vnet` | P2P chat hub                    |

### **Horror / Lore (12)**

`redroom` • `dollhouse` • `morgue` • `snuff` • `asylum` • `cult` • `skinwalker` • `corridor204863` • `ghost` • `schizo` • `necro` • `void`

### **Black Market (8)**

`silkroad` • `zeroauction` • `blackbank` • `weaponry` • `passports` • `darkdrop` • `vektrapay` • `bounty`

### **Infrastructure (10)**

`watchtower` • `orbital` • `cctv-core` • `eye` • `substation04` • `stasi` • `deadchannel` • `signal0` • `deepocean` • `norilsk-relay`

### **Network Hubs (15)**

`forum` • `deepwiki` • `pastebin` • `whisper` • `dump` • `index` • `project9` • `echolab` • `phantom` • `glitch` • `stasis` • `entropy` • `hive` • `nexus`

### **Special Nodes (5)**

`hashbeat` • `lifeleaks` • `luna` • `subcell` • `feed99`

---

## 🎨 Screenshots

| Desktop                                    | Browser                                    | Terminal                                     |
| ------------------------------------------ | ------------------------------------------ | -------------------------------------------- |
| ![Desktop](assets/screenshots/desktop.png) | ![Browser](assets/screenshots/browser.png) | ![Terminal](assets/screenshots/terminal.png) |

| Profile                                    | Feed                                 | Hellroom                                     |
| ------------------------------------------ | ------------------------------------ | -------------------------------------------- |
| ![Profile](assets/screenshots/profile.png) | ![Feed](assets/screenshots/feed.png) | ![Hellroom](assets/screenshots/hellroom.png) |

| Settings                                     | VDEC                                 | Connection                                       |
| -------------------------------------------- | ------------------------------------ | ------------------------------------------------ |
| ![Settings](assets/screenshots/settings.png) | ![VDEC](assets/screenshots/vdec.png) | ![Connection](assets/screenshots/connection.png) |

---

## 🔧 Configuration

### **Themes**

Set active theme via CLI: `theme <name>`
List all themes: `theme list`

### **Display**

- Resolution scaling: automatic letterbox
- Fullscreen toggle: **F11**
- FPS display: **F1**
- Debug overlay: **F2**

### **Audio**

- Volume control in **Settings > Audio**
- Built-in **Music Player** widget with:
  - Play/Pause, Next/Previous
  - Shuffle & Repeat modes
  - Visualizer & progress bar
  - Track list

---

## 🧪 Development

### **Adding a New Site (.vex Format)**

1. Create a `.vex` file in `assets/sites/`:

```markdown
[TITLE] My Site Title
[CATEGORY] core
[ID] mysite.vnet
[MAP_X] 0
[MAP_Y] 0
[HAS_KEY] false
[HACK_DIFFICULTY] 1.0

[CONTENT]
[TITLE] My Site // NODE
[HR]
[BADGE:TAG:BLOOD]
[TEXT] Welcome to my site!
[LINK:vnet.dir] >> Return to Directory
[/CONTENT]
```

2. The parser automatically loads all `.vex` files at startup
3. Placeholders like `ICE_COUNT`, `VCOIN_BALANCE`, `TRACE_LEVEL`, `HANDLE_NAME`, `PORT_NUMBER` are replaced dynamically

### **Adding a New Command**

1. Define command in `vnet_protocol.h`
2. Add parsing in `server_core.cpp` (`RunVNTServer` loop)
3. Add client handling in `vnet.cpp` (`UpdateVNET`)
4. Add CLI logic in `vnet.cpp` (`ProcessCommand`)

### **Creating a New Theme**

1. Add `ThemeDef` entry in `render.cpp`
2. Include theme name in `theme list` output
3. Set via CLI: `theme <name>`

---

## 🤝 Contributing

We welcome contributions! Here's how:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### **Guidelines**

- Follow **C++17** standards
- Use **Raylib** for all rendering/input
- Maintain **consistent styling** (see existing code)
- **Document** new features in the README
- **Test** thoroughly before submitting

---

## 📄 License

This project is licensed under the **MIT License** – see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **Raylib** – The incredible game development library
- **Vyne Script** – For the original concept and design inspiration
- **The VNET Community** – For testing, feedback, and dark lore contributions

---

## 📬 Contact

- **Project Lead:** [@t2ncay](https://github.com/t2ncay)
- **Discord:** [VNET Community](https://discord.gg/vnet)
- **Email:** [vnet@project.com](mailto:4tvnex@gmail.com)

---

## 🕯️ "The network is alive. It is drinking your heat."

_Welcome to the VNET. There's no turning back._

---

**Version 9.5.0** – _"Cold Signal" Update_

[![Made with C++](https://img.shields.io/badge/Made%20with-C%2B%2B-red.svg)](https://isocpp.org/)
[![Powered by Raylib](https://img.shields.io/badge/Powered%20by-Raylib-blue.svg)](https://www.raylib.com/)
