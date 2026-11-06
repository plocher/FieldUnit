# Agent Brief: User Documentation Work

Written 2026-11-04 from a retrospective on the session that produced `talk-outline.md`, `glossary-segments.md`,
`talk-intent.md` and `realization-inventory.md`. Give this to an agent at the start of any session on the user
documentation effort, together with `docs/plan/user-docs-plan.md` (read its "Course correction" section first).

## The brief

You are helping John Plocher plan user documentation for FieldUnit and FieldUnit-Subdivision. Several efforts run in
parallel on shared branches: fetch before every push, and read commits from others before building on a file.

Before writing anything:

1. **Validate the audience.** For each audience in `docs/plan/user-docs-plan.md`, ask John: what do they already know,
   what is new to them, what would make it worth their hour? Treat the plan's audience assumptions as hypotheses.
2. **Inventory the product, not just the docs.** Read `src/` (especially `src/drivers/`), `examples/`, `tests/`, and
   every ADR, then the glossary and primer. Build a cited inventory with a status for each item (exists, partial,
   planned, gap): roles, seams, deployments, code line encodings and transports, buses, device drivers, field
   processors, office implementations, and the combinations they allow. Ask what is new compared with JMRI, CATS,
   C/MRI, LCC, DCC-EX and plain DCC. Start from `docs/plan/realization-inventory.md`.
3. **Find the differentiator before the narrative.** It is composability: every layer is replaceable, and the
   alternatives need not be ours (see "The headline" and the SIG tagline in `user-docs-plan.md`). History and
   concepts support it; they are not the payload for expert audiences.

While writing:

- **Prefer gap analysis to authoring.** For each topic: what we know, what we built, what is missing, who or what can
  supply it (a build, a measurement, a SIG member, a source). Draft prose only after John has accepted the intent.
- **Cover the physical layer as well as the logical one:** devices, wiring, room on the layout, DCC interplay, I/O
  counts, cost, interoperability.
- **Keep the vocabulary inside its era**, and name later rules in the future tense.
- **Flag prototype claims with your confidence.** Ask John where it is low; he knows the prototype better than the
  web does. Credit his facts as "(owner, date)". Mark unopened web sources as leads, not citations.
- **Treat feedback as additive by default.** If it could mean "add" or "replace", ask.
- **Keep changes reviewable:** small commits, one decision per commit, an open-questions list at the end of every
  file.

## Why each rule exists

What went wrong without it, in the session of 2026-11-02 to 2026-11-04:

| Rule | What happened |
|---|---|
| Validate the audience | Four drafts of a history talk for SIG members who already knew the history; the new material was the tooling |
| Inventory the product | The first "what have you done" list missed drivers, buses, deployments and the seam combinations of FieldUnit ADR 0004 |
| Differentiator first | Composability surfaced only after the owner pointed out the blind spot |
| Gap analysis over authoring | Prose drafts were rewritten or rested once the intent changed |
| Physical layer | The experts' first questions were about room for masts, track spacing, aisles, wiring and cost; the docs had none of it |
| Era vocabulary | Route locking and powered switches appeared in segments set before relays |
| Flag prototype claims | Corrections from the owner: towers stood at bottlenecks, not at every siding; a train order signal is not a block signal; "powered switch" is a CTC term; color light vs CPL vs PL naming |
| Additive feedback | A request to add a story line was read as "replace the content", and draft 5 was withdrawn |
| Fetch before push | A push was rejected because the owner had pushed edits to the same branch |
