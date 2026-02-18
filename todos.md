# Project TODOs (QA & Verification)

> Last updated: 2026-02-18

## Overview

-   Scope: Manual QA for widgets (buttons, window, containers, inputs)
-   Reporting: Log issues in `issues.md` with clear repro steps
-   Status keys: TODO, In Progress, Blocked, Done

## Assignments

### @rafaa

-   Task: Test all buttons; verify each attribute and state (bg/fg, hover, radius, icon positions, size modes)
-   Actions:
    -   Check TAILLE_AUTO / TAILLE_FIXE / TAILLE_FIT_CONTENT behavior
    -   Validate CSS styling (hover, borders, tooltip)
    -   Confirm click callbacks and accessibility (mnemonics)
-   Deliverables: Report problems + screenshots
-   Status: TODO

### @noureddine

-   Task: Test window (Fenetre) and containers (Conteneur); assist rafaa if needed
-   Actions:
    -   Verify title alignment, header bar buttons, icon handling
    -   Check margins, padding, alignment (START/CENTER/END/FILL)
    -   Validate enfants_hexpand/vexpand behavior
-   Deliverables: Findings in `issues.md`, propose fixes
-   Status: TODO

### @meriem

-   Task: Test inputs (ChampTexte, ChampMotDePasse, ChampNombre, ChampSelect, ChampZoneTexte)
-   Actions:
    -   Validate required, max length, regex/policy, value bounds, selection
    -   Ensure CSS styles apply (focus, error) and callbacks fire
    -   Check accessibility (label mnemonics, tab order)
-   Deliverables: Edge cases list + bugs logged in `issues.md`
-   Status: TODO

## Reporting Template

-   Title: [Widget] Short issue description
-   Environment: Windows, GTK4
-   Steps to Reproduce: 1..n
-   Expected: ...
-   Actual: ...
-   Screenshots: (optional)
-   Suggested Fix: (optional)
