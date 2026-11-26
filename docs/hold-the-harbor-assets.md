# Hold the Harbor — Asset Manifest

**Engine:** raylib (C++23) — 2D
**Art direction:** CraftPix pixel art only, from the itch.io packs you own. The game renders at 576×324 and is scaled up by a whole-number factor (×2 or ×3) with point filtering, so pixels stay sharp. Never scale a sprite by a fraction. Sprites face right, and left-facing is drawn by flipping (a negative source-rectangle width in `DrawTexturePro`). Leave the vector packs out: Parallax Ocean, Underwater World, the 256 px and 512 px pirate icons.
**Audio direction:** Music is the Ovani Pirate Music Pack, using its three intensity levels for shop → waves → boss. Sound effects are short and punchy, from Pixabay plus your Ovani Weaponry, Jingles & Retro packs.

## Placeholder policy

Phase 1 uses primitives only:
- captain: a 16×40 `DARKBLUE` rectangle
- grunts: 16×40 `MAROON` rectangles
- pier: a 576×48 `BROWN` strip
- slash hitbox: a 24×24 half-transparent `YELLOW` rectangle
- sky: a flat `SKYBLUE` clear colour

The only sound is SFX-01. Nothing below gets prepared until its phase starts.

## Conventions for every asset

- **Files:** PNG-32 for all images. raylib can't read PSD, AI or EPS; keep those in `assets/_source/` and never load them.
- **Naming:** rename CraftPix files to snake_case by role, e.g. `captain_attack2.png`, `grunt_walk.png`.
- **Frame sizes:** the itch.io pages don't publish frame sizes; the one known figure is "48 px tall" for Pirate Invasion. Measure each strip once: frame width = strip width ÷ frame count. Record width, height, frame count, frames per second, pivot and hitbox in `assets/data/animations.json`. A small script in `tools/` can do the slicing and write the JSON.
- **Pivot:** bottom-centre for characters, so feet sit on the pier line. Centre for effects and projectiles.
- **No texture atlas in the slice.** raylib has no atlas packer, and about 25 strip textures is well within budget.
- **Audio:** sound effects are WAV, 16-bit, 44.1 kHz, mono, with silence trimmed and peaks around −3 dBFS. Music is converted from Ovani's WAV (48 kHz/24-bit) to OGG Vorbis (stereo, quality 6) and streamed with `LoadMusicStream`.

---

## Phase 1 assets

### SFX
| ID | Name | Spec | Purpose |
|---|---|---|---|
| SFX-01 | Sword slash | WAV mono 44.1 kHz, 0.2–0.3 s, airy whoosh with no impact. Source: Ovani *Weaponry: Melee* or Pixabay | Proves the audio pipeline works; plays on every slash |

---

## Phase 2 assets

### Sprites & sprite sheets
| ID | Name | Spec | Purpose |
|---|---|---|---|
| SPR-01 | Captain (player) | **Pirate Invasion Game Assets**, Captain. Horizontal strips, 48 px tall, frame width measured. Animations: idle, walk, attack 1–4, hurt, death (the sneer can be a taunt later). Starting fps: idle 8, walk 10, attacks 14, hurt 10, death 8. There's no throw animation: reuse the first three frames of attack 1 and release the bomb on frame 3. Pivot bottom-centre. Body hitbox about 16×40, measured. Attack hitboxes active on frames you choose per attack | The player; the combo is what State is built around |
| SPR-02 | Grunt pirate | **Pirate Invasion Game Assets**, pirate crew. 48 px tall strips: idle, walk, attack, hurt, death. Pivot bottom-centre. Same fps as SPR-01 | Basic enemy; the template the Brute gets compared against |

### Tilesets & environments
| ID | Name | Spec | Purpose |
|---|---|---|---|
| TIL-01 | Pier and shore | **Pirate Bay Pixel 2D Tileset**: 32×32 tiles from the 117-tile set (pier planks, posts, stones, stairs, grass edges). The pier is 18 tiles across, with its top at y = 260 in the 576×324 frame. Hand-placed in code or a tiny JSON grid; no Tiled needed | The arena floor |

### Backgrounds & UI
| ID | Name | Spec | Purpose |
|---|---|---|---|
| BG-01 | Harbor backdrop | **Pirate Bay Pixel 2D Tileset**, background layers: sky, clouds, water, islands. Each layer is cropped or tiled to 576×324 with no fractional scaling. Clouds drift at about 4 px/s; water bobs ±1 px on a 1.5 s cycle | The screen doesn't scroll, so movement in the background keeps it alive |

### SFX
| ID | Name | Spec | Purpose |
|---|---|---|---|
| SFX-02 | Blade hit | WAV mono, 0.15–0.3 s, meaty thud plus a short ring, 2–3 variants picked at random. Ovani *Weaponry: Melee* | A connected hit |
| SFX-03 | Captain hurt | WAV mono, 0.3–0.5 s, grunt. Pixabay | Feedback when the captain takes damage |
| SFX-04 | Pirate death | WAV mono, 0.5–0.8 s, cry plus a body thump. Pixabay | An enemy dies |

---

## Phase 3 assets

### Sprites & sprite sheets
| ID | Name | Spec | Purpose |
|---|---|---|---|
| SPR-03 | Bomb | **Bombs and Explosions Asset Pack**: a round bomb from its 20 shell types. One frame (or a fuse flicker if the pack has one), drawn about 12–16 px across, rotating as it arcs. Pivot centre | The thrown projectile |
| SPR-04 | Coin | **Pirate Bay Pixel 2D Tileset**: the animated coin. Spinning loop at about 10 fps; size measured (fits inside a 32×32 cell). Pivot bottom-centre for bouncing | The gold drop |

### Tilesets & environments
| ID | Name | Spec | Purpose |
|---|---|---|---|
| TIL-02 | Pier clutter | **Water Pack Pixel Art**: whole and broken barrels, anchor, chest, a broken-ship piece, on the 32×32 grid. Decoration only, no collision, and kept out of the fighting lanes | Makes the pier read as a harbor |

### Effects
| ID | Name | Spec | Purpose |
|---|---|---|---|
| FX-01 | Explosion | **Bombs and Explosions Asset Pack**: animated explosion sheet. Frames measured, plays once at about 16 fps, pivot centre. Visual radius matches the damage radius in data | The bomb's payoff |
| FX-02 | Smoke puff | **Smoke Effects**: "rising smoke" (the listed sizes are 64×64 and 128×128; confirm which). Plays once, alpha fades in the last 30% | What's left after an explosion |
| FX-03 | Smoke ring | **Smoke Effects**: "smoke ring". Plays once, pivot centre | A poof when a pirate dies or a bomber is caught by an explosion |

### Backgrounds & UI
| ID | Name | Spec | Purpose |
|---|---|---|---|
| UI-01 | Pixel font | **Game UI Pixel Art**: the bundled custom font. Confirm the file is TTF or OTF, since raylib loads TTF, OTF and BMFont. Load at a 16 px base; render at 8 or 16 px in the 576×324 frame. Fallback: raylib's built-in font | Damage numbers now, HUD and shop text in Phase 5 |

### SFX
| ID | Name | Spec | Purpose |
|---|---|---|---|
| SFX-05 | Bomb throw + fuse | WAV mono, 0.3–0.6 s, a whoosh into a hiss. Pixabay | Throw feedback |
| SFX-06 | Explosion | WAV mono, 0.8–1.2 s, punchy low end, 2 variants. Ovani *Weaponry: Ranged* or Pixabay | The explosion; also used for the screen-shake moment |
| SFX-07 | Coin drop | WAV mono, 0.2–0.4 s, metallic bounce. Pixabay | Gold falling |
| SFX-08 | Coin pickup | WAV mono, 0.1–0.3 s, bright chime, pitch nudged up for streaks. Ovani *Retro Gaming* | Collecting gold |

---

## Phase 4 assets

### Sprites & sprite sheets
| ID | Name | Spec | Purpose |
|---|---|---|---|
| SPR-05 | Brute | **Pirates Sprite Sheet for Platformer**: the bulkiest of the six crew. Its itch.io page doesn't list the animations; you need walk, attack, hurt and death as a minimum, so confirm in the zip. Drawn at native size (no scaling), pivot bottom-centre | The slow, heavy enemy type |
| SPR-06 | Bomber | **Bombs and Explosions Asset Pack**: one of its three characters with a throwing animation. Needs idle or walk plus throw. If hurt or death frames are missing, use a white flash plus FX-03 on death. Pivot bottom-centre | The ranged enemy; lobs SPR-03 bombs from the shared pool |

### Music
| ID | Name | Spec | Purpose |
|---|---|---|---|
| MUS-01 | Wave combat theme | **Ovani Pirate Music Pack**, one track at **medium** intensity. OGG stereo q6, looping the whole file | The main track during waves |

### SFX
| ID | Name | Spec | Purpose |
|---|---|---|---|
| SFX-09 | Wave start | WAV mono, 1–2 s, ship's bell or horn. Pixabay | Announces a wave |
| SFX-10 | Wave cleared | WAV stereo, 1.5–3 s, triumphant stinger. Ovani *Jingles & Stingers Vol. 3* | Wave over; the break between waves begins |

---

## Phase 5 assets

### Sprites & sprite sheets
| ID | Name | Spec | Purpose |
|---|---|---|---|
| SPR-07 | The Captain (boss) | **Pirate Captain and Warriors Pixel Art Sprite Pack**: the captain boss of the four. Animations aren't listed on the page; you need idle, walk, at least 2 attacks, hurt and death, so confirm in the zip. Pivot bottom-centre. Should stand visibly taller than SPR-01; if it doesn't, compensate in the staging rather than with non-integer scaling | The final fight |

### Backgrounds & UI
| ID | Name | Spec | Purpose |
|---|---|---|---|
| UI-02 | HUD and shop frames | **Game UI Pixel Art**: window tiles as nine-slice panels (corners fixed, edges and middle stretched) plus buttons. A commenter reports the download may not match the previews, so check the zip first. Fallback: the Pixel Art 2D Game UI pack you own, not yet checked | HUD bar, shop panel, boss health bar frame |

### Icons
| ID | Name | Spec | Purpose |
|---|---|---|---|
| ICN-01 | Upgrade icons | **48 Pirate Skills Pixel Art**, 32×32. Pick 6: saber (blade damage), cannonball (bomb capacity), cannon (blast radius), rum (heal), compass or map (move speed), and one for max health | Shop items |
| ICN-02 | Captain portrait | **Pirate Avatar Pixel Art**, 32×32: the avatar closest to SPR-01 | Next to the HUD health bar |
| ICN-03 | Gold and chest | **48 Chest and Treasure Pixel Art Icons**, 32×32: a coin pile and a chest | Gold counter and shop header |

### Music
| ID | Name | Spec | Purpose |
|---|---|---|---|
| MUS-02 | Shop / break theme | The same Ovani track as MUS-01 at **low** intensity. OGG stereo q6, looped | The calm between waves |
| MUS-03 | Boss theme | The same track at **high** intensity. OGG stereo q6, looped. Crossfades in on `BossPhaseChanged` if the intensity files match in tempo and length; otherwise switch at phase boundaries | The Captain fight |

### SFX
| ID | Name | Spec | Purpose |
|---|---|---|---|
| SFX-11 | Purchase | WAV mono, 0.3–0.6 s, coin-purse jingle. Pixabay | Buying an upgrade |
| SFX-12 | UI click | WAV mono, 0.05–0.15 s, wooden tick. Ovani *Retro Gaming* or Pixabay | Shop navigation |
| SFX-13 | Boss laugh | WAV mono, 1–2 s, deep laugh or roar. Pixabay | The Captain's entrance and his phase 2 |
| SFX-14 | Boss slam | WAV mono, 0.4–0.8 s, heavy impact plus wood creak. Ovani *Weaponry: Melee* | His heaviest attack |

---

## Totals

| Type | Count |
|---|---|
| Sprites | 7 (5 characters: SPR-01, 02, 05, 06, 07; the SPR-04 coin; the SPR-03 bomb) |
| Tilesets / environment | 2 |
| Backgrounds | 1 (layered) |
| Effects | 3 |
| UI (font + frames) | 2 |
| Icon sets | 3 |
| Music | 3 (one Ovani track at three intensities) |
| SFX | 14 |
| **Total line items** | **35** |

Nothing has to be drawn from scratch. The real work is measuring frames, cropping, renaming and picking sounds.

## Sourcing notes

- **Art — all from your itch.io library (CraftPix).** CraftPix's license allows commercial use with no royalties; credit is appreciated but not required. Record every pack in `docs/asset-credits.md` as you import it.
- **Things to check in the zips before each phase starts:**
  - SPR-05 and SPR-07: their animation lists aren't published.
  - SPR-06: whether it has hurt and death frames.
  - UI-01: the font's file format.
  - UI-02: whether the download matches the previews.
  - FX-02: which of the published sizes it is.
- **Music — Ovani (owned).** Royalty-free, including commercial use. Delivered as WAV; convert once with a script in `tools/` and keep the WAVs in `assets/_source/`.
- **SFX — Pixabay plus the Ovani packs you own.** For each Pixabay sound, record its page URL and the license as shown on that page in `asset-credits.md`, so you never have to rediscover where something came from.
- **Worth your own time, because it carries the game's identity:**
  - A "Hold the Harbor" title wordmark drawn in Aseprite.
  - The hitbox and active-frame data in `animations.json`. That data is where the combat feel actually lives, and it's hand-tuning work no pack can do for you.
