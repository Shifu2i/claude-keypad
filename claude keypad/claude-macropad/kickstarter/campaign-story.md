# Claude Macropad — Kickstarter Campaign Story

> This file is the full body of the Kickstarter campaign page.
> All `[RENDER: ...]` tags are placeholders — replace with real prototype photos or clearly-labelled renders before submitting.
> Kickstarter prohibits photorealistic renders as primary product images. Real prototype photos are required.

---

## Campaign Header

**Project Title:** Claude Macropad — A Mechanical Macro Pad for Claude Code

**Subtitle / Blurb:**
> 5 keys, 2 encoders, and one very specific workflow. A fan-made mechanical macro pad built for Claude Code power users.

**Category:** Technology → Hardware
*(Alternative: Design → Product Design)*

**Campaign Duration:** 30 days *(recommended — do not exceed 35)*

---

---

## Campaign Body

---

### [HERO IMAGE]
> `[RENDER: Hero shot — assembled macro pad on a desk, angled 3/4 view, Claude orange case, black keycaps and knobs, USB-C cable plugged in. Real prototype photo required for Kickstarter approval. If prototype is not yet ready, use a clearly-labelled CAD render or sketch here and replace before launch.]`

---

### What Is the Claude Macropad?

The Claude Macropad is a five-key mechanical macro pad with two rotary encoders, built specifically for people who live in Claude Code. It sits next to your keyboard, gives your most-used actions a dedicated physical key, and gets out of the way.

It is a fan project. It is not affiliated with or endorsed by Anthropic.

---

### [FEATURE OVERVIEW IMAGE]
> `[RENDER: Flat lay — macro pad centre frame, with callout labels pointing to: 5 keycap positions, 2 encoder knobs, USB-C port, hotswap sockets (showing one switch half-extracted), Pro Micro controller position on PCB.]`

---

### Why Does This Exist?

Claude Code users reach for the same five or six actions hundreds of times a day — accept, reject, new session, toggle mode, scroll through history. Right now those all live inside a terminal window, mapped to keyboard shortcuts that compete with every other shortcut on a full keyboard.

A dedicated physical pad changes that. One press. No hunting. No chord conflicts. No accidental trigger.

---

### Key Features

| | |
|---|---|
| **5 mechanical switches** | Kailh MX hotswap sockets — use any MX-compatible switch you already own |
| **2 rotary encoders** | EC11 rotary encoders with push-click, black anodised aluminium knobs |
| **QMK firmware** | Open-source firmware. Full VIA support — remap every key and encoder in-browser, no coding required |
| **Layered profiles** | Multiple layers mean the same pad works across different tools and contexts |
| **Claude orange case** | FDM ASA, `#DA7756` — robot-face silhouette, 120 × 66 × 28 mm |
| **USB-C** | Braided USB-C cable included |
| **Open source** | PCB, case files, and firmware all published under open licences after shipping |

---

### [FEATURE CLOSE-UPS]
> `[RENDER: 3-panel close-up strip — (1) hotswap socket with switch being removed, (2) EC11 encoder knob detail shot, (3) USB-C port on the case edge.]`

---

### How It Works

**Plug in. Open VIA. Start mapping.**

1. Connect the macro pad via USB-C — it enumerates immediately as a standard HID keyboard, no driver installation required.
2. Open [usevia.app](https://usevia.app) in any Chromium browser.
3. Load the VIA keymap JSON (included in the box and on GitHub).
4. Drag and drop keycodes onto each key position and encoder action.
5. Changes save instantly to the macro pad's EEPROM — no software running in the background.

> `[RENDER: Screenshot or GIF of VIA interface showing the Claude Macropad layout loaded, with keys being remapped by drag-and-drop.]`

---

### Default Keymap

Out of the box, the macro pad ships with a Claude Code–focused default layer:

| Key | Default Action | Notes |
|---|---|---|
| Key 1 | Accept suggestion (`Tab`) | Remappable |
| Key 2 | Reject / dismiss (`Escape`) | Remappable |
| Key 3 | New session (`Ctrl+N`) | Remappable |
| Key 4 | Toggle compact mode (`Ctrl+Shift+M`) | Remappable |
| Key 5 (1.75U) | Submit / confirm (`Enter`) | Remappable |
| Left encoder CW/CCW | Scroll history up/down | Remappable |
| Left encoder click | Clear context | Remappable |
| Right encoder CW/CCW | Increase/decrease context window | Remappable |
| Right encoder click | Cycle layer | Remappable |

*All keycodes are fully remappable in VIA. The default map is an opinionated starting point.*

---

### The Hardware

> `[RENDER: PCB top-down photo — bare PCB showing component layout, matrix traces, Pro Micro footprint, Kailh hotswap sockets, EC11 positions. Label: "Custom PCB — manufactured by JLCPCB".]`

**PCB**
Designed in EasyEDA. Custom-dimension board (114.5 × 49 mm) manufactured and assembled by JLCPCB. The Pro Micro (ATmega32U4) controller is through-hole socketed, so it can be replaced without desoldering.

**Case**
FDM-printed ASA in Claude orange (`#DA7756` / Pantone 7416 C). The robot-face silhouette cutout on the top panel is what makes this recognisably *Claude*. ASA is used rather than PLA for its UV stability and heat resistance — it won't warp on a sun-exposed desk.

> `[RENDER: Case top-down showing robot-face silhouette cutout detail. Good natural light, flat lay.]`

**Switches**
Hotswap only — no soldering required. Any MX-compatible switch fits. Switches are **not included** in the base tier; the kit ships switch-ready so you can use your own. Switch add-on packs are available (see Rewards).

**Encoders**
2× EC11 rotary encoders with black anodised aluminium knobs, 20mm diameter. Both support push-click as a separate keycode.

---

### Specs

| | |
|---|---|
| **Dimensions** | 120 × 66 × 28 mm (desk surface to keycap top) |
| **Weight** | ~[TBD from prototype — est. 180–220 g] |
| **Keys** | 4× 1U + 1× 1.75U |
| **Encoders** | 2× EC11, push-click |
| **Switch compatibility** | MX-compatible (hotswap) |
| **Controller** | Pro Micro (ATmega32U4) |
| **Firmware** | QMK + VIA |
| **Connection** | USB-C (full-speed HID) |
| **Case material** | FDM ASA |
| **Case colour** | Claude orange `#DA7756` |
| **Stabilisers** | None required |
| **OS compatibility** | Windows, macOS, Linux (standard HID — no drivers) |

---

### Prototype Progress

This project is currently in the prototype stage. Here is where we are:

> `[RENDER: Prototype progress photo — show whichever of (PCB, printed case, assembled unit) exists at time of launch. Label with sprint/version number. This section MUST contain at least one real photograph of physical hardware for Kickstarter approval.]`

- ✅ Specifications locked (v1.3)
- ✅ PCB designed in EasyEDA
- ✅ Case CAD complete
- ✅ FDM colour swatch approved
- ✅ Component sourcing finalised
- ✅ QMK firmware functional on dev board
- ⬜ JLCPCB prototype units received *(in progress)*
- ⬜ First full assembly and bring-up

> `[RENDER: Side-by-side comparison — early sketch or v1 PCB layout alongside the final design, showing iteration.]`

We are not shipping a concept. Prototypes must pass a full bring-up checklist (all keys, both encoders, USB enumeration, hotswap retention) before the campaign launches.

---

### The Team

> `[PHOTO: Creator photo — real headshot or workshop photo of the person(s) behind the project.]`

**[Creator name]** — *hardware designer, firmware author*
[2–4 sentence bio: relevant background, keyboard community involvement, prior hardware projects, GitHub link.]

*[Add additional team members if applicable: PCB reviewer, case designer, etc.]*

Community roots: active on r/MechanicalKeyboards, [GeekHack profile link], [GitHub profile link].

---

### Manufacturing & Production Plan

PCBs are manufactured and assembled by **JLCPCB** (China). Case parts are FDM-printed and will be produced by **[printer/service name — TBD]** for the production run.

Post-campaign timeline:

| Milestone | Est. Duration After Campaign End |
|---|---|
| Pledge manager / address collection (BackerKit) | Weeks 1–3 |
| Final BOM lock and component pre-order | Weeks 2–4 |
| PCB production run (JLCPCB) | Weeks 4–8 |
| Case production run | Weeks 5–9 |
| QC and assembly | Weeks 9–12 |
| International shipping to fulfilment hubs | Weeks 12–14 |
| Backer shipments dispatched | Weeks 14–16 |

*Estimated delivery: [Month + Year — minimum 5 months post-campaign end, add 2-month buffer from your internal estimate.]*

---

### Rewards Overview

> See `reward-tiers.md` for full tier details.

> `[RENDER: Reward tier visual — one clean flat-lay image per tier showing exactly what's in the box. Label each clearly.]`

---

### Stretch Goals

If funding exceeds the base goal, the following unlocks are planned (subject to manufacturing feasibility):

| Funding Level | Unlock |
|---|---|
| **100% (base)** | Claude Macropad — Claude orange |
| **150%** | Second colour option: [TBD — e.g. off-white / "Claude light"] |
| **200%** | Laser-engraved keycap legends for default Claude Code layer |
| **300%** | [TBD — e.g. USB-C breakout for daisy-chaining, or acrylic top panel option] |

*Stretch goals will only be pursued if manufacturing capacity and timeline allow. They will never delay base-tier fulfilment.*

---

### Frequently Asked Questions

**Do I need to know how to code to use this?**
No. VIA is a drag-and-drop browser app. You never touch firmware unless you want to.

**Can I use my own switches?**
Yes. The board uses Kailh MX hotswap sockets — any MX-compatible switch fits. Switches are not included in the base tier.

**Is this compatible with Mac/Windows/Linux?**
Yes. It is a standard USB HID device. No drivers, no software running in the background.

**Does it work with applications other than Claude Code?**
Yes. It is a programmable keyboard — it sends standard keycodes. You can map it to any application.

**Is the firmware open source?**
Yes. QMK-based firmware and VIA keymap JSON are on GitHub under GPL-2.0.

**Are the PCB and case files open source?**
Yes. PCB files under CERN OHL-W 2.0, case files under CC BY-NC-SA 4.0. Both released on GitHub after shipping.

**Will you ship internationally?**
Yes — see reward tiers for per-region shipping costs.

**What if I want a different colour?**
[TBD — answer once stretch goal / variant plan is confirmed.]

**Is this officially endorsed by Anthropic?**
No. This is a fan project. Claude and Claude Code are trademarks of Anthropic, PBC.

---

*Fan project — not affiliated with or endorsed by Anthropic, PBC.*
*Claude and Claude Code are trademarks of Anthropic, PBC.*
