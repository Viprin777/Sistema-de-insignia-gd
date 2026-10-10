# Badge System (Geode mod for Geometry Dash)

A badge / achievement collection for Geometry Dash, built with the [Geode SDK](https://geode-sdk.org).
Targets **Geode 5.x** and **Geometry Dash 2.2081**.

## What it does

- 38 badges in 6 tiers (Bronze, Silver, Gold, Platinum, Diamond, Special).
- A popup slides in from the top of the screen when you unlock one (sound optional).
- A **Badges** button on the main menu opens the collection: progress bars, unlock dates,
  and All / Unlocked / Locked tabs. **Reset** wipes all progress (with confirmation).
- If many badges unlock at once (for example the first launch with an old save), you get one
  summary popup instead of a flood.

### Where progress comes from

| Badges | Source |
| --- | --- |
| Levels beaten, flawless runs, practice runs | Tracked by this mod, counted per **different** level (so replaying one level can't farm them). Local/editor levels are ignored. |
| Deaths | Tracked by this mod (one per attempt). |
| Stars, demons, user coins, secret coins, jumps | Read from Geometry Dash's own stat counters, so your existing progress counts immediately. |
| Daily streak | Consecutive calendar days you open the main menu or a level. |
| Night Owl / Early Bird | Beat a level between 00:00-05:00 / 05:00-08:00 local time. |
| Badge Collector / Badge Master | Meta badges for unlocking other badges. |

## Project layout

```
badge-system/
  mod.json              Mod manifest and settings
  CMakeLists.txt        Build script
  logo.png              Mod list icon
  about.md              Text shown on the mod's page in Geode
  changelog.md          Changelog shown in Geode
  .github/workflows/    GitHub Actions build (Windows, macOS, Android)
  src/
    main.cpp            Hooks: main menu button + PlayLayer tracking
    Badges.hpp/.cpp     Badge catalog (add or edit badges here), tiers, colors
    BadgeManager.*      Progress, saving/loading, unlocking, popup queue
    BadgeToast.*        The slide-in unlock popup
    BadgesLayer.*       The Badges screen (list, tabs, reset)
    BadgeUI.*           Medal drawing and small UI helpers
```

## Before you build: personalize it

1. In `mod.json`, change `"id"` (`yourname.badge-system`) and `"developer"` to your own.
   The id must look like `developer.mod-name`, all lowercase.
2. In `mod.json`, make `"geode"` match the Geode version you have installed
   (run `geode sdk version`). It is set to `5.8.0`.

## Build it yourself (Windows / macOS / Linux)

1. Install the [Geode CLI](https://docs.geode-sdk.org/getting-started/geode-cli) and the SDK:
   follow https://docs.geode-sdk.org/getting-started/ (it also covers compilers and CMake).
2. Open a terminal in this folder and run:

```
geode build
```

The result is `yourname.badge-system.geode` in the `build` folder. Drop it into your
`geode/mods` folder (or use "Install from file" in the in-game Geode menu), then restart the game.

## Build with GitHub Actions (no local setup)

1. Create a GitHub repo and push this folder to it.
2. Open the **Actions** tab. The "Build Mod" workflow builds Windows, macOS and Android
   and uploads a combined `.geode` file as an artifact.

## Settings (in the Geode mod settings)

- **Show Unlock Popups**
- **Play Unlock Sound**
- **Show Menu Button**

## Adding or changing badges

Open `src/Badges.cpp` and add a line to `catalog()`:

```cpp
add("my-badge", "My Badge", "Beat 5 different levels", Tier::Silver, Metric::Completions, 5, "GJ_completesIcon_001.png");
```

The icon can be a Geometry Dash sprite frame name (if the frame doesn't exist, the badge's first
letter is shown instead) or one of the drawn glyphs: `@star`, `@arrow`, `@moon`, `@sun`.
Keep the two meta badges at the end of the list.

## Troubleshooting

- **Compile error on the sound line** in `BadgeManager.cpp` (`playEffect(...)`): the audio function
  signature can differ between game versions. Delete that one line, or the "Play Unlock Sound"
  setting simply won't do anything.
- **Compile errors about Geode APIs**: this code was written against Geode 5.x. If you are on an
  older SDK, update with `geode sdk update`.
- **No Badges button on the main menu**: check the "Show Menu Button" setting. The button goes into
  the main menu's bottom bar (or the bottom-right corner if the Node IDs mod isn't installed).
- **Saved progress** lives in the mod's save data (`geode/mods/<mod id>/`) and is written when the game closes.
