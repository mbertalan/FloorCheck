# Floor Check — test plan

Run after `BUILD_STEPS.md` step 7 (mod shows in the main menu's Mods list). Use a fresh save or a
throw-away copy of an existing one. For each line note **pass / fail** and, on fail, what you saw.
Anything unexpected: keep a screenshot and the last 50 lines of
`%LOCALAPPDATA%/FactoryGame/Saved/Logs/FactoryGame.log`.

## 0. What is new in 1.1.3 (test these first)

The whole of 1.1.3 is the research card, so these four rows are the release. Nothing else was touched.

| # | Do | Expect |
|---|---|---|
| R1 | Open the HUB terminal, tier 1, click **Floor Check** | the **Rewards** panel holds one card named *Floor Check* with an icon on it, not an empty box |
| R2 | Hover or read that card | it describes the height readout and names `/floorcheck zero` |
| R3 | Look at the **Cost** panel on the same screen | two item stacks with icons: **10 Iron Rod** and **10 Iron Plate**. Compare it side by side with the vanilla *Base Building* milestone; it should read the same way |
| R4 | Buy the milestone | it takes exactly those items, finishes in 30 s, and the readout starts working |

If R1 or R3 is still empty, the answer is in the log: open
`%LOCALAPPDATA%/FactoryGame/Saved/Logs/FactoryGame.log` and search for `LogFloorCheck`. One line reads
"Floor Check research card: N cost items, N reward cards" and names anything it could not load. Send that line.

## 0b. What was new in 1.1.2

Before you start, note whether `<Satisfactory>/FactoryGame/Configs/FloorCheck.cfg` already exists. If it does it
still holds `ReadoutSizePercent: 50` from the last build, so rows S1 and S2 below are the ones that prove the fix.

| # | Do | Expect |
|---|---|---|
| S1 | In chat, type `/floorcheck size` | a reply with the size you are on now and the range 50-200 |
| S2 | Type `/floorcheck size 70` while a foundation hologram is up | the readout grows **immediately**, no restart. Reply: "readout size: 70 %. Saved on this computer." |
| S3 | Type `/floorcheck size 500`, then `/floorcheck size 5` | both are pulled back into 50-200 and the reply says so |
| S4 | Type `/floorcheck size abc` | red reply asking for a number; nothing changes |
| S5 | Quit to desktop, start again, take out a foundation | the size from S2 is still there. File: `%LOCALAPPDATA%/FactoryGame/Saved/Config/Windows/Game.ini`, section `[/Script/FloorCheck.FloorCheckLocalSettings]` |
| S6 | Pause menu -> Mods -> Floor Check | the page shows **Readout size** as a vertical list, sitting at 70 % on a fresh install |
| S7 | **The 1.1.1 bug.** Drag the slider on that page from inside a loaded game | it **moves**, and the number follows. If it still refuses, say so - the chat command is then the only route and that is fine |
| S8 | Open the same page from the **main menu**, before loading a save, and drag the slider | it moves. If S7 fails but S8 works, report exactly that: it names the remaining cause in one word |
| S9 | Press *Reset to default* on that page | it goes back to 70 % |
| S10 | After S2, move the slider on the page, then type `/floorcheck size default` and move it again | the first time nothing happens on screen - a size typed in chat wins on purpose; after `default` the slider drives the readout again |
| S11 | Compare the two lines with 1.1.1 from memory | both are clearly bigger, and the smaller second line is now readable rather than borderline |
| S12 | `/floorcheck pos left` | the box jumps to the left of the crosshair, the same distance out, and does not cover the crosshair |
| S13 | `/floorcheck pos top` | the box sits above the crosshair, clear of it |
| S14 | `/floorcheck pos bottom` - then check it three ways: plain foundation, zoop-dragging a row, and a red "can't build here" placement | it must not sit on the game's own hint lines in any of the three. This is the one position that was not proven before shipping |
| S15 | `/floorcheck pos right`, then `/floorcheck size 200` | at maximum size the box still grows away from the crosshair and stays on screen |
| S16 | `/floorcheck pos sideways` | red reply listing left, right, top, bottom; nothing changes |
| S17 | Quit and restart | the position from S12-S15 is still there |
| S18 | Main menu -> Mods, look at the Floor Check entry | author reads **W@ithere**, version **1.1.3** |
| S19 | Multiplayer, if you can: a second player types `/floorcheck size 200` | only **their** readout changes. Yours does not move, and neither does the host's |

## 0c. What was new in 1.1.0

Still worth re-running, but read the numbers as history: the size range is now 50-200 % and the default is 70 %,
so N2, N3 and N5 will not match word for word.

| # | Do | Expect |
|---|---|---|
| N1 | Take out any foundation and look at the readout | the box sits **beside the crosshair**, level with it, and does not cover the game's build hints. Two lines at most, and **no** grey `/floorcheck` line |
| N2 | Compare with 1.0.1 from memory | the text is about **half** the old size |
| N3 | Pause menu -> Mods -> Floor Check | a page with one slider **Readout size**, 40-200 %, sitting at 50 % |
| N4 | Move the slider while a hologram is up | the readout changes size immediately |
| N5 | Quit to desktop, start again, re-check the slider | the value you chose is still there. File: `<Satisfactory>/FactoryGame/Configs/FloorCheck.cfg` |
| N6 | Place a foundation on flat ground, free (nothing to snap to) | **one** line, `Floor top` |
| N7 | Aim a second foundation so it snaps **on top** of the first | **two** lines: `Floor top` (the new surface) and `Floor base` (the one you stack on), 1.0 m apart for 8x1 |
| N8 | From N7, drag the hologram back out over open terrain | the `Floor base` line **disappears**. If it stays and shows a stale height, report it - this is the one thing that could not be proven before the test |
| N9 | Zoop a vertical column of foundations | `Floor top` reads the top of the whole column, not the first one |
| N10 | Take out a ramp | the height reads the **high end** of the slope, not the middle |
| N11 | Type `/floorcheck` in chat | the reply mentions the site zero **and** where to change the readout size |

## A. Loads at all

| # | Do | Expect |
|---|---|---|
| A1 | Start the game, open the log, search `FloorCheck` | lines from SML loading the plugin and `LogFloorCheck: Floor Check module started` |
| A2 | Load a save (or start a new game and reach the world) | log has `Altimeter HUD updates started (authority)`; and either `Registering MAM research tree` (tree asset made) or the warning `MAM research tree asset ... not found; falling back to a tier-1 HUB milestone` (asset skipped). Either is fine |
| A3 | Open chat (Enter), type `/floorcheck` | green reply "Floor Check zero: sea level. ..." |

## B. Research node

| # | Do | Expect |
|---|---|---|
| B1 | Before buying the research: take out a foundation | **no** readout box appears |
| B2 | Tree asset made: build/open the MAM | a tab **Surveying** with one node **Floor Check**, cost 10 Iron Rod + 10 Iron Plate, 30 s |
| B2' | Tree asset skipped: open the HUB terminal, Tier 1 | a milestone **Floor Check** with the same cost |
| B3 | Buy it, wait for the timer | it completes; the node/milestone shows as done |

## C. Foundations

| # | Do | Expect |
|---|---|---|
| C1 | Take out **Foundation 8m x 1m**, aim at flat ground | a dark box under the crosshair: `Floor top  N m` plus a grey line "above sea level - /floorcheck zero to set a site zero". N changes as you move up/down slopes |
| C2 | Place it, then aim a second 8x1 so it snaps **on top** of the first | reading is exactly **1.0 m higher** than for the first one |
| C3 | Same with 8m x 2m and 8m x 4m stacked on the 8x1 | +2.0 m and +4.0 m respectively |
| C4 | Snap a foundation flush next to an existing one (same level) | same number as the existing one would show |
| C5 | Press the nudge keys (default: arrow keys with Ctrl) to move the hologram up/down | the number follows in the game's nudge steps |
| C6 | Zoop a line of foundations | the number stays that of the first one (all in one line share a height) |
| C7 | Take out a wall, machine or conveyor belt | box disappears (only foundations and lifts show; flag `bShowForAllHolograms` changes this) |
| C8 | Put the build gun away / open the build menu | box disappears; reappears when a foundation is held again |
| C9 | Ramp 8x4: note the number | heads-up: for ramps the number is the middle of the slope (known MVP limitation, `PLAN.md` section 8); just note what you see |

## D. Conveyor lift

| # | Do | Expect |
|---|---|---|
| D1 | Take out **Conveyor Lift Mk.1**, aim at a floor before the first click | two lines: `Lift end` and `Lift start`; start is the floor height (write the number down), end is start + the lift's minimum height |
| D2 | Click once, then move the mouse up to raise the lift | `Lift end` increases in the lift's steps (1 m); `Lift start` stays fixed |
| D3 | Reverse the lift with the build-mode key (default R) so it goes down | `Lift end` becomes smaller than `Lift start` |
| D4 | Compare `Lift start` with the `Floor top` reading of the foundation it stands on | ideally equal. If they differ by a constant (e.g. always +1.0 m), write down the difference — it becomes a one-line constant in the code |

## E. Site zero (chat command)

| # | Do | Expect |
|---|---|---|
| E1 | Stand on a foundation, type `/floorcheck zero` | reply "Site zero set: N m above sea level ..." where N is that foundation's `Floor top` reading (within 0.1 m) |
| E2 | Take out a foundation and snap it on top of the one you stand on | reading now shows **`+1.0 m`** (signed) and the grey line says "from site zero (sea level +N m)" |
| E3 | Aim at ground lower than the site zero | negative number, e.g. `-3.5 m` |
| E4 | `/floorcheck zero 20` | site zero is now 20 m above sea level; readings shift accordingly |
| E5 | `/floorcheck zero abc` | red error message, nothing changes |
| E6 | `/floorcheck` | reply describes the current site zero |
| E7 | `/floorcheck sea` | back to unsigned "above sea level" numbers |
| E8 | `/fc zero` | alias works like `/floorcheck zero` |

## F. Save / load

| # | Do | Expect |
|---|---|---|
| F1 | Set a site zero (E1), save, quit to main menu, load the save | `/floorcheck` still reports the same site zero and readings are still signed |
| F2 | `/floorcheck sea`, save, load | back at sea level after loading |

## G. Multiplayer (optional, only if you have a second machine or a friend)

| # | Do | Expect |
|---|---|---|
| G1 | Client joins a host that has the research bought | client sees readouts too |
| G2 | Host types `/floorcheck zero` | within a second the client's readouts become signed with the same site zero |
| G3 | Client types `/floorcheck zero 5` | host's readouts change too (command runs on the server) |

## H. Nothing broke

| # | Do | Expect |
|---|---|---|
| H1 | Build normally for a few minutes (belts, machines, walls) with the mod on | no stutter, no crash; log has no `LogFloorCheck` errors |
| H2 | Disable the mod in Satisfactory Mod Manager and load the same save | game loads fine (the site zero is simply forgotten; that is expected) |
