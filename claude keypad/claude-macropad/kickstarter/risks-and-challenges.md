# Claude Macropad — Risks & Challenges

> This is a mandatory Kickstarter field. It must be honest, specific, and confident.
> Vague assurances ("we've thought about everything") are a red flag to experienced backers — they will skip your project.
> Structure: name the risk → describe its likelihood → describe your mitigation.
> Paste the content below directly into the Risks & Challenges field in the Kickstarter project editor.

---

## Risks & Challenges (Campaign Text — Paste into Kickstarter)

Hardware projects are genuinely hard to deliver on time. We're being upfront about the specific risks and what we've done to reduce them.

---

**Supply chain — microcontroller availability**

The Pro Micro (ATmega32U4) has historically been subject to availability fluctuations. Our mitigation: we will purchase full controller inventory before the campaign launches, sized to cover our worst-case funding scenario. If the Pro Micro becomes unavailable, the PCB footprint is compatible with a pin-compatible alternative (RP2040-based drop-in with QMK support) — firmware support for this fallback is being validated in parallel.

---

**Supply chain — component lead times**

EC11 encoders, Kailh MX hotswap sockets, and PBT keycap blanks all have stable supply from LCSC/Mouser but can face 4–8 week lead times at volume. We will pre-order all components immediately after the campaign closes rather than waiting until PCB production is confirmed.

---

**Manufacturing scale-up**

Our prototype was produced in a 5-unit run at JLCPCB. Scaling to a full production run introduces risk of yield variation, especially on the hotswap socket placement. Our mitigation: a 50-unit pilot run will be completed before the full production run is ordered, with 100% electrical testing on the pilot units. Any PCB revision required after the pilot will be executed before the main run.

---

**FDM case at volume**

FDM printing individual cases is not scalable beyond ~100 units without significant lead time. For production volumes above 150 units, we will evaluate switching the case to SLS nylon or vacuum-formed ABS — these provide better dimensional consistency. Any material change will be communicated to backers before production is ordered, with a sample approval step.

---

**Firmware readiness**

The QMK firmware is functional on the prototype hardware. VIA support is in integration testing. The risk is that the VIA keymap JSON requires revision after PCB rev 2 finalises pin assignments. Our mitigation: firmware is fully open-source and community-maintainable; any post-ship bugs can be fixed by users directly without waiting on us. We will not ship until VIA integration passes our bring-up checklist.

---

**Timeline delays**

Our target delivery date includes a [2–3 month] buffer over our internal best-case estimate. The most likely delay scenarios are:

- PCB revision required after pilot run (+4–6 weeks)
- Case tooling adjustment (+2–4 weeks)
- International shipping congestion (+2–4 weeks)
- Chinese New Year shutdown if production overlaps January/February (+3–5 weeks)

If a delay becomes likely, backers will receive an update immediately — not after the original date has passed. We commit to a minimum monthly update cadence throughout production.

---

**Overfunding**

If we significantly exceed our funding goal, we will need to scale component orders and case production proportionally. We have discussed capacity with our manufacturing partners up to [TBD] units. Beyond that threshold, we will communicate clearly with backers about any timeline impact before accepting additional pledges via late-pledge.

---

**Shipping and customs**

International customs duties are unpredictable and vary by destination country. We are shipping DDU (Delivered Duty Unpaid) to non-EU regions — local import duties, if any, are the backer's responsibility. For the EU, we will register for the IOSS scheme to pre-collect and remit VAT. Backers in high-duty countries (Brazil, India) should be aware that import taxes may apply on delivery.

---

**This is a fan project**

Claude and Claude Code are trademarks of Anthropic, PBC. This project is not affiliated with or endorsed by Anthropic. If Anthropic were to object to the use of their branding or colour scheme, we would need to revise the case design. We have kept the design to a colour and silhouette reference rather than reproducing official logos, to minimise this risk. In the event of a required redesign, we would communicate the change to backers before production.

---

**What we won't do**

We will not go dark. We will not take the money and stop communicating. If we hit a serious obstacle we cannot resolve, we will tell backers clearly and work out options — refunds, revised scope, extended timeline — transparently.

---

*(End of Risks & Challenges campaign text)*
