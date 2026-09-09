# Floor Check — changelog

## 1.1.1 (2026-09-09)
- The mod is credited to **W@iThere** in the in-game mod list. No gameplay change.

## 1.1.0 (2026-09-09)
- The readout now sits **beside the crosshair** instead of under it, so it no longer covers the game's own build hints (build mode, zoop count, "can't build here").
- **Half the size** by default. You can change it yourself: pause menu -> Mods -> Floor Check -> Readout size, anywhere from 40% to 200%. The choice is saved on your own machine and survives a restart; in multiplayer everyone picks their own.
- **Stacking foundations now reads like a lift**: "Floor top" is the surface you are about to create, "Floor base" is the surface you are stacking on. The base line is hidden when you place freely on the ground. Vertical stacking (zoop) counts to the top of the whole column, and ramps now report their high end instead of the middle of the slope.
- The research stays a **Tier 1 HUB milestone** called "Floor Check" (10 Iron Rod + 10 Iron Plate, 30 s). You can still buy it at the HUB terminal on an old save that is already past Tier 1.
- The line reminding you about `/floorcheck zero` is gone from the screen. That help now lives in the chat command's own reply (type `/floorcheck`), on the research card, and here.

## 1.0.1 (2026-09-09) — first public build, tested in game and working
- Readout under the crosshair while placing foundations (floor top) and conveyor lifts (start and end), in metres above sea level.
- `/floorcheck zero` (alias `/fc zero`) sets a shared, saved site zero from the floor you stand on; `/floorcheck zero <m>` sets it explicitly; `/floorcheck sea` resets.
- Research "Floor Check" (10 Iron Rod + 10 Iron Plate, 30 s). Appears as a Tier 1 HUB milestone in this build; the MAM tab "Surveying" is planned.
- Known limits: ramps report the middle of the slope; lift readings use the lift's placement point.

## 1.0.0 (2026-09-09) — withdrawn
- Same content as 1.0.1, but the plugin file inside the zip lost its Game Feature flag during a manual version patch. Deleted from ficsit.app.
