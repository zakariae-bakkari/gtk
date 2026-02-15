# Issues

## 1. 2024-02-15

**Description:**
The problem is related to the window (`fenetre`): the background color does not work as expected.

**Assigned to:**
@zakariae-bakkari

**Status:**
To fix

---

## 2. 2025-02-15

**Title:**
Button Width Behavior - All buttons expand to full container width

**Description:**
When buttons are added to a container, ALL buttons take up the full width of the container regardless of their sizing mode (TAILLE_AUTO, TAILLE_FIXE, or TAILLE_FIT_CONTENT).

**Expected Behavior:**

-   `TAILLE_AUTO` mode: Buttons should take only the space needed for their content
-   `TAILLE_FIXE` mode: Buttons should respect the specified width/height
-   `TAILLE_FIT_CONTENT` mode: Buttons should fit their content with optional minimum dimensions

**Current Behavior:**
All buttons expand horizontally to fill the entire container width, ignoring the sizing mode configuration.

**Root Cause Analysis:**
The issue appears to be:

1. Container's `enfants_hexpand` property may be overriding button sizing
2. GTK4's default behavior for widgets in a Box may be forcing expansion
3. The `gtk_widget_set_hexpand()` setting in bouton.c may not be properly preventing expansion in AUTO/FIT_CONTENT modes

**Affected Files:**

-   `widgets/sources/bouton.c` - Button creation and sizing logic
-   `widgets/sources/conteneur.c` - Container child expansion handling
-   `src/main.c` - Button creation examples

**Assigned to:**
@zakariae-bakkari

**Status:**
To fix

**Priority:**
Medium

**Steps to Reproduce:**

1. Create buttons with TAILLE_AUTO mode
2. Add them to a container
3. Observe that buttons take full width instead of fitting their content
