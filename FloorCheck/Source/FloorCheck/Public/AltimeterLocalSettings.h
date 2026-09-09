#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AltimeterSubsystem.h"
#include "AltimeterLocalSettings.generated.h"

/**
 * The readout size and position the player set with the chat command, stored on this one machine only.
 *
 * Unreal writes it to <Satisfactory>/FactoryGame/Saved/Config/Windows/Game.ini, next to the save games and the
 * logs, so it survives a restart. It is never replicated and never saved with the world: in multiplayer each
 * player's own machine keeps its own answer, and nothing about it travels to anybody else.
 *
 * The chat command is a second route to the same two settings, deliberately independent of SML's settings page.
 */
UCLASS( Config = Game )
class FLOORCHECK_API UFloorCheckLocalSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	/** One-switch flip: false ignores whatever is stored here and leaves the settings page as the only route. */
	static constexpr bool bAllowChatOverride = true;

	/** Percent, or 0 while the player has never set a size from chat. */
	UPROPERTY( Config )
	int32 ReadoutSizePercent = 0;

	/** Index into EFloorCheckPlacement, or -1 while the player has never set a position from chat. */
	UPROPERTY( Config )
	int32 ReadoutPlacement = -1;

	/** Value to fall back to when nothing is stored, so a caller can tell "not set" from a real choice. */
	static constexpr int32 NoSizePercent = 0;
	static constexpr int32 NoPlacement = -1;

	/** The size stored on this machine in percent, or NoSizePercent when the player never set one. */
	static int32 GetSizePercent();

	/** Stores a size in percent and writes it to disk. NoSizePercent removes it again. */
	static void SetSizePercent( int32 sizePercent );

	/** The position stored on this machine, or the subsystem's default when the player never set one. */
	static EFloorCheckPlacement GetPlacement();

	/** Stores a position and writes it to disk. */
	static void SetPlacement( EFloorCheckPlacement placement );
};
