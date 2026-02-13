# Project Structure

This document describes the purpose and contents of the main folders used in this GTK (C / GTK4) project.

---

## 📁 src/

This folder contains the **core application logic** and entry point.

### Contents:
- **main.c**
  - Application entry point
  - Initializes GTK
  - Creates and runs the main application window
- **app.c / app.h** (optional but recommended)
  - Encapsulates the `GtkApplication`
  - Handles application lifecycle (startup, activate, shutdown)
  - Keeps `main.c` minimal and clean

### Responsibilities:
- Application initialization
- High-level flow control
- Connecting widgets together
- No UI layout logic specific to widgets

---

## 📁 widgets/

This folder contains **custom reusable UI components**.

### Structure:
```

widgets/
├── headers/
└── sources/

```

### widgets/headers/
- Contains **public widget interfaces** (`.h`)
- Declares:
  - Widget creation functions (e.g. `*_new()`)
  - Public APIs used by the rest of the application
- Does NOT contain implementation details

Example:
- `login_form.h`
- `navbar.h`
- `custom_button.h`

---

### widgets/sources/
- Contains **widget implementations** (`.c`)
- Defines:
  - Widget layout
  - Signals and callbacks
  - Internal helper functions
- Each `.c` file corresponds to one `.h` file

Example:
- `login_form.c`
- `navbar.c`
- `custom_button.c`

---

### Responsibilities:
- Encapsulate UI logic
- Promote reusability
- Keep `src/` free from widget internals

---

## 📁 resources/

This folder contains **non-code assets** used by the application.

### Contents:
- **style.css**
  - GTK CSS styling
  - Colors, spacing, fonts, themes
- **icons/**
  - Application and UI icons (SVG/PNG)
- **images/** (optional)
  - UI images, backgrounds, logos
- **.ui files** (optional)
  - GtkBuilder XML files if used

### Responsibilities:
- Visual appearance
- UI styling and theming
- Static assets bundled with the application

---