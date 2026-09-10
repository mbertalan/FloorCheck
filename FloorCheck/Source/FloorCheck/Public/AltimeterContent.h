#pragma once

#include "CoreMinimal.h"
#include "Unlocks/FGUnlockInfoOnly.h"
#include "FGSchematic.h"
#include "AltimeterContent.generated.h"

/**
 * The research node's "what you get" card, used only when the game's own BP_UnlockInfoOnly cannot be loaded.
 * The game's UFGUnlockInfoOnly is abstract, so a concrete subclass is needed.
 * It unlocks nothing by itself; the HUD checks whether UAltimeterSchematic has been purchased.
 */
UCLASS()
class FLOORCHECK_API UAltimeterUnlockInfo : public UFGUnlockInfoOnly
{
	GENERATED_BODY()
};

/**
 * MAM research node "Floor Check" (tech tier 1). No dependencies: it is available as soon as the MAM exists,
 * i.e. right after the Tier 1 "Field Research" milestone, which is also when foundations and the first lift are in use.
 */
UCLASS()
class FLOORCHECK_API UAltimeterSchematic : public UFGSchematic
{
	GENERATED_BODY()
public:
	UAltimeterSchematic();

	/**
	 * Fills the research card the player reads: the cost items, the reward card and the icons. It runs at the
	 * module's CONSTRUCTION phase rather than in the constructor because the class-default object is built while
	 * the module DLL loads, and in the shipping game the game's own content is not reliably reachable that early.
	 */
	static void ConfigureContent();

	/**
	 * Fallback used when the MAM research tree asset is not available: turns the class default object
	 * into a plain tier-1 HUB milestone so the research is still reachable in game.
	 */
	static void ConfigureAsHubMilestone();
};
