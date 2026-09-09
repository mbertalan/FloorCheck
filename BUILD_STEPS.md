# Floor Check — build & install steps

> **On a build machine with no game installed, do NOT follow the click-driven steps below.** Use the headless
> route in `../ENVIRONMENT.md` and the exact commands in `NEXT_SESSION.md` (build, assets, package). This
> file remains the reference for a normal editor-based setup on another PC.

Start here **after** `../SETUP_GUIDE.md` is done, i.e. Visual Studio 2022, Unreal Engine 5.6.1-CSS,
Wwise and the starter project are installed and `FactoryGame.uproject` opens in the editor without
errors. Everything below happens inside that starter project. The flow is the standard
Satisfactory mod flow; the only mod-specific parts are the names and one extra config file.

Placeholders used in this file (replace with the real folders from `../SETUP_GUIDE.md`):

| Placeholder | Meaning | Typical value |
|---|---|---|
| `<StarterProject>` | the folder that contains `FactoryGame.uproject` | `C:/SML` |
| `<Satisfactory>` | the game install folder | `C:/Program Files (x86)/Steam/steamapps/common/Satisfactory` |
| `<ThisFolder>` | this repository folder | wherever you cloned it |

Rule of thumb for the whole file: whenever the editor asks to save something, say yes; whenever a
step fails, keep the exact error text (or a screenshot) — it names the cause far better than guessing
at the C++ does.

---

## Step 1 — Create the mod skeleton with the Alpakit wizard (editor)

1. Open the starter project: double-click `<StarterProject>/FactoryGame.uproject`. First open takes
   a while (shaders). Ignore Wwise warnings.
2. In the top toolbar click the **Alpakit Dev** button (alpaca in a cardboard box). If you can't
   find it: menu `File > Alpakit Dev`.
3. In the Alpakit Dev panel click **Create Mod**.
4. Fill the wizard exactly like this:
   * Template: **C++ & Blueprint** (not "Blueprint Only")
   * Mod Reference (the "Mod Name" field): `FloorCheck` — no spaces, exactly this spelling
   * Display Name / Friendly Name: `Floor Check`
   * Author / Created By: your name (anything, editable later)
   * Leave **Show Content Directory** checked
5. Click **Create Mod**. A Content Browser opens on `FloorCheck Content`.

What this created on disk (check in Explorer):

```text
<StarterProject>/Mods/GameFeatures/FloorCheck/
  FloorCheck.uplugin
  Config/AccessTransformers.ini
  Config/PluginSettings.ini
  Source/FloorCheck/FloorCheck.Build.cs
  Source/FloorCheck/Public/FloorCheck.h
  Source/FloorCheck/Private/FloorCheck.cpp
  Content/FloorCheck.uasset          <- the Game Feature Data asset (see Step 4 if it is missing)
```

**Undo:** close the editor and delete the folder `<StarterProject>/Mods/GameFeatures/FloorCheck`.

---

## Step 2 — Drop in our source and config files

1. **Close the Unreal editor** (File > Exit). Do not skip this; the editor locks files.
2. Copy our code over the generated skeleton. Git Bash, one line
   (it replaces the generated `Source/FloorCheck` folder and the `Config` files):

```bash
rm -rf "<StarterProject>/Mods/GameFeatures/FloorCheck/Source" && cp -r "<ThisFolder>/FloorCheck/Source" "<StarterProject>/Mods/GameFeatures/FloorCheck/" && cp "<ThisFolder>/FloorCheck/Config/"*.ini "<StarterProject>/Mods/GameFeatures/FloorCheck/Config/"
```

   Or by hand in Explorer: delete the generated `Source` folder, then copy
   `<ThisFolder>/FloorCheck/Source` and the three files in `<ThisFolder>/FloorCheck/Config`
   (`PluginSettings.ini`, `AccessTransformers.ini`, `Alpakit.ini`) into
   `<StarterProject>/Mods/GameFeatures/FloorCheck/`, choosing "Replace".

   **This mod needs its `AccessTransformers.ini`** (it lets our code read one private value of the game's
   conveyor-lift hologram). If that file is not copied, the build fails with an error mentioning
   `mTopTransform` being inaccessible. Open the copied file in Notepad and confirm it contains:

```ini
[AccessTransformers]
Friend=(Class="AFGConveyorLiftHologram", FriendClass="AAltimeterSubsystem")
```

3. Keep the **generated** `FloorCheck.uplugin` (it already has the wizard's name/author and the
   `BuiltInInitialFeatureState`). Open it in Notepad and make sure it contains this block; if the
   `Modules` array is missing or the name differs, paste it in just before the final `}`
   (add a comma after the previous entry):

```json
	"Modules": [
		{
			"Name": "FloorCheck",
			"Type": "Runtime",
			"LoadingPhase": "Default"
		}
	],
```

   Also confirm these three lines exist somewhere in the same file:

```json
	"GameVersion": ">=491125",
	"BuiltInInitialFeatureState": "Active",
	"Plugins": [ { "Name": "SML", "Enabled": true, "SemVersion": "^3.12.0" } ]
```

   If in doubt, our reference copy is `<ThisFolder>/FloorCheck/FloorCheck.uplugin`; you can
   replace the generated file with it entirely.
4. Expected final layout:

```text
<StarterProject>/Mods/GameFeatures/FloorCheck/Source/FloorCheck/
  FloorCheck.Build.cs
  Public/FloorCheck.h        Private/FloorCheck.cpp
  Public/AltimeterSubsystem.h    Private/AltimeterSubsystem.cpp
  Public/AltimeterWidget.h       Private/AltimeterWidget.cpp
  Public/AltimeterCommand.h      Private/AltimeterCommand.cpp
  Public/AltimeterContent.h      Private/AltimeterContent.cpp
  Public/AltimeterModules.h      Private/AltimeterModules.cpp
```

**Undo:** delete `<StarterProject>/Mods/GameFeatures/FloorCheck` and redo Step 1.

---

## Step 3 — Compile (Visual Studio)

1. Regenerate the Visual Studio project files so the new module is picked up. Either:
   * Explorer: right-click `<StarterProject>/FactoryGame.uproject` > **Generate Visual Studio
     project files** (Windows 11 hides it under "Show more options"), or
   * PowerShell (the same command `../SETUP_GUIDE.md` uses):

```powershell
& "C:/Program Files/Unreal Engine - CSS/Engine/Build/BatchFiles/Build.bat" -projectfiles -project="<StarterProject>/FactoryGame.uproject" -game -rocket -progress
```

2. Open `<StarterProject>/FactoryGame.sln` in Visual Studio 2022.
3. In the toolbar set Solution Configuration = **Development Editor**, Solution Platform = **Win64**.
4. In Solution Explorer expand `Games`, right-click the **FactoryGame** project > **Build**.
5. Wait. Expected: **5–20 min** for an incremental build. Because of the access transformer the game's
   own hologram headers are regenerated too, so the first build of this mod is a bit longer than usual.
6. Success looks like `========== Build: 1 succeeded, 0 failed ==========` in the Output window.

If it fails, the three most likely causes:

* **Access transformer not applied** — error text like `'mTopTransform': cannot access private member`.
  Check Step 2 item 2, then add or remove a blank line in `AccessTransformers.ini` (the tool only
  re-reads the file when its modified date changes) and build again.
* **A missing include path** — error text like `cannot open source file "Something/FGSomething.h"`.
  The fix is a one-line include change in our code.
* **A renamed or removed game function after a game update** — error text like
  `'GetBuildGunStateFor': is not a member of 'AFGBuildGun'`. Coffee Stain renames things between
  versions; the fix is to look up the new name in the game headers.

In all cases: in the Output window copy the first 20–30 lines that contain `error` (or take a
screenshot of the Error List tab) and paste them to Claude. Do not edit the `.cpp/.h` files by hand.

**Undo:** nothing to undo — the build only writes generated files under
`<StarterProject>/Mods/GameFeatures/FloorCheck/Intermediate` and `Binaries`, which can be
deleted freely.

---

## Step 4 — Verify the mod in the editor

1. Open `<StarterProject>/FactoryGame.uproject` again (the editor was rebuilt, so it may be slow).
2. In any Content Browser click **Settings** (the cog / "View Options" at the bottom right) and
   tick **Show Plugin Content** and **Show C++ Classes**.
3. In the folder tree on the left find **FloorCheck Content** (under "Plugins").
4. Check the Game Feature Data asset:
   * It must exist directly inside `FloorCheck Content` and be named exactly **FloorCheck**
     (asset type "FGGameFeatureData"). If it is missing: right-click in the folder >
     **Miscellaneous > Data Asset** > pick class **FGGameFeatureData** > name it `FloorCheck`.
   * Double-click it. Under **Initial State** click **Edit Plugin** and make sure Initial State is
     **Active**; close that dialog.
   * If the asset's **Current State** is not `Active`, click the **Active** button.
   * Known editor bug: the shown state can lie. Cross-check in `Edit > Plugins`, search
     `FloorCheck`, click its Edit link and confirm "Initial State: Active" there too.
   * Save (Ctrl+S).
5. Check the C++ classes: in the folder tree open **FloorCheck C++ Classes > FloorCheck >
   Public**. You should see `AltimeterSubsystem`, `AltimeterWidget`, `AltimeterCommand`,
   `AltimeterSchematic`, `AltimeterUnlockInfo`, `AltimeterGameWorldModule`. If the folder is empty
   the build did not produce our module — go back to Step 3.

**Undo:** nothing changes on disk except the data asset; deleting it reverts to the wizard state.

---

## Step 5 — Create the MAM research tree asset (optional but recommended)

This is the one piece that has to be an editor-made asset (see `PLAN.md` D8). Without it the mod
still works: the research falls back to a plain **Tier 1 HUB milestone** named "Floor Check"
and the game log gets one warning line from `LogFloorCheck` saying the tree was not found.

1. In the Content Browser, right-click on empty space inside **FloorCheck Content** >
   **Blueprint Class**.
2. In the "Pick Parent Class" dialog expand **All Classes**, search `FGResearchTree`, select it,
   click **Select**.
3. Name the new asset exactly `ResearchTree_FloorCheck` (this name is hard-coded in the C++
   fallback logic; a typo means the fallback kicks in).
4. Double-click the asset to open it. If the Blueprint editor only shows a small "Class Defaults"
   panel, click the **Class Defaults** button in the toolbar. In the Details pane, category
   **Research Tree**, set:
   * **Display Name**: `Surveying`
   * **Pre Unlock Display Name**: `Surveying`
   * **Post Unlock Description**: `Measuring tools for lining up factory floors`
   * **Pre Unlock Description**: same text (optional)
   * **Unlock Dependencies** and **Visibility Dependencies**: leave **empty** — the tab should be
     there the moment the MAM is built.
   * Research Tree Icon can stay empty for the MVP.
5. **Compile** and **Save** (toolbar buttons), then close the Blueprint editor.
6. Add the single research node with SML's tree editor:
   * Right-click `ResearchTree_FloorCheck` in the Content Browser >
     **Scripted Asset Actions > SMLEditor: Open in Research Tree Editor**.
   * **Double-click** an empty grid cell near the top-left to select where the node goes.
   * In any Content Browser (press **Ctrl+Space** for a temporary one) navigate to
     **FloorCheck C++ Classes > FloorCheck > Public** and single-click **AltimeterSchematic** so
     it is selected (Show C++ Classes must be on, Step 4.2).
   * Back in the tree editor click **Set Schematic from Selected**. The node should now show
     "Floor Check".
   * Click the tool's **Save** button (or Ctrl+S with the asset focused). Close the tool.
7. Confirm with `File > Choose Files to Save...` that nothing is left unsaved.

If any part of this refuses to work, skip the step: the automatic HUB-milestone fallback means the
mod is still fully testable, and Claude can look at the error afterwards.

**Undo:** delete the `ResearchTree_FloorCheck` asset in the Content Browser (right-click >
Delete). The code falls back to the HUB milestone automatically.

---

## Step 6 — Package and copy into the game (Alpakit)

One-time settings, in the Alpakit Dev panel under **Dev Packaging Settings > Windows**:

1. Tick **Enabled**.
2. Tick **Copy to Game Path**, click the **...** button and pick the game install folder
   `<Satisfactory>` (the folder that contains `FactoryGame.exe` / `FactoryGameSteam.exe`).
3. Optionally tick **Launch Game Type** and choose the matching entry (Steam or Epic) so Alpakit
   starts the game for you after packaging. Leave the other targets unticked for now.

Then, every time you want a new build in the game:

4. In the mod list find **Floor Check (FloorCheck)** and click its **Alpakit!** button.
5. A popup says the mod is being packaged; open the **Alpakit Log** (three-dots button next to the
   alpaca icon > Alpakit Log) to follow along. First package builds the **Shipping** flavour of the
   mod, so expect **10–30 min**; later ones are faster.
6. Success: the log ends without red lines and this folder now exists in the game:

```text
<Satisfactory>/FactoryGame/Mods/FloorCheck/
  FloorCheck.uplugin
  Binaries/Win64/...
  Content/Paks/...
```

If the log shows errors, copy the last 40 lines of the Alpakit Log and paste them to Claude.

**Undo:** delete `<Satisfactory>/FactoryGame/Mods/FloorCheck`.

---

## Step 7 — Install SML in the game and launch

Only once:

1. Install the **Satisfactory Mod Manager** (SMM) from `https://smm.ficsit.app/` if it is not
   installed yet (run the game to the title screen once before that).
2. In SMM select the game install (Steam/Epic), search **Satisfactory Mod Loader** and click the
   download arrow. That installs SML 3.12 into `<Satisfactory>/FactoryGame/Mods/SML`. Alternative
   without SMM: in Alpakit Dev click **Alpakit!** next to *Satisfactory Mod Loader (SML)*.

Every test run:

3. Launch the game — either the **Launch** button in SMM, normal Steam/Epic launch (mods load
   either way because the files are already in place), or let Alpakit launch it (Step 6.3).
4. Confirm the mod loaded:
   * Main menu > **Mods** button: the list must contain **Floor Check**.
   * Or open the log
     `%LOCALAPPDATA%/FactoryGame/Saved/Logs/FactoryGame.log` in Notepad and search for
     `FloorCheck`: you should see SML loading the plugin and at least one `LogFloorCheck`
     line ("Floor Check module started").
   * If the game says "A version of the 'FloorCheck' plugin has already been enabled": there is
     an old copy in `<Satisfactory>/FactoryGame/Mods/` — delete every `FloorCheck` folder there
     and package again.
5. Continue with `TEST_PLAN.md`.

Handy while testing — add `-log` to the game's launch options (Steam: right-click the game >
Properties > Launch Options) to get a live log window next to the game.

**Undo:** SMM > the trash-can button next to SML removes SML, or **Mods off** in the top-left of SMM
turns all mods off without deleting anything.

---

## Rebuild loop after Claude changes the code

1. Close the editor.
2. Copy the changed files (Step 2, item 2 — the same one-liner works every time).
3. Build Development Editor (Step 3, items 2–5).
4. Open the editor, click **Alpakit!** (Step 6, item 4), launch the game.

Steps 1, 4, 5, and the SML install never need repeating.
