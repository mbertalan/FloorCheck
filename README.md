# Floor Check

A [Satisfactory](https://www.satisfactorygame.com/) mod that tells you how high you are building.

While you hold a **foundation** or a **conveyor lift** in the build gun, a small readout beside the crosshair
shows its height in metres, updated live as you move the hologram. Heights count from sea level, or from a
**site zero** you set yourself, so every floor of a factory lines up.

- Foundations show **Floor top**, the surface you are about to create, and **Floor base**, the surface you are
  stacking on. Zooped columns read to the top of the column; ramps read their high end.
- Conveyor lifts show **Lift start** and **Lift end**.
- `/floorcheck zero` sets the site zero from the floor you are standing on, `/floorcheck zero <metres>` sets it
  explicitly, `/floorcheck sea` goes back to sea level. Alias: `/fc`. The site zero is shared in multiplayer and
  saved with the game.
- Readout size: pause menu → **Mods** → **Floor Check** → *Readout size*, 40 % to 200 %. Saved per machine, so
  every player in a session picks their own.

Unlocked by a Tier 1 HUB milestone called **Floor Check** (10 Iron Rod + 10 Iron Plate, 30 s). It can still be
bought on a save that is already past Tier 1.

## Install

From the [Satisfactory Mod Manager](https://ficsit.app/), or by hand: unzip `FloorCheck-<version>-Windows.zip`
into `<game>\FactoryGame\Mods\GameFeatures\FloorCheck\` so that `FloorCheck.uplugin` sits directly in that folder.
The Satisfactory Mod Loader (SML 3.12+) must be installed first.

## Requirements

Satisfactory 1.2 (game build ≥ 491125), SML 3.12, built against Unreal Engine 5.6.1-CSS.

## Repository layout

| Path | What it is |
|---|---|
| `FloorCheck/` | the mod itself: `Source/FloorCheck/{Public,Private}`, `Config/`, `FloorCheck.uplugin` |
| `PLAN.md` | design and architecture, with the decision table (why each choice was made and how to flip it) |
| `BUILD_STEPS.md` | how to build and package the mod |
| `TEST_PLAN.md` | in-game test checklist, newest version first |
| `CHANGELOG.md` | player-facing changes per version |

## Building

The mod is C++ only and ships no `.uasset` files of its own. Put `FloorCheck/` into a Satisfactory Mod Loader
starter project under `Mods/GameFeatures/FloorCheck/`, build the `FactoryEditor Win64 Development` target, then
package with Alpakit. `BUILD_STEPS.md` has the long version.

## Credits

W@iThere. Built with the Satisfactory modding toolchain and SML.
