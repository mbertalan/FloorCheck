#pragma once

#include "CoreMinimal.h"
#include "Command/ChatCommandInstance.h"
#include "AltimeterCommand.generated.h"

/**
 * Chat command "/floorcheck" (alias "/fc"): shows or changes the site zero, the readout size and where the
 * readout sits.
 *
 *   /floorcheck                  show the zero in use
 *   /floorcheck zero             site zero = the height you are standing on right now
 *   /floorcheck zero 12.5        site zero = 12.5 m above sea level
 *   /floorcheck sea              back to sea level
 *   /floorcheck size             show the readout size
 *   /floorcheck size 70          readout size = 70 %
 *   /floorcheck size default     hand the size back to the settings page
 *   /floorcheck pos              show where the readout sits
 *   /floorcheck pos left         readout to the left of the crosshair (also right, top, bottom)
 *
 * SML runs chat commands on the server, so the site zero goes straight into the replicated, saved subsystem
 * state. Size and position are the opposite: they belong to one screen, so they are handed to the machine of
 * the player who typed the command and stored there.
 */
UCLASS()
class FLOORCHECK_API AAltimeterCommand : public AChatCommandInstance
{
	GENERATED_BODY()
public:
	AAltimeterCommand();

	/** Site zero taken from where the player stands is rounded to this many centimetres (swallows collision jitter). */
	static constexpr float FeetRoundingCm = 10.f;

	virtual EExecutionStatus ExecuteCommand_Implementation( class UCommandSender* Sender, const TArray< FString >& Arguments, const FString& Label ) override;
};
