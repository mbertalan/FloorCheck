#pragma once

#include "CoreMinimal.h"
#include "Module/GameWorldModule.h"
#include "Module/GameInstanceModule.h"
#include "AltimeterModules.generated.h"

/**
 * Root game-world module: registers the altimeter subsystem, the chat command, the "Floor Check" schematic and,
 * when the editor-made asset exists, the "Surveying" MAM research tree. Without the tree asset the schematic becomes
 * a tier-1 HUB milestone so the mod stays testable.
 */
UCLASS()
class FLOORCHECK_API UAltimeterGameWorldModule : public UGameWorldModule
{
	GENERATED_BODY()
public:
	UAltimeterGameWorldModule();

	/** One-switch flip: true skips the MAM tree entirely and always uses the tier-1 HUB milestone. */
	static constexpr bool bForceHubMilestone = false;

	/** Blueprint-generated class path of the MAM research tree asset inside this plugin's content folder. */
	static constexpr const TCHAR* ResearchTreePath = TEXT( "/FloorCheck/Schematics/Research/ResearchTree_FloorCheck.ResearchTree_FloorCheck_C" );

	virtual void DispatchLifecycleEvent( ELifecyclePhase Phase ) override;
};

/**
 * Root game-instance module: registers the settings page the player uses to size the readout, and the remote
 * call object that carries a size or position typed in chat back to that one player's own machine.
 * It is a separate module because SML keeps mod settings on the game instance, which outlives any single world.
 */
UCLASS()
class FLOORCHECK_API UAltimeterGameInstanceModule : public UGameInstanceModule
{
	GENERATED_BODY()
public:
	UAltimeterGameInstanceModule();

	/** One-switch flip: true drops the settings page and pins the readout to AAltimeterSubsystem::DefaultHudScale. */
	static constexpr bool bDisableSettingsPage = false;

	virtual void DispatchLifecycleEvent( ELifecyclePhase Phase ) override;
};
