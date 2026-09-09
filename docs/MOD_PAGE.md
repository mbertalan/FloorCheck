# ficsit.app mod page — Floor Check

Paste-ready text for the Floor Check page on ficsit.app. Update it whenever the README changes.

Images are served from this repository, so they keep working on the mod page. Base URL:
`https://raw.githubusercontent.com/mbertalan/FloorCheck/main/docs/screenshots/`

---

## Short description (max 128 characters)

Shows how high you are building, in metres, beside the crosshair while you place foundations and conveyor lifts.

## Long description

Hold a **foundation** or a **conveyor lift** in the build gun and a small readout appears beside the
crosshair, showing its height in metres and updating live as you move the hologram. Heights count from sea
level, or from a **site zero** you set yourself, so every floor of a factory lines up.

![Stacking a foundation, with the readout showing Floor top +4.0 m and Floor base +0.0 m](https://raw.githubusercontent.com/mbertalan/FloorCheck/main/docs/screenshots/01-foundation-height.jpg)

**Foundations** show **Floor top**, the surface you are about to create, and **Floor base**, the surface you
are stacking on. Zooped columns read to the top of the column, ramps read their high end, and the base line
is hidden when you place freely on the ground.

**Conveyor lifts** show **Lift start** and **Lift end**, so you know which floor the lift arrives on before
you place it.

![A conveyor lift, with the readout showing Lift end +7.0 m and Lift start +1.0 m](https://raw.githubusercontent.com/mbertalan/FloorCheck/main/docs/screenshots/02-conveyor-lift-height.jpg)

### Chat commands

`/fc` is a shorter alias for `/floorcheck`.

![The chat command /floorcheck zero and its replies](https://raw.githubusercontent.com/mbertalan/FloorCheck/main/docs/screenshots/03-site-zero-chat.jpg)

- `/floorcheck` — shows what the readout is measuring from right now
- `/floorcheck zero` — sets the site zero from the floor you are standing on
- `/floorcheck zero 12.5` — sets the site zero to an exact height in metres
- `/floorcheck sea` — back to measuring from sea level
- `/floorcheck size 70` — readout size in percent, anywhere from 50 to 200
- `/floorcheck size default` — hands the size back to the settings page
- `/floorcheck pos left` — moves the readout. Also `right`, `top`, `bottom`

The site zero is shared with everyone in the session and saved with the game. Size and position are stored on
your own computer and never sent to anyone else, so every player in a session picks their own. Size can also
be set from the pause menu under **Mods → Floor Check → Readout size**, though the chat command is the sure
route.

### Unlocking it

A Tier 1 HUB milestone called **Floor Check**: 10 Iron Rod + 10 Iron Plate, 30 seconds. It can still be
bought at the HUB terminal on a save that is already past Tier 1.

### Requirements

Satisfactory 1.2 (game build 491125 or newer) and SML 3.12.

Source and changelog: https://github.com/mbertalan/FloorCheck
