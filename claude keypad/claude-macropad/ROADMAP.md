# Claude Macropad — Roadmap

A fan-made mechanical macro pad for Claude Code power users.  
5 keys · 2 rotary encoders · Claude orange · Robot-face silhouette

---

## Overview

This roadmap tracks the project from concept to a finished, repeatable build. Progress moves through five sprints: design, prototyping, first build, refinement, and release.

---

## Sprint 1 — Prototype Prep
**Status: 🟠 In Progress**

All design decisions locked and sourcing complete before any manufacturing orders are placed.

- [ ] Creator alignment (goals, scope, licensing, fan-project disclaimer)
- [ ] Case CAD (robot-face silhouette, 120 × 66 × 28 mm, FDM ASA tolerances)
- [ ] PCB design in EasyEDA (matrix wiring, Pro Micro footprint, EC11 footprint, Kailh MX hotswap sockets)
- [ ] FDM colour swatch test (dial in Claude orange `#DA7756` on target printer/filament)
- [ ] Component sourcing list with links and lead times (Pro Micro, EC11 encoders, Kailh sockets, keycaps, USB-C cable)
- [ ] QMK firmware skeleton (keyboard definition, matrix config, encoder map, VIA JSON stub)
- [ ] JLCPCB prototype order placed (PCB + PCBA, 5-piece minimum)

**Exit criteria:** order confirmation received from JLCPCB; FDM swatch approved.

---

## Sprint 2 — Firmware Foundation
**Status: ⬜ Not Started**

Firmware reaches feature-complete state while hardware is being manufactured.

- [ ] QMK `config.h` — matrix pins, encoder pins, USB descriptor (VID/PID)
- [ ] Default keymap — 5 Claude Code shortcuts mapped (accept, reject, new session, toggle mode, custom)
- [ ] Encoder actions — left encoder: scroll history / right encoder: adjust context window (or user-defined)
- [ ] VIA support — `via.json` with all key positions and encoder actions exposed
- [ ] `README` for `firmware/` — build instructions, flash instructions, VIA usage
- [ ] Firmware tested on a Pro Micro dev board (if hardware delayed)

**Exit criteria:** firmware compiles cleanly with `qmk compile`; VIA JSON validates.

---

## Sprint 3 — First Build & Bring-Up
**Status: ⬜ Not Started**

Assemble prototype and verify every subsystem works.

- [ ] PCBs received from JLCPCB — visual inspection (no cold joints, correct components)
- [ ] Case printed in ASA — fit-check PCB mounting holes, encoder cutouts, USB-C port clearance
- [ ] Flash firmware — confirm Pro Micro enumerates as HID keyboard
- [ ] Key matrix test — each key registers correct keycode in VIA / `hid_listen`
- [ ] Encoder test — both encoders register CW/CCW events and push-click
- [ ] Hotswap socket test — switch removal/reinsertion without pad damage
- [ ] USB-C cable test — braided cable fits port; no intermittent disconnects
- [ ] Add product renders to `renders/render.png` (photo or CAD render of assembled unit)
- [ ] Document any PCB or case errata discovered

**Exit criteria:** all keys, encoders, and USB enumerate correctly; no hardware defects blocking use.

---

## Sprint 4 — Refinement
**Status: ⬜ Not Started**

Fix errata found in Sprint 3 and polish the design for repeatability.

- [ ] PCB rev 2 — fix any footprint, trace, or silkscreen issues from Sprint 3
- [ ] Case rev 2 — adjust tolerances based on print results (encoder shaft height, PCB fit)
- [ ] Firmware polish — debounce tuning, encoder sensitivity, QMK RGB/LED stubs if desired
- [ ] Second prototype build with rev 2 files — confirm errata resolved
- [ ] Bill of Materials (BOM) finalized with Mouser/LCSC part numbers and quantities
- [ ] Assembly guide written (`docs/assembly.md`) — photos, torque specs, flashing steps

**Exit criteria:** second prototype passes all Sprint 3 checks with no new errata.

---

## Sprint 5 — Documentation & Release
**Status: ⬜ Not Started**

Publish everything needed for someone else to build the same pad.

- [ ] Release CAD files (STL + source) under CC BY-NC-SA 4.0
- [ ] Release PCB files (Gerbers + EasyEDA JSON) under CERN OHL-W 2.0
- [ ] Release firmware under GPL-2.0 (QMK standard)
- [ ] `hardware/README.md` — sourcing notes, print settings, CAD tool version
- [ ] `firmware/README.md` — full build + flash walkthrough
- [ ] Update main `README.md` — link renders, BOM, assembly guide, firmware release
- [ ] GitHub release tag `v1.0` with Gerbers, STLs, and compiled `.hex` attached
- [ ] Optional: short build video or photo journal

**Exit criteria:** a builder with no prior context can source, print, assemble, and flash from the repo alone.

---

## Milestones

| Milestone | Sprint | Target |
|---|---|---|
| Design locked | 1 | — |
| Firmware compiles | 2 | — |
| First hardware in hand | 3 | — |
| Errata resolved | 4 | — |
| Public release | 5 | — |

---

## Out of Scope (v1.0)

- RGB / per-key lighting
- Wireless (Bluetooth)
- OLED or e-ink display
- Custom keycap legends or dye-sub printing
- Production run (this is a one-off / open-source DIY project)

---

*Fan project — not affiliated with or endorsed by Anthropic, PBC.*  
*Claude and Claude Code are trademarks of Anthropic, PBC.*
