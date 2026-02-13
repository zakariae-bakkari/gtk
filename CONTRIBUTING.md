# Contributing Guidelines

## Naming Conventions (Mandatory)
- Files: lowercase snake_case
- Functions: module_action()
- Structs: PascalCase
- Macros: UPPER_SNAKE_CASE

## Code Rules (Mandatory)
- One widget per `.c/.h` file
- No logic in `main.c`
- No global variables (unless justified)
- Functions must be < 50 lines when possible

## Widgets
- Widgets must be placed in `widgets/`

## Commits
- Clear commit messages
- No committing build/ folders
---

## Git Commit Message Convention

All commits **must** follow the convention below.
This ensures a clean history, easier reviews, and better collaboration.

---

### 2.1 Format

Each commit message must follow this structure:

| Element | Description |
| --- | --- |
| `type` | Nature of the change |
| `scope` | Affected module or area (optional) |
| `description` | Short, imperative summary |

**Format:**

```

type(scope): description

```

Rules:
- Use **lowercase**
- Description must be **imperative** (e.g. “add”, not “added”)
- No period at the end
- Max ~72 characters for the first line

---

### 2.2 Commit Types

| Type | Usage |
| --- | --- |
| `feat` | New feature |
| `fix` | Bug fix |
| `refactor` | Code restructuring (no behavior change) |
| `docs` | Documentation only |
| `test` | Tests only |
| `style` | Formatting only (no logic change) |
| `chore` | Maintenance tasks |
| `perf` | Performance improvements |
| `ci` | CI/CD related changes |
| `build` | Build system or dependencies |

---

### 2.3 Examples

| Commit message | Meaning |
| --- | --- |
| `feat(auth): add JWT login` | New authentication feature |
| `fix(user): prevent null email` | Bug fix |
| `docs(readme): update setup` | Documentation change |
| `chore(widgets): rename button to bouton` | Maintenance / refactor |
| `build(cmake): update gtk dependencies` | Build-related change |

---

### 2.4 Invalid Examples (Do NOT use)

❌ `update files`
❌ `fixed bug`
❌ `commit 1`
❌ `Add new feature`

---
