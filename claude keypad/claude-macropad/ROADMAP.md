# Claude Macropad — Product Development Roadmap

**Creator:** Magnus (solo)  
**Project:** Fan-made mechanical macro pad for Claude Code power users  
**Spec version:** v1.3 (current)  
**Go-to-market path:** Open prototype → Kickstarter campaign → mass manufacturing run → open-source release

---

## Where We Are (April 2026)

Sprint 1 officially closed April 17, 2026. The project now moves into parallel tracks: firmware development runs concurrently with prototype manufacturing so neither blocks the other.

---

## Phase 0 — Specification (Complete ✅)

Ran across four spec iterations before any manufacturing began.

| Version | Key change |
|---|---|
| Spec v1 | Initial concept — SLS nylon case, broad component sketch |
| Spec v1.1 | SLS PA12 nylon with dye finish; 122 × 67 mm footprint; PCB 116 × 51 mm |
| Spec v1.2 | Switched to FDM ASA (no dye step needed); colour swatch test added to process |
| Spec v1.3 | Dimensions finalised against Rama M6-B + Stream Deck Mini benchmarks; full BOM locked |

**v1.3 locked dimensions:**

| | |
|---|---|
| Overall footprint | 120 × 66 mm |
| Total height (desk → keycap top) | 27.7 mm |
| PCB bounding box | 114.5 × 49 mm |
| Wall thickness | 2.5 mm flat · 3.0 mm at arm joints |
| Case construction | Two-part tray + top plate, M2 × 6 mm screws, M2 brass heat-set inserts |
| Case material | FDM ASA, Claude orange `#DA7756` (Pantone 7416 C) |

---

## Phase 1 — Sprint 1: Prototype Prep
**Status: 🟡 Review (closed April 17, 2026)**  
**Capacity:** 15 hrs available · 11 hrs planned · solo

Sprint goal: have everything ready to place the JLCPCB prototype order.

**P0 — Critical path:**
- [ ] DM original X creator for alignment and green-light *(30 min — Day 1)*
- [ ] Order FDM ASA colour swatch — orange + red-orange variants *(15 min — Day 1–2)*
- [ ] Finalise case CAD — robot-face silhouette, tolerances for FDM, M2 insert positions, encoder cutouts, USB-C clearance *(5–6 hrs — Day 2–6)*
- [ ] PCB layout in EasyEDA — 1×5 matrix, Pro Micro footprint, 2× EC11, 5× Kailh MX hotswap, diodes, USB-C *(3–4 hrs — Day 5–8)*

**P1 — Goal at risk:**
- [ ] Component sourcing list — LCSC/Mouser part numbers, quantities, lead times for all 14 BOM lines
- [ ] QMK firmware skeleton — `keyboard.json`, stub `config.h`, matrix and encoder pin assignments

**P2 — Stretch:**
- [ ] Place JLCPCB prototype order (PCB + PCBA, 5-unit minimum)

**Exit criteria:** creator green-light received · CAD exported to STL · PCB passes DRC · colour swatch approved · components sourced · QMK stub compiles  
**Key risks:** creator declines (backup: independent launch) · EC11 footprint mismatch (mitigate: verify against datasheet before ordering) · swatch delays (does not block core deliverables)

---

## Phase 2 — Sprint 2: Firmware Foundation
**Status: ⬜ Not Started**  
**Runs concurrently with PCB manufacturing lead time (~2–3 weeks)**

Firmware reaches feature-complete state on a dev board while JLCPCB produces the PCBs. No hardware dependency for this sprint.

**Deliverables:**
- [ ] `config.h` — matrix dimensions (1×5), encoder pins, USB VID/PID descriptor
- [ ] `keymap.c` — default Claude Code layer: accept (`Tab`), reject (`Esc`), new session (`Ctrl+N`), toggle mode (`Ctrl+Shift+M`), submit (`Enter`)
- [ ] Encoder map — left: scroll history CW/CCW + clear context on click · right: cycle layer CW/CCW + custom on click
- [ ] `via.json` — all 5 key positions + 4 encoder actions exposed, validates against VIA schema
- [ ] Additional layers — at minimum: Layer 0 (Claude Code default) · Layer 1 (user-defined blank)
- [ ] Compile test — `qmk compile -kb claude_macropad -km default` passes clean
- [ ] Flash test on Pro Micro dev board — all keycodes register in VIA / `hid_listen`
- [ ] `firmware/README.md` — build steps, flash steps, VIA usage, layer switching

**Exit criteria:** firmware compiles clean · VIA JSON validates · all keys and encoder events register correctly on dev hardware

---

## Phase 3 — Sprint 3: First Build & Bring-Up
**Status: ⬜ Not Started**  
**Starts when:** JLCPCB prototype PCBs arrive

Physical assembly of the first complete unit and systematic verification of every subsystem.

**Hardware receive:**
- [ ] PCBs inspected — no cold joints, correct component placement and orientation, hotswap sockets seated flush
- [ ] Case parts printed in ASA — fit-check: PCB mounting holes align, encoder shaft height clears top plate, USB-C port fully accessible, no warp

**Assembly & bring-up:**
- [ ] Pro Micro socketed and flashed with Sprint 2 firmware
- [ ] Unit enumerates as HID keyboard — confirmed in Device Manager / `system_profiler`
- [ ] All 5 keys register correct keycodes in VIA
- [ ] Both EC11 encoders register CW, CCW, and push-click as separate events
- [ ] Hotswap retention — switch seats fully, removes cleanly without lifting pad
- [ ] USB-C cable seats without play; no intermittent disconnects under light flex
- [ ] Case fasteners torqued with M2 screws into heat-set inserts — no cracking, secure close

**Documentation:**
- [ ] Prototype photos taken — all angles (required for Kickstarter; see `kickstarter/pre-launch-checklist.md`)
- [ ] All errata documented in `docs/errata-v1.md` (PCB, case, firmware issues found)
- [ ] `renders/render.png` updated with real prototype photo

**Exit criteria:** complete unit passes all key/encoder/USB checks with no blocking defects · errata list written · prototype photos taken

---

## Phase 4 — Sprint 4: Refinement (Rev 2)
**Status: ⬜ Not Started**  
**Starts when:** Sprint 3 errata list is complete

Resolve all Sprint 3 findings. A second prototype build confirms the fixes before any campaign content is locked.

**PCB rev 2:**
- [ ] Fix all footprint, trace, routing, or silkscreen issues from errata list
- [ ] Re-run DRC — zero errors
- [ ] Order 5-unit rev 2 run from JLCPCB

**Case rev 2:**
- [ ] Adjust tolerances from Sprint 3 measurements (encoder shaft height, PCB pocket, USB-C slot width)
- [ ] Re-print one unit, fit-check against rev 2 PCB

**Firmware polish:**
- [ ] Debounce tuning — eliminate any chatter observed in Sprint 3
- [ ] Encoder sensitivity adjustment if needed
- [ ] Final VIA JSON review — ensure layer names and key labels are clean for public use

**Second build:**
- [ ] Assemble one complete unit from rev 2 PCB + case
- [ ] Re-run full Sprint 3 bring-up checklist — must pass with zero new errata

**Documentation:**
- [ ] BOM finalised — all 14 lines with LCSC/Mouser part numbers, exact quantities, unit cost
- [ ] `docs/assembly.md` written — step-by-step with photos, torque specs, flashing steps
- [ ] Rev 2 prototype photos taken (these are the campaign photos)

**Exit criteria:** rev 2 unit passes full bring-up checklist · zero new errata · BOM and assembly guide complete · campaign-quality prototype photos in hand

---

## Phase 5 — Kickstarter Prep
**Status: ⬜ Not Started**  
**Starts when:** Rev 2 prototype is in hand (Phase 4 complete)  
**Hard requirement:** Kickstarter will not approve a hardware campaign without a working physical prototype

All placeholders in `kickstarter/campaign-story.md` must be resolved before submission.

**Media production:**
- [ ] Replace all `[RENDER: ...]` placeholders in `campaign-story.md` with real prototype photos
- [ ] Hero image — 3/4 angle assembled unit, 1024 × 576 px minimum (16:9)
- [ ] Feature callout image — flat lay with component labels
- [ ] Close-up strip — hotswap socket, encoder knob, USB-C port
- [ ] PCB top-down — bare board showing component layout
- [ ] Case detail — robot-face silhouette cutout
- [ ] VIA screenshot or GIF — browser app with keymap loaded
- [ ] Reward tier flat-lays — one image per tier showing box contents
- [ ] Campaign video — 90–120 seconds, prototype working on camera (see `kickstarter/pre-launch-checklist.md` for script structure)

**Campaign content:**
- [ ] All `[TBD]` values in `reward-tiers.md` filled — COGS confirmed from JLCPCB volume quote (100/250/500 units)
- [ ] Funding goal calculated — tooling + minimum production run + 10% fees + 15% contingency
- [ ] Reward tier prices set — Early Bird ≥ 20% below standard, both cover COGS + fees
- [ ] Shipping costs confirmed per region from fulfilment partner
- [ ] Delivery dates set — internal best-case + 2–3 month buffer
- [ ] `[Creator name]` filled, creator bio written, GitHub and Reddit linked
- [ ] AI disclosure added if any AI-generated content used in campaign

**Platform setup:**
- [ ] Kickstarter creator account created and identity-verified
- [ ] BackerKit project created (for post-campaign pledge management)
- [ ] Pre-launch page live on Kickstarter
- [ ] Email list live (Mailchimp or Klaviyo) — collect signups independent of Kickstarter
- [ ] Pre-launch page URL shared to: r/MechanicalKeyboards, r/MechKeys, r/ClaudeAI, GeekHack, personal channels

**Pre-launch target:** 500+ followers / email subscribers before campaign goes live  
**Lead time:** Kickstarter review takes 1–3 business days after submission

**Exit criteria:** all `[RENDER]` and `[TBD]` placeholders resolved · campaign submitted and approved · pre-launch page live · 500+ followers

---

## Phase 6 — Kickstarter Campaign (Live)
**Status: ⬜ Not Started**  
**Duration:** 30 days (do not exceed 35)

- [ ] Campaign goes live — email blast and community posts fire at launch
- [ ] Monitor Day 1–2 closely — funding spikes at open; respond to all backer questions within 24 hrs
- [ ] Mid-campaign update posted (Day 14–15) — progress, any stretch goal unlocks
- [ ] Final 48-hour push — reminder email, social posts
- [ ] Campaign closes — all-or-nothing: funded or nothing collected

**If funded:**
- [ ] Kickstarter pays out ~14 days after campaign close (minus ~10% fees)
- [ ] BackerKit pledge manager opened — collect final shipping addresses and add-on selections
- [ ] Backer update posted immediately: "We're funded — here's what happens next"

**Exit criteria:** campaign successfully funded · BackerKit open · all backer addresses collected

---

## Phase 7 — Mass Production
**Status: ⬜ Not Started**  
**Starts when:** Kickstarter funds clear and BackerKit address collection closes (~Week 3 post-campaign)

| Week (post-campaign) | Milestone |
|---|---|
| 1–3 | BackerKit pledge manager open — collect addresses and add-ons |
| 2–4 | BOM quantities confirmed; all components pre-ordered (Pro Micro, EC11, Kailh sockets, keycaps) |
| 4–8 | JLCPCB full production PCB run |
| 5–9 | Case production run — evaluate SLS or vacuum-form if FDM unscalable above 150 units |
| 9–12 | QC inspection — 100% electrical test on all units; visual pass on cases |
| 9–12 | Packing and fulfilment prep |
| 12–14 | Freight to regional fulfilment hubs |
| 14–16 | Backer shipments dispatched |

**QC pass criteria (per unit):** all 5 keys register · both encoders register CW/CCW/click · USB enumerates · hotswap retention passes · case fits flush with no cracking at inserts

**Risk flags to communicate proactively:**
- Chinese New Year (January/February) — plan production run to avoid or buffer around it
- MCU chip availability — pre-order Pro Micro stock immediately after campaign closes
- FDM case yield — if rejection rate exceeds 10%, evaluate tooling alternatives before full run

**Exit criteria:** all backer units shipped · tracking numbers sent to all backers via BackerKit

---

## Phase 8 — Open Source Release
**Status: ⬜ Not Started**  
**Starts when:** all backers have received their units (Phase 7 complete)

Publish everything needed for any builder to replicate the pad independently.

- [ ] `hardware/` — EasyEDA JSON source, Gerber export, STL files for case (top + bottom), BOM with LCSC part numbers
- [ ] `firmware/` — complete QMK source, compiled `.hex` for Pro Micro, VIA JSON
- [ ] `docs/assembly.md` — final version with production photos
- [ ] `hardware/README.md` — sourcing notes, print settings (layer height, infill, ASA profile), EasyEDA version
- [ ] `firmware/README.md` — full build + flash walkthrough, VIA usage
- [ ] Licence files added — firmware: GPL-2.0 · PCB: CERN OHL-W 2.0 · case: CC BY-NC-SA 4.0
- [ ] Main `README.md` updated — links to BOM, assembly guide, firmware release, all renders
- [ ] GitHub release tag `v1.0` — Gerbers, STLs, and compiled `.hex` attached as release assets

**Exit criteria:** a builder with no prior context can source, print, assemble, and flash from the repo alone

---

## Milestone Overview

| Milestone | Phase | Depends on |
|---|---|---|
| Spec locked (v1.3) | 0 | — |
| Creator green-light | 1 | DM response |
| JLCPCB order placed | 1 | PCB DRC-clean + CAD exported |
| Firmware compiles + validates | 2 | Sprint 1 pin assignments |
| First unit assembled and functional | 3 | PCBs arrive from JLCPCB |
| Rev 2 unit passes bring-up | 4 | Sprint 3 errata list |
| Campaign-quality prototype in hand | 4 | Rev 2 build |
| Kickstarter approved and live | 5 | Rev 2 + all media produced |
| Campaign funded | 6 | 30-day campaign |
| All backers shipped | 7 | Production run complete |
| Open source v1.0 published | 8 | Shipping complete |

---

## Out of Scope — v1.0

These are explicitly deferred. Do not scope-creep mid-campaign.

- RGB / per-key lighting
- Wireless (Bluetooth or 2.4 GHz)
- OLED or e-ink display
- Custom keycap legends, dye-sub, or artisan caps
- Injection-moulded case (MOQ and tooling cost not justified at Kickstarter scale)
- Windows/macOS companion app (VIA covers this)
- Second hardware revision post-Kickstarter (v2 is a separate project)

---

*Fan project — not affiliated with or endorsed by Anthropic, PBC.*  
*Claude and Claude Code are trademarks of Anthropic, PBC.*
