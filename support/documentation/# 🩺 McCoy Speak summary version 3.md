<p align="center">
  <img src="./media/mccoy-speaks-banner.png" alt="McCoy Speaks Project Banner" width="720"/>
</p>

---

<p align="center">
  <b>McCoy Speaks</b><br>
  <i>"I’m a doctor, not a microcontroller!"</i><br><br>
  <sub>A self-contained ATtiny85 audio playback project inspired by Dr. Leonard H. McCoy.</sub>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/PlatformIO-Built-blue?logo=platformio" alt="Built with PlatformIO"/>
  <img src="https://img.shields.io/badge/MCU-ATtiny85-orange" alt="ATtiny85 MCU"/>
  <img src="https://img.shields.io/badge/Power-LiPo_3.7V-yellow" alt="LiPo Power"/>
  <img src="https://img.shields.io/badge/Audio-DFPlayer_Mini-green" alt="DFPlayer Mini Audio"/>
  <img src="https://img.shields.io/badge/Tribute-Star_Trek-blueviolet" alt="Star Trek Tribute"/>
  <img src="https://img.shields.io/badge/License-Open_Hardware-lightgrey" alt="Open Hardware License"/>
</p>


---

## 🎯 What It Is

**McCoy Speaks** is a self-contained audio playback system that delivers classic *Dr. Leonard “Bones” McCoy* voice lines at the press of a single button.  

Built around an **ATtiny85 microcontroller**, a **DFPlayer Mini audio module**, and a **LiPo battery**, the project recreates the spirit of old-school Star Trek tech — practical, modular, and proudly over-engineered for the fun of it.

This project was designed for clarity and longevity: every wire, file, and firmware line is documented, versioned, and open for others to learn from or adapt.

---

## ⚙️ Core Features

- **One-button playback** — short press triggers a random McCoy quote  
- **LED feedback** — optional playback and status indication  
- **Rechargeable power** — 3.7 V LiPo battery with TP4056 charge control  
- **Compact design** — easy to assemble, modify, or repair  
- **Open firmware** — written in C/C++ under PlatformIO for ATtiny85  
- **Fully documented** — hardware, software, and audio all versioned in GitHub  

---

## 🔧 Hardware Snapshot

| Function | Component |
|-----------|------------|
| MCU | ATtiny85 |
| Audio | DFPlayer Mini + 8 Ω speaker |
| Power | 3.7 V 280 mAh LiPo battery |
| Charging | TP4056 module (with protection) |
| Control | Single pushbutton (PB2) |
| Indicator | Optional LED (PB0 or PB1) |
| Protection | Polyfuse on battery positive |
| Decoupling | 100 nF + 10 µF capacitors near MCU and amplifier |

> Design goal: use common, low-cost parts that can be easily replaced or reused.

---

## 💻 Software Snapshot

| Category | Details |
|-----------|----------|
| Platform | PlatformIO (VS Code) |
| Language | C / C++ |
| Core | MicroCore for ATtiny85 |
| Behavior | Short press → random clip playback |
| Optional | Long press → volume mode |
| LED Logic | On during playback; blink for mode; fade on volume adjust |
| Stability | Software debounce, low-latency playback, sleep-ready design |

> The firmware focuses on timing stability and clean state transitions, using minimal RAM while preserving responsive control.

---

## 🔊 Audio & Presentation

- Features curated McCoy voice clips from the *Star Trek* universe  
- DFPlayer-compatible WAV/MP3 format (8-bit mono, 22 kHz recommended)  
- Audio files stored on microSD for quick updates or customization  
- Optional LED synchronizes with speech playback for visual flair  

---

## 🧠 Philosophy

> Keep it simple. Keep it fixable. Keep it fun.  

McCoy Speaks isn’t just about playback — it’s about **learning by building**.  
Every component was chosen to be understandable, hand-solderable, and reproducible using off-the-shelf modules.  
GitHub serves as the single source of truth for the design, code, and documentation, ensuring anyone can follow along or rebuild it from scratch.

---

## ❤️ Credits

Created as a personal learning and maker project blending hardware, firmware, and retro fandom.  
Inspired by the wit and wisdom of **Dr. Leonard H. McCoy**, whose voice reminds us that sometimes — engineering *is* a form of medicine.

---

**“I’m a doctor, not a microcontroller!”**

---
