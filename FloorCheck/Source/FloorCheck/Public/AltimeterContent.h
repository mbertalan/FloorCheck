#pragma once

#include "CoreMinimal.h"
#include "Unlocks/FGUnlockInfoOnly.h"
#include "FGSchematic.h"
#include "AltimeterContent.generated.h"

/**
 * The research node's "what you get" card. The game's UFGUnlockInfoOnly is abstract, so a concrete subclass is needed.
 * It unlocks nothing by itself; the HUD checks whether UAltimeterSchematic has been purchased.
 */
UCLASS()
class FLOORCHECK_API UAltimeterUnlockInfo : public UFGUnlockInfoOnly
{
	GENERATED_BODY()
public:
	UAltimeterUnlockInfo();
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
	 * Fallback used when the MAM research tree asset is not available: turns the class default object
	 * into a plain tier-1 HUB milestone so the research is still reachable in game.
	 */
	static void ConfigureAsHubMilestone();
};
