# Floor Check — plan & architecture (Phase 1 doc)

Written 2026-09-09. Target: **Satisfactory 1.2** (game build >= 491125), **SML 3.12**, **Unreal Engine 5.6.1-CSS**.
Working name in the request: "height level mod". Chosen name: **Floor Check** (chosen from a shortlist; ADA line:
"Guessing the height of a floor is not efficient. Checking is."). Mod reference (the technical id, no spaces): `FloorCheck`.
The research is called **Floor Check**. It ships as a **Tier 1 HUB milestone** (decision D11, 2026-09-09); the MAM tab **Surveying** is not planned work any more.

## 1. What the mod does (MVP)

While you hold a **foundation** or a **conveyor lift** in the build gun, a small box under the crosshair shows
its height in metres, updated live as you move or nudge the hologram:

```
      Floor top  12.0 m                      Lift end  +8.0 m
above sea level - /floorcheck zero ...        Lift start  0.0 m
                                             from site zero (sea level +12.0 m) - /floorcheck
```

* **Foundation:** the number is the height of the **top surface** (the floor you will walk on), so an 8x1, 8x2
  and 8x4 foundation placed on the same ground read 1.0 / 2.0 / 4.0 m higher than that ground.
* **Conveyor lift:** two numbers, the fixed **start** (first click) and the moving **end**.
* **Zero:** by default heights are **above sea level** (the game's own Z = 0, see section 2). Optionally you
  stand on your factory's ground floor and type `/floorcheck zero` in chat: from then on every reading is
  relative to that **site zero** and shown with a sign (`+4.0 m`, `-2.0 m`). `/floorcheck sea` goes back.
  The site zero is shared by everyone in the game and saved with the save file.

## 2. Feasibility verdict (focus ladder rung 1)

* **Feasible.** Everything needed exists in the 1.2 headers (verified, section 7): the build gun exposes the
  hologram being placed, a foundation knows its height, the lift hologram knows where its moving end is,
  SML provides world subsystems (saved + replicated state), chat commands and MAM tree registration.
* **The "zero ground" question is already answered by the game:** in Satisfactory's coordinate system
  **Z = 0 is sea level** and the unit is centimetres; the highest terrain is about 488 m above it. So
  "height above sea level in metres" is simply Z / 100 with no calibration needed. The site zero is an
  optional offset on top of that.
* **Hard constraint:** a Satisfactory mod can only be compiled and packaged inside the
  custom Unreal Editor (about 30 GB toolchain: Visual Studio 2022, UE 5.6.1-CSS, Wwise, starter project).
  Nothing in this folder can produce the installable package by itself, and as of 2026-09-09 that toolchain
  is **not installed on this PC**. This session delivers **complete, reviewed C++ source + step-by-step build
  instructions**; packaging happens once `../SETUP_GUIDE.md` has been done (then `BUILD_STEPS.md`).
* **Prior art, heads-up:** the ficsit.app mod "Location and Rotation" (`HologramLocation`) shows the raw X/Y/Z
  position of any hologram. It does not show the floor top, lift start/end, metres, or a site zero, and it has no
  research unlock, so Floor Check is still worth having. It is also a fallback if the toolchain install stalls.

## 3. Design decisions (all one-switch reversible)

| # | Decision | Why | Flip |
|---|---|---|---|
| D1 | Name **Floor Check**, mod reference `FloorCheck`, research node **Floor Check**, MAM tab **Surveying**, chat command `/floorcheck` (alias `/fc`) | readable, no clash found on ficsit.app | strings in `AltimeterContent.cpp`, `AltimeterCommand.cpp`, BUILD_STEPS step 5; the mod reference is fixed once published |
| D2 | Zero = **sea level (world Z = 0)** by default | it is the game's own fixed datum, needs no calibration, matches the request for a map-wide zero | n/a |
| D3 | Optional **site zero** set by chat command, **one per save, shared by all players**, persisted in the save | "am I aligned with my factory floor" needs a factory-specific zero; a chat command needs no editor-made input assets; shared is the simplest multiplayer model | per-player zero later: move `mSiteZeroCm` to the player state; keybinding later: Enhanced Input asset in the editor |
| D4 | Site zero from `/floorcheck zero` = **height of the player's feet, rounded to 10 cm**; `/floorcheck zero 12.5` sets it explicitly | standing on the floor you want as zero is the natural gesture; rounding removes collision jitter | `AAltimeterCommand::FeetRoundingCm` |
| D5 | Foundations show the **top surface** (origin + height / 2); lifts show **start and end** (actor origin and origin + top offset) | the top is where you walk and what you align; the lift's moving end is what you are adjusting | labels/formulas in `AAltimeterSubsystem::ReadHologram` |
| D6 | Only foundations and conveyor lifts show a reading | that is the request; less HUD noise | `AAltimeterSubsystem::bShowForAllHolograms = true` shows the placement point of every building |
| D7 | Readout is gated behind the MAM research **Floor Check** (tier 1, cost 10 Iron Rod + 10 Iron Plate, 30 s, **no dependencies**) in a new MAM tab **Surveying** | request: "in the beginning when people get foundation or the first lift". The MAM itself arrives with Tier 1 "Field Research", which is the same moment foundations (HUB Upgrade 3 / Base Building) and the first lift (HUB Upgrade 4 / Logistics) are in use, so no dependency is needed and the node is buyable the minute the MAM is built | `AAltimeterSubsystem::bRequireResearch = false` makes the HUD work from the first minute |
| D8 | The MAM tree is an **editor-made Blueprint asset** (about 10 clicks, documented); if it is missing the code **falls back** to a plain tier-1 HUB milestone and logs a warning | the tree node data lives in a Blueprint struct C++ cannot build safely; the fallback keeps the mod testable | `UAltimeterGameWorldModule::bForceHubMilestone = true` |
| D9 | HUD is a **C++-built UMG widget** (dark box, three lines) added to the viewport, refreshed 20x per second by a timer in the subsystem | no Blueprint widget = no extra editor steps; a timer is cheaper and simpler than per-frame hooks | later: vanilla-styled widget; `UpdateIntervalSeconds` |
| D10 | Multiplayer-safe from day one: the site zero lives in a **server-owned, replicated subsystem**; the chat command runs on the server (SML behaviour); each machine renders its own HUD from its own build gun | cheap now, painful to retrofit | n/a |
| D11 | The research **stays a Tier 1 HUB milestone**; the MAM tab **Surveying** is dropped from scope (2026-09-09) | the tree asset's node fields are mangled Blueprint-struct fields the headless script cannot fill, and the milestone works: the worry was raised and dismissed that a Tier 3-4 player could not buy an old-tier milestone | `bForceHubMilestone`; if the milestone ever proves unreachable, `AAltimeterSubsystem::bRequireResearch = false` or a different `mTechTier` |
| D12 | Reading a lift's moving end needs one private field of the game's lift hologram; done with an SML **Access Transformer** (`Friend`) instead of editing game headers | it is the documented mechanism, keeps the mod self-contained | `Config/AccessTransformers.ini` |

## 4. Data architecture — what is stored where

* **One saved value:** the site zero (`mHasSiteZero`, `mSiteZeroCm`) on `AAltimeterSubsystem`, an SML mod
  subsystem with policy *Spawn on Server, Replicate*. It implements `IFGSaveInterface` (`ShouldSave = true`)
  so the game writes the two `SaveGame` properties into the save file. Server is the source of truth; clients
  only receive a replicated copy.
* **Everything else is derived, never stored:** the reading is recomputed 20 times a second from the local
  player's build gun -> hologram -> (foundation defaults, lift top offset) and the active zero. The widget is a
  pure view of one `FAltimeterReading` struct.
* **Research state** is the game's own purchased-schematics list (`AFGSchematicManager::IsSchematicPurchased`),
  re-checked once per second.

## 5. Runtime flow

1. World starts: SML spawns `AAltimeterSubsystem` (server; replicated to clients), registers the `/floorcheck`
   chat command and the `Floor Check` schematic, plus the `Surveying` tree if the asset exists.
2. On every machine with a screen the subsystem starts a 0.05 s timer.
3. Timer: find the local player -> its build gun -> if in *Build* state, the hologram. If the research is bought
   and the hologram is a foundation or lift, compute the reading; otherwise hide the widget.
4. First valid reading creates the widget and adds it to the viewport; later readings only update text.
5. `/floorcheck zero` (server): reads the sender's feet height, rounds, stores it in the subsystem -> replicates
   to all clients -> every HUD switches to signed "from site zero" numbers on its next refresh.
6. Save game: the subsystem's two properties are written; on load they come back before the timer starts.

## 6. Files produced (interface contract)

Plugin root: `High level/FloorCheck/` (drop-in for `<StarterProject>/Mods/GameFeatures/FloorCheck/`).

```
FloorCheck/
  FloorCheck.uplugin                     (Alpakit-style, SML ^3.12.0, GameVersion >=491125, BuiltInInitialFeatureState Active)
  Config/PluginSettings.ini                  (copy of template)
  Config/AccessTransformers.ini              Friend=(Class="AFGConveyorLiftHologram", FriendClass="AAltimeterSubsystem")
  Config/Alpakit.ini                         (Windows + WindowsServer + LinuxServer targets)
  Source/FloorCheck/FloorCheck.Build.cs
  Source/FloorCheck/Public/FloorCheck.h          FFloorCheckModule + LogFloorCheck
  Source/FloorCheck/Private/FloorCheck.cpp
  Source/FloorCheck/Public/AltimeterSubsystem.h      struct FAltimeterReading, class AAltimeterSubsystem
  Source/FloorCheck/Private/AltimeterSubsystem.cpp
  Source/FloorCheck/Public/AltimeterWidget.h         class UAltimeterWidget (UUserWidget)
  Source/FloorCheck/Private/AltimeterWidget.cpp
  Source/FloorCheck/Public/AltimeterCommand.h        class AAltimeterCommand (AChatCommandInstance)
  Source/FloorCheck/Private/AltimeterCommand.cpp
  Source/FloorCheck/Public/AltimeterContent.h        UAltimeterUnlockInfo, UAltimeterSchematic
  Source/FloorCheck/Private/AltimeterContent.cpp
  Source/FloorCheck/Public/AltimeterModules.h        UAltimeterGameWorldModule (root)
  Source/FloorCheck/Private/AltimeterModules.cpp
```
Module API macro: `FLOORCHECK_API`. All includes use full paths relative to the FactoryGame `Public/` folder,
e.g. `#include "Equipment/FGBuildGun.h"`, `#include "Hologram/FGConveyorLiftHologram.h"`,
`#include "Buildables/FGBuildableFoundation.h"`, `#include "Subsystem/ModSubsystem.h"`, `#include "Command/ChatCommandInstance.h"`.

### 6.1 `AAltimeterSubsystem` (AModSubsystem + IFGSaveInterface)
* Flags: `bRequireResearch = true`, `bShowForAllHolograms = false`, `UpdateIntervalSeconds = 0.05f`,
  `ResearchRecheckSeconds = 1.0f`, `WidgetZOrder = 50`.
* `static AAltimeterSubsystem* Get(UObject* worldContext)` via `USubsystemActorManager`.
* `HasSiteZero()`, `GetSiteZeroCm()`, `GetActiveZeroCm()`, server-only `SetSiteZeroCm(float)`, `ClearSiteZero()`.
* `bool ReadHologram(const AFGHologram*, FAltimeterReading&) const` — the formulas of D5.
* `bool IsUnlockedFor(APlayerController*) const` — cached schematic check.
* `BeginPlay` starts the timer (not on dedicated servers); `EndPlay` clears it and removes the widget.

### 6.2 `UAltimeterWidget` (UUserWidget)
* `NativeOnInitialized` builds CanvasPanel > Border (anchored to screen centre, 70 px below it) > VerticalBox
  with three TextBlocks (primary 26 pt bold orange, secondary 16 pt, footer 12 pt grey). Starts collapsed.
* `SetReading(const FAltimeterReading&)` formats and shows/hides. Numbers are one decimal; `%+.1f` when a site
  zero is active; never "-0.0".

### 6.3 `AAltimeterCommand` (AChatCommandInstance)
* `CommandName = "altimeter"`, alias `fc`, player-only, 0+ arguments.
* `ExecuteCommand_Implementation`: no args -> describe zero; `sea|reset|clear` -> `ClearSiteZero`;
  `zero|set [metres]` -> explicit metres or feet height (actor Z - capsule half height, rounded to 10 cm).

### 6.4 `AltimeterContent`
* `UAltimeterUnlockInfo : UFGUnlockInfoOnly` (concrete; name + description card shown in the MAM).
* `UAltimeterSchematic : UFGSchematic`: `mType = EST_MAM`, "Floor Check", `mTechTier = 1`, cost Iron Rod 10
  (`/Game/FactoryGame/Resource/Parts/IronRod/Desc_IronRod`) + Iron Plate 10, `mTimeToComplete = 30`, one info unlock,
  no dependencies, icon `TXUI_SIcon_BaseBuilding`. `static ConfigureAsHubMilestone()` flips the CDO to `EST_Milestone`.

### 6.5 `UAltimeterGameWorldModule` (root)
* ctor: `bRootModule = true; mSchematics += UAltimeterSchematic; mChatCommands += AAltimeterCommand; ModSubsystems += AAltimeterSubsystem`.
* `DispatchLifecycleEvent(CONSTRUCTION)`: try `LoadClass<UFGResearchTree>("/FloorCheck/Schematics/Research/ResearchTree_FloorCheck.ResearchTree_FloorCheck_C")`;
  found -> `mResearchTrees.AddUnique`; missing -> HUB-milestone fallback + warning. Then `Super`.

## 7. Verified API facts (from the 1.2 headers in `../_reference/game-headers/`)
* Sea level: "In Satisfactory's coordinate system 0 is sea level, unit is centimetres" (wiki Talk:World; highest
  terrain 488 m). `AFGBuildableFoundation::mHeight` is public with the comment "Origo is assumed to be half way
  between" (FGBuildableFoundation.h:27-29), also `mElevation` for ramps.
* `AFGCharacterPlayer::GetBuildGun()` (FGCharacterPlayer.h:560). `AFGBuildGun::IsInState(EBuildGunState)` (FGBuildGun.h:313),
  `GetBuildGunStateFor(EBuildGunState)` (291), `EBuildGunState::BGS_BUILD` (19-28).
* `UFGBuildGunStateBuild::GetHologram() const` (FGBuildGunBuild.h:205, public). `AFGHologram::GetBuildClass()` (FGHologram.h:271, public),
  `GetActorLocation()` (it is an AActor).
* `AFGConveyorLiftHologram::mTopTransform` — private, "Transform of the top part of the lift, in actor local space"
  (FGConveyorLiftHologram.h:104-105); `GetHeight()` is `|mTopTransform.Z|`. Read through the Friend access transformer
  (format verified against the SML docs example: `Friend=(Class="AFGHologram", FriendClass="UAACopyBuildingsComponent")`).
* `AFGFoundationHologram : AFGFactoryBuildingHologram : AFGBuildableHologram` (FGFoundationHologram.h:13).
* `AFGSchematicManager::Get(UWorld*)` (FGSchematicManager.h:147) and `IsSchematicPurchased(schematic, APlayerController* = nullptr)` (223).
* SML `AModSubsystem` (`ReplicationPolicy` public, `SpawnOnServer_Replicate`; manager calls `SetReplicates(true)`,
  SubsystemActorManager.cpp:30-31). `UGameWorldModule::ModSubsystems`, `mChatCommands`, `mSchematics`, `mResearchTrees`.
  Native root modules are discovered automatically (`FPluginModuleLoader::FindRootModulesOfType` scans native classes with `bRootModule`).
* SML `AChatCommandInstance` (`CommandName`, `Aliases`, `Usage`, `MinNumberOfArguments`, `bOnlyUsableByPlayer`,
  `ExecuteCommand` BlueprintNativeEvent, `PrintCommandUsage`), `UCommandSender::GetPlayer()` -> `AFGPlayerController*`,
  `SendChatMessage(FString, FLinearColor)`. `AChatCommandSubsystem` is *SpawnOnServer*, so commands run server-side.
* `IFGSaveInterface`: `ShouldSave`, `NeedTransform`, `Pre/PostSaveGame`, `Pre/PostLoadGame`, `GatherDependencies` (all BlueprintNativeEvent).
* `UFGUnlockInfoOnly` is abstract with protected `mUnlockName`, `mUnlockDescription`, icons (FGUnlockInfoOnly.h:14-33).
* `UFGSchematic` fields (FGSchematic.h:202-271).
* Milestones (wiki): HUB Upgrade 3 = foundations, HUB Upgrade 4 = Conveyor Lift Mk.1, Tier 1 Field Research = MAM.
  Progression assets `Schematic_1-1/1-2/1-3` exist for a future dependency if wanted.

## 8. Out of scope for the MVP (backlog)
* Keybinding for "set site zero" (needs an Enhanced Input asset made in the editor) and a per-player site zero.
* Vanilla-styled widget, own icon for the research node, localisation.
* Ramp foundations: the number is the origin + height/2, which for a ramp is the middle of the slope, not an edge.
  Fix later with `mElevation` once the exact geometry is checked in game.
* Conveyor lift: the start/end are the hologram origin and its top offset; check in game whether that is the belt
  surface or the base plate and add a constant if needed.
* Showing the height of the floor you are standing on (no hologram) and a per-floor "level number".
* Publishing on ficsit.app (needs an account + icon).
