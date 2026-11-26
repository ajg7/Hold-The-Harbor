# Hold the Harbor — Build Plan

**The game:** You're a pirate captain holding a single pier. Crews come ashore from both sides, and you slash and bomb them. Between waves you spend their gold, and the enemy Captain comes for you last.
**Learning objective:** Learn game programming patterns by first hitting the problem each one solves. Each pattern also drills one modern C++ skill: ownership, RAII, polymorphism, templates, or callbacks.
**Engine / language:** raylib / C++23 — 2D
**Patterns in play:** Game Loop, Update Method, State, Object Pool, Type Object, Observer
**Vertical slice:** One pier, three enemy types across five waves, an upgrade shop between waves, and a Captain boss after wave five. You can win or lose, and it should make you want a second run.

## What you'll have at the end

A single-screen pixel-art brawler, built from the CraftPix pirate packs. The captain has a 4-hit sword combo and throws bombs. Grunts, brutes and bombers come ashore from both ends of the pier, explosions throw them back, and they leave smoke and gold behind. Between waves you buy upgrades with that gold. After wave five, the enemy Captain arrives with two phases, while the Ovani pirate music builds in intensity.

## Architecture decisions

- **How game objects are modeled: a shallow class tree with owned parts** — *your call.* An `Entity` base class has `Player`, `Enemy` and `Boss` underneath it, and each one owns an `Animator`, a `Collider` and a `Health`. There's no deeper hierarchy. Variety comes from data (Type Object) and from behaviour objects (State), not from more subclasses.
- **Update order and timestep** — *recommendation.* Gameplay runs at a fixed 60 Hz using an accumulator, and drawing runs as fast as it can. Each tick runs in one written-down order: input → player → enemies → projectiles → collision resolve → deaths and events → spawner. A fixed order makes bugs reproducible.
- **Ownership** — *recommendation.* The scene owns long-lived objects through `std::unique_ptr`. Short-lived objects (bombs, effects, coins) live by value inside pools and are referenced by handle. Nothing stores a raw pointer to something it doesn't own. raylib's C structs (`Texture2D`, `Sound`, `Music`, `Font`) are owned by exactly one C++ object.
- **Content in data** — *recommendation.* Enemy types, waves, upgrades, boss stats and animation frame specs all live in JSON (nlohmann/json). Tuning the game never requires a recompile.
- **Collision** — *recommendation.* Plain rectangle-overlap checks of every pair. One screen holds about 40 objects at the worst moment, so the all-pairs check stays correct. That's why Spatial Partition is left out.
- **Rendering** — *recommendation.* The game draws to a 576×324 off-screen texture that is scaled up by a whole-number factor to the window: ×2 is 1152×648, ×3 is 1728×972. Point filtering keeps the pixels sharp. 576×324 is 18 of the 32-pixel tiles across and matches the Water Pack background exactly.
- **Input** — *recommendation.* Read keys directly through a thin map from actions (`MoveLeft`, `Attack`, `Throw`) to keys. Command, the pattern that would allow key rebinding and replays, is left out of this version.
- **Tooling** — CMake, building as C++23 (`/std:c++23` on MSVC Build Tools 14.52 or later). raylib, nlohmann/json and doctest are pulled in by CMake's FetchContent. Visual Studio's profiler and AddressSanitizer find performance problems and memory bugs. GitHub Spec Kit manages the specs, with each phase written up as a feature spec in `specs/`.

## Pattern map

| Pattern | Chapter | Where it lands | Why this game needs it |
|---|---|---|---|
| Game Loop | 9 | Phase 1 | Combat timing (combo windows, invincibility frames, knockback) has to be the same at 30 fps and 240 fps |
| Update Method | 10 | Phase 1 | Captain, three enemy types, a boss, bombs and effects all need to update without `main` knowing each one |
| State | 7 | Phase 2 | A 4-hit combo plus hurt, death and throwing breaks a set of booleans immediately. The boss's two phases reuse the same machinery |
| Object Pool | 19 | Phase 3 | Bombs, explosions, smoke, coins and damage numbers spawn and die by the dozen each wave. In C++ the problem is constant `new`/`delete` and dangling pointers |
| Type Object | 13 | Phase 4 | Grunt and Brute differ only in numbers and sprites, and five waves need constant rebalancing |
| Observer | 4 | Phase 5 | One enemy death has to update gold, HUD, sound, screen shake and the wave director. The shop makes the coupling unbearable |

---

## Phase 1 — A Rectangle on a Pier

**Goal:** A block captain fights block pirates on a pier, at the same speed on any machine, and can lose and restart.
**Pattern in focus:** Game Loop and Update Method. These are the backbone patterns, so they come first.

### Milestone 1.1 — The captain moves at the same speed at any frame rate

Set up the CMake project and initialize Spec Kit. Make its constitution the architecture decisions above, so later specs are checked against them. A `Game` class owns the window and the loop: its constructor opens the window, its destructor closes it, and it can't be copied. Draw the pier as a ground rectangle, render to the 576×324 target, and scale that up to the window. The captain is a rectangle you move left and right. Write the movement as "speed × frame time" first, then replace it with a fixed-step accumulator.

- **The wall:** Move a fixed distance per frame and remove the frame-rate cap. The captain sprints. Then switch to variable frame time and drag the window around: one long frame and he tunnels past an enemy.
- **Concept:** Fixed timestep with an accumulator (ch. 9) — gameplay is a simulation, and it must not depend on how fast the machine renders. C++ side: RAII for the window. If construction succeeds, the window exists, and destruction always closes it.
- **Checkpoint:** Before running anything, predict where the captain is after two seconds of holding right at 30, 60 and 240 fps. Cap the frame rate at each, measure, and compare. If the results differ by more than one step's worth of distance, explain why.
- **Assets needed:** Placeholders only.

### Milestone 1.2 — Pirates come ashore from both ends and die to a slash

Pirate blocks spawn on a timer at both pier edges and walk toward the captain. Attacking creates a short-lived hitbox in front of him, and the rectangle-overlap check kills what it touches. First write the loop that checks each object's kind and handles it in `main`. Then move behaviour into an `Entity` base class with virtual `update` and `draw`, owned by the scene in a `std::vector<std::unique_ptr<Entity>>`.

- **The wall:** The type-checking version needs an edit in `main` every time a new kind of object appears.
- **Concept:** Update Method (ch. 10) — each object advances itself. C++ side: virtual dispatch, `unique_ptr` ownership, and safe removal: flag objects dead during the update and erase them after.
- **Checkpoint:** Without looking, explain why erasing an enemy inside the update loop breaks the iteration, and what the erase–remove step does instead. Then do the unsafe erase on purpose and catch it in the debugger.
- **Assets needed:** SFX-01 (the only sound in this phase).

### Milestone 1.3 — You can lose, and you can try again

Add captain health, contact damage, a game-over screen and restart. Introduce `Scene` and a `SceneManager` with Title → Play → GameOver. Restarting destroys the Play scene and builds a fresh one.

- **Concept:** Scene lifetime — what dies on restart, and what must survive it (the `ResourceManager` will, from Phase 2).
- **Checkpoint:** Restart ten times in a row. Predict whether memory use grows, then check Task Manager or the Visual Studio memory graph. Then argue in three sentences why the scene owns the enemy list and the `Game` doesn't.
- **Assets needed:** Placeholders only.

---

## Phase 2 — The Captain Gets a Body

**Goal:** The real pixel captain and pirates, with a 4-hit combo, hurt and death that can never tangle.
**Pattern in focus:** State.

### Milestone 2.1 — Every texture loads once and unloads once

Build the `ResourceManager`. It owns textures by name and hands out `const Texture2D&` references. It can't be copied and is created only after the window exists. Add `SpriteSheet`, `Animation` (frame size, count and speed come from `animations.json`) and `Animator`. The captain idles and walks, grunts walk, the pier is built from the Pirate Bay tiles, and the background is layered sky, water and islands. The screen never scrolls, so the life comes from clouds drifting and water bobbing.

- **The wall:** First load the texture in the `Enemy` constructor: thirty pirates, thirty copies of the same image. Then copy an object that unloads its texture in its destructor, and watch sprites turn into white rectangles once the copy dies.
- **Concept:** RAII and single ownership. raylib is a C library, so its structs need exactly one C++ owner, and the rule-of-five members decide what happens on copy and move.
- **Checkpoint:** Make the `ResourceManager` copyable on purpose, copy it, and predict what draws on the next frame. Then name which rule-of-five members you deleted, and why `= delete` beats "just don't copy it."
- **Assets needed:** SPR-01 (idle, walk), SPR-02 (idle, walk), TIL-01, BG-01.

### Milestone 2.2 — The combo works, until it doesn't

Build combat the naive way, with flags and timers inside `Player::update`. Pressing attack again within a short window chains to the next of four attacks. Taking damage interrupts with hurt and knockback, and running out of health plays death. The bomb throw is a stub. Play it hard and keep a list of every bug you find in `docs/state-bugs.md`.

- **The wall:** You'll swing during hurt stun, keep attacking after death, slide while attacking, and get the combo stuck on step three. That list is the spec for the next milestone.
- **Concept:** Independent flags multiply into impossible states.
- **Checkpoint:** Count your flags. Work out how many combinations they allow, and how many of those are legal. The gap between the two numbers is the case for State.
- **Assets needed:** SPR-01 (attack 1–4, hurt, death), SFX-02, SFX-03.

### Milestone 2.3 — Every bug on the list becomes impossible

Build a `PlayerState` base class with enter, exit, input-handling and update methods. The states are Idle, Walk, Attack (which tracks its combo step), Throw, Hurt and Dead. The current state is held in a `unique_ptr`, and a state changes things by returning its successor. Enemies get Approach, Attack, Hurt and Dead.

- **Concept:** State (ch. 7). C++ side: transferring ownership with `unique_ptr`, and using enter/exit to start and clean up each state's work.
- **Checkpoint:** Replay every bug in `state-bugs.md`. Each one must now be impossible, and for each you can point to the single line that rules it out.
- **Assets needed:** SPR-02 (attack, hurt, death), SFX-04.

---

## Phase 3 — Bombs, Booms and Smoke

**Goal:** Bombs and everything they leave behind, in volume, without stutter or crashes.
**Pattern in focus:** Object Pool.

### Milestone 3.1 — Bombs arc, explode and shake loose gold (the naive version)

The captain throws a bomb that arcs under gravity and has a fuse. It explodes with area damage and knockback, leaves a smoke puff, and dead pirates drop bouncing coins you can pick up. Create every one of these with `std::make_unique` and destroy it when it's finished. Put barrels and an anchor on the pier as scenery.

- **Concept:** Short-lived objects and who owns them. Keep it naive on purpose: this milestone is the "before" picture for the next one.
- **Checkpoint:** Explain how a coin's lifetime differs from a pirate's, and which object should own each.
- **Assets needed:** SPR-03, FX-01, FX-02, FX-03, SPR-04, TIL-02, SFX-05, SFX-06, SFX-07, SFX-08.

### Milestone 3.2 — The stress test: make it hurt

Add a debug key that spawns two hundred pirates and a bomb storm. Profile the run with Visual Studio's Diagnostic Tools (frame time and allocation counts), and run a separate build with AddressSanitizer turned on.

- **The wall:** Thousands of allocations per frame and a frame-time graph full of spikes. Then an explosion that holds a pointer to a pirate killed earlier in the same frame, which ASan reports as heap-use-after-free.
- **Concept:** What heap allocation actually costs in C++ (allocator work, fragmentation, cache misses), and why pointers into objects with their own lifetimes go stale.
- **Checkpoint:** Before the run, sketch the frame-time graph you expect: flat, sawtooth, or spiky. Afterwards, explain the ASan report in three sentences.
- **Assets needed:** None new.

### Milestone 3.3 — `ObjectPool<T>` ends the churn

Write a fixed-capacity template that stores objects by value. A free list runs through the dead slots, and callers get handles (slot index plus generation counter) instead of pointers. Pool bombs, explosions, smoke, coins and floating damage numbers.

- **Concept:** Object Pool (ch. 19). C++ side: class templates, generation counters as a cheap dangling-reference check, and why growing a `std::vector<T>` can invalidate every reference into it.
- **Checkpoint:** Set the explosion pool to 3. Predict the exact failure, then choose a policy on purpose (refuse new ones, or recycle the oldest) and defend it. Rerun the stress test and put the before and after frame-time graphs side by side.
- **Assets needed:** UI-01 (if you want styled damage numbers; otherwise raylib's default font).

---

## Phase 4 — The Pirates Arrive in Waves

**Goal:** Three enemy kinds and five waves, all defined in data, rebalanced while the game is running.
**Pattern in focus:** Type Object.

### Milestone 4.1 — A second and third enemy, the subclass way

Add the Brute (slow, hits hard, resists knockback) and the Bomber (keeps its distance and lobs bombs using the Phase 3 pool) as `Enemy` subclasses.

- **The wall:** The Brute is the Grunt with different numbers and a different sprite. Tuning its health means a rebuild. The Bomber is the only one that actually behaves differently.
- **Concept:** Telling "different data" apart from "different behaviour".
- **Checkpoint:** Diff `Grunt.cpp` and `Brute.cpp`. Every line that differs only in a number belongs in a data file.
- **Assets needed:** SPR-05, SPR-06.

### Milestone 4.2 — `EnemyType` lives in data

Merge Grunt and Brute into one `Enemy` that holds a `const EnemyType&`. Enemy types load at startup from `enemies.json`: health, speed, damage, knockback resistance, gold dropped, sprite keys and a behaviour tag. The behaviour tag selects which state set the enemy uses: melee or thrower. Behaviour stays in code; numbers move to data.

- **Concept:** Type Object (ch. 13). C++ side: `const` shared data, and lifetime — the types are owned by a registry that outlives every enemy.
- **Checkpoint:** Add a throwaway fourth enemy, a reskin of one of the six crew in the platformer pirate pack, end to end. Touch only the JSON and the asset folder. If you had to edit a `.cpp`, the Type Object work didn't land.
- **Assets needed:** None new. The reskin is temporary and not part of the slice.

### Milestone 4.3 — The wave director

`waves.json` lists each wave's spawns: enemy type, count, which side, and delay. The director spawns them, detects when a wave is cleared, and starts the break between waves. A key reloads the JSON while the game runs.

- **Checkpoint:** Rebalance wave 3 while the game is running. First write down which data must *not* reload in the middle of a wave, and why.
- **Assets needed:** MUS-01, SFX-09, SFX-10.

---

## Phase 5 — Gold, Shop and the Captain

**Goal:** The whole loop — waves, gold, shop, boss — with systems that react to each other without knowing about each other.
**Pattern in focus:** Observer.

### Milestone 5.1 — Gold and HUD, wired the obvious way

When a pirate dies, the enemy code directly adds gold, refreshes the HUD, plays a sound, shakes the screen and tells the wave director.

- **The wall:** `Enemy.cpp` now includes `Hud.hpp`, `AudioManager.hpp` and `WaveDirector.hpp`, and the shop will mean opening it again. Draw the include graph and look at it.
- **Concept:** Coupling — how many files you have to open to make one change.
- **Checkpoint:** Count how many headers `Enemy.cpp` includes that have nothing to do with being an enemy. Write that number down; it should be zero by the end of 5.2.
- **Assets needed:** UI-01, UI-02, ICN-02, ICN-03.

### Milestone 5.2 — The enemy just announces that it died

Build an `EventBus` with typed events: `EnemyKilled`, `PlayerHurt`, `WaveCleared`, `GoldChanged`, `BossPhaseChanged`. Listeners subscribe with `std::function` callbacks. Each subscription returns a `Subscription` object, and its destructor unsubscribes the listener.

- **Concept:** Observer (ch. 4). C++ side: lambdas that capture `this`, the listener that's destroyed while still subscribed, and RAII as the fix for it.
- **Checkpoint:** Without the `Subscription` object, destroy the HUD while an event is going out, and predict what happens before you run it. Then explain why the destructor is the right place to unsubscribe.
- **Assets needed:** None new.

### Milestone 5.3 — The shop between waves

A shop panel opens during the break between waves. It offers four to six upgrades from `upgrades.json`, for example: blade damage, bomb capacity, blast radius, max health, a rum heal, and move speed. Each upgrade has a cost and an icon. Buying one fires an event, and the player's stats update in response.

- **Checkpoint:** Add a new upgrade using only JSON and one new subscriber. Count the files you touched.
- **Assets needed:** ICN-01, MUS-02, SFX-11, SFX-12.

### Milestone 5.4 — The Captain comes for you

The boss has his own state machine. In phase 1 he fights with melee combos. Below half health he switches to phase 2: he calls in grunts through the wave director and throws bomb volleys. Then death. His stats come from data, his volleys use the pool, and `BossPhaseChanged` raises the music intensity and shows a boss health bar. This milestone uses every pattern in the plan.

- **Checkpoint:** Sketch the boss's state diagram from memory, then compare it to the code. Any transition in the code that isn't on your sketch is either a bug or missing documentation.
- **Assets needed:** SPR-07, MUS-03, SFX-13, SFX-14.

---

## The slice is done when

- A full run ends in a win or a loss, and both lead cleanly back to a restart.
- Someone who has never played clears wave 1 with no explanation, and loses at least once before beating the Captain.
- The Phase 3 stress test shows a flat frame-time graph, and a full run with AddressSanitizer is clean.
- Every enemy, wave, upgrade and boss number can be rebalanced without a rebuild.
- `Enemy.cpp` includes no HUD, audio, shop or wave-director headers.

## Deliberately not in the slice

- The sea-monster bosses (shark, crab and octopus from Water Monsters) and the other three Captain & Warriors bosses
- Storm waves using the Weather Effects pack (rain, lightning)
- More piers or stages, and anything that scrolls
- Saving, high scores, and a settings menu
- Gamepad support and key rebinding (the Command pattern)
- Spatial Partition, and Data Locality as a profiling exercise
- Packaging the game and publishing it on itch.io
- Ovani's music plugins (they're for Unreal, Unity and Godot; music layering here is done by hand)
