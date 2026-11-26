# Hold the Harbor Constitution

## Core Principles

### I. Fixed-Timestep Simulation, Fixed Update Order

- Gameplay MUST advance in fixed 60 Hz steps driven by a time accumulator. Rendering runs
  uncapped and MUST NOT change simulation results.
- No gameplay quantity (movement, timers, combo windows, invincibility frames, knockback) may be
  scaled by variable frame time.
- Each tick MUST run in this documented order: input → player → enemies → projectiles →
  collision resolve → deaths and events → spawner. A spec that changes the order MUST say so and
  update this list.
- Objects MUST NOT be removed from a container while it is being iterated. Mark them dead during
  update, then erase them in a separate pass.

**Rationale**: Combat timing must feel the same at 30 fps and 240 fps, and a fixed order makes bugs
reproducible.

### II. Single Ownership Through RAII

- Every resource has exactly one owner. The scene owns long-lived objects through
  `std::unique_ptr`. Short-lived objects (bombs, effects, coins, damage numbers) live by value in
  pools and are referenced by handle (slot index + generation).
- Nothing stores a raw pointer or reference to an object it does not own, unless that object's
  owner provably outlives the holder (e.g. a `const EnemyType&` held by an enemy, where the type
  registry outlives every enemy).
- raylib C structs (`Texture2D`, `Sound`, `Music`, `Font`, the window) MUST each be owned by exactly
  one C++ object that releases them in its destructor. Owners of such handles MUST explicitly
  `= delete` copy operations and define or delete moves on purpose (rule of five).
- Event subscriptions MUST be RAII objects that unsubscribe in their destructor.
- Gameplay code MUST NOT call `new` or `delete` directly.

**Rationale**: raylib is a C library with no lifetimes of its own. Dangling handles and double
unloads are the main C++ bugs this project is built to eliminate.

### III. Shallow Hierarchy; Variety From Data and State

- Game objects use one shallow tree: `Entity` → `Player`, `Enemy`, `Boss`. Each owns its parts
  (`Animator`, `Collider`, `Health`). Adding a subclass deeper than one level below `Entity` is
  forbidden.
- Variation that is only numbers or sprites MUST be expressed as data (Type Object). Variation in
  behaviour MUST be expressed as State objects, with the current state held in a `unique_ptr` and
  transitions made by returning the successor state.
- Independent boolean flags MUST NOT be used to model mutually exclusive modes (attacking, hurt,
  dead, throwing).
- Gameplay entities MUST announce what happened through typed events on the `EventBus`. They MUST
  NOT include HUD, audio, shop, or wave-director headers.

**Rationale**: Deep hierarchies and flag soup produce impossible states and tight coupling. Data
plus small behaviour objects keep each change local.

### IV. Content Lives in Data

- Enemy types, waves, upgrades, boss stats, and animation specs (frame size, count, fps, pivot,
  hitboxes, active frames) MUST live in JSON under `assets/data/`, loaded with nlohmann/json.
- Rebalancing the game MUST NOT require a recompile. Adding an enemy type that only reuses existing
  behaviour MUST touch only JSON and asset files.
- Hot reload MUST NOT swap data that live objects depend on mid-wave. A spec that adds reload MUST
  state which data is safe to reload and when.
- Malformed data MUST fail loudly at load time with the file and field named, never silently
  default.

**Rationale**: Combat feel lives in tuning numbers. Tuning that needs a rebuild slows iteration
enough that it stops happening.

### V. Pixel-Perfect Presentation

- The game renders to a 576×324 off-screen target, scaled to the window by a whole-number factor
  with point filtering.
- Sprites MUST NOT be scaled by a fraction. Left-facing sprites are drawn by flipping (negative
  source width), not with separate art.
- Character pivots are bottom-centre; effect and projectile pivots are centre.
- The screen does not scroll.

**Rationale**: The CraftPix art only looks right at integer scale, and 576×324 is exactly 18
32-pixel tiles wide and matches the Water Pack background.

### VI. Wall Before Pattern

- This is a learning project. A milestone that introduces a pattern MAY first build the naive
  version (the "wall") and so temporarily violate Principles II–IV, but only where the build plan
  names that naive version.
- Every such violation MUST be listed in the spec's Complexity Tracking table and MUST be removed
  by the milestone that introduces the replacing pattern. Naive code MUST NOT survive past that
  milestone.
- Each milestone's checkpoint (a prediction written down before running it, then a measurement)
  MUST be recorded before the milestone counts as done.

**Rationale**: Patterns stick when you have first felt the problem they solve. Without a deadline,
the wall becomes permanent technical debt.

### VII. Scope Discipline and Simplicity

- Work MUST stay inside the vertical slice in `docs/hold-the-harbor-plan.md`. Anything in its
  "Deliberately not in the slice" list (extra bosses, weather, more stages, scrolling, saving, high
  scores, settings, gamepad, rebinding/Command, Spatial Partition, packaging) is out of scope
  unless this constitution is amended first.
- Prefer the simplest approach that is correct at the slice's scale. Example: collision is an
  all-pairs rectangle-overlap check, which is correct for about 40 objects, so no spatial
  structure is added.
- Input goes through a thin action → key map (`MoveLeft`, `Attack`, `Throw`), with no Command layer.

**Rationale**: A finished, fun slice teaches more than an unfinished ambitious one.

## Technology & Asset Constraints

- **Language & build**: C++23 (`CMAKE_CXX_STANDARD 23`, no extensions), CMake ≥ 3.28, MSVC Build
  Tools 14.52+ as the primary toolchain. Code MUST compile warning-free at `/W4 /permissive-`
  (MSVC) or `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` (others).
- **Dependencies**: raylib, nlohmann/json, and doctest only, pulled through FetchContent and pinned
  to exact tags. A new dependency requires a constitution amendment.
- **Code layout**: game logic lives in the `hth_core` library under `src/`; `src/main.cpp` stays a
  thin entry point; tests live in `tests/` and link `hth_core`.
- **Assets**: PNG-32 images, WAV (16-bit, 44.1 kHz, mono) sound effects, OGG Vorbis (q6) streamed
  music. Source files (PSD, AI, EPS, original WAVs) live in `assets/_source/` and are never loaded
  at runtime. Asset files use snake_case names by role. Only CraftPix pixel art packs, Ovani audio,
  and Pixabay sounds are used, and every imported pack or sound MUST be recorded in
  `docs/asset-credits.md` with its source and license.
- **Placeholders**: a phase uses primitive placeholders until its assets are scheduled in
  `docs/hold-the-harbor-assets.md`. No asset is prepared before its phase starts.

## Development Workflow & Quality Gates

- **Specs**: each milestone or phase is written up as a Spec Kit feature under `specs/`. Every
  `plan.md` MUST pass the Constitution Check against Principles I–VII before design and again
  after it.
- **Tests**: logic that does not need a window (accumulator stepping, state transitions, pool
  handles and generations, JSON loading and validation, event dispatch, wave director) MUST have
  doctest coverage in `hth_tests`. A bug fix SHOULD begin with a failing test when the bug can be
  reproduced without rendering.
- **Gates before a milestone is done**:
  1. A clean build of `hold_the_harbor` and `hth_tests` in Debug and Release, with all tests
     passing.
  2. The milestone's checkpoint prediction and result are recorded.
  3. From Phase 3 onwards, a run with `HTH_ENABLE_ASAN=ON` reports no errors.
- **Slice acceptance**: the slice is done only when every bullet in the plan's "The slice is done
  when" list holds: a flat stress-test frame-time graph, a clean full run under ASan, every number
  rebalanceable without a rebuild, and `Enemy.cpp` free of HUD, audio, shop, and wave-director
  includes.
- **Commits**: small and scoped to one milestone step; the build stays green on `main`.

## Governance

- This constitution overrides informal conventions, and every spec, plan, task list, and review is
  checked against it. Where it conflicts with `docs/hold-the-harbor-plan.md`, the conflict MUST be
  resolved by amending one of the two, never by ignoring it.
- **Amendments**: proposed through `/speckit-constitution`, with the reason and any affected specs
  listed in the Sync Impact Report, and committed separately from feature code.
- **Versioning**: semantic. MAJOR removes or redefines a principle; MINOR adds a principle or
  section or materially expands guidance; PATCH covers clarifications and wording.
- **Compliance**: violations are allowed only when they are justified in a spec's Complexity
  Tracking table (Principle VI gives the standing case). `/speckit-analyze` findings that conflict
  with the constitution are treated as critical.

**Version**: 1.0.0 | **Ratified**: 2026-11-25 | **Last Amended**: 2026-11-25
