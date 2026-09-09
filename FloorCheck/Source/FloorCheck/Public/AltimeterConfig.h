#pragma once

#include "CoreMinimal.h"
#include "Configuration/ModConfiguration.h"
#include "AltimeterSubsystem.h"
#include "AltimeterConfig.generated.h"

/**
 * The mod's settings page, reachable in game from the pause or main menu under Mods -> Floor Check.
 *
 * SML stores it per machine in <Satisfactory>/FactoryGame/Configs/FloorCheck.cfg and writes it back
 * whenever the player moves the slider, so the choice survives a restart and never travels to other players.
 *
 * The same size can be set from chat instead, which is stored elsewhere and wins over this page. See
 * UFloorCheckLocalSettings.
 *
 * The whole page is built in C++: the class default object is given a root section and one property, both
 * instances of SML's own already-cooked editor-widget classes, so the mod ships no assets of its own.
 */
UCLASS()
class FLOORCHECK_API UFloorCheckConfig : public UModConfiguration
{
	GENERATED_BODY()
public:
	/** Key of the size property inside the section, and therefore also its name in FloorCheck.cfg. */
	static constexpr const TCHAR* SizePropertyName = TEXT( "ReadoutSizePercent" );

	/** The slider edits whole percent, so the subsystem's scale range is expressed in percent here. */
	static constexpr int32 DefaultSizePercent = static_cast< int32 >( AAltimeterSubsystem::DefaultHudScale * 100.f + 0.5f );
	static constexpr int32 MinSizePercent = static_cast< int32 >( AAltimeterSubsystem::MinHudScale * 100.f + 0.5f );
	static constexpr int32 MaxSizePercent = static_cast< int32 >( AAltimeterSubsystem::MaxHudScale * 100.f + 0.5f );

	/** Identifies this configuration to SML. The mod reference also names the .cfg file. */
	static FConfigId Id();

	/**
	 * Fills the class default object with the section and the size property. Returns false when SML's property
	 * classes could not be loaded, in which case the configuration must not be registered.
	 * Does nothing on a second call.
	 */
	static bool BuildDefaults();

	/**
	 * The size the readout is actually drawn at on this machine, in percent: the value set from chat when there
	 * is one, otherwise the settings page, otherwise the default.
	 */
	static int32 GetSizePercent( UObject* worldContext );

	/** The same answer as a multiplier, which is what the widget wants. */
	static float GetHudScale( UObject* worldContext );
};
