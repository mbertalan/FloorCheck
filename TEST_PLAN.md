# Floor Check — test plan

Run after `BUILD_STEPS.md` step 7 (mod shows in the main menu's Mods list). Use a fresh save or a
throw-away copy of an existing one. For each line note **pass / fail** and, on fail, what you saw.
Anything unexpected: keep a screenshot and the last 50 lines of
`%LOCALAPPDATA%/FactoryGame/Saved/Logs/FactoryGame.log`.

## 0. What is new in 1.1.0 (test these first)

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
