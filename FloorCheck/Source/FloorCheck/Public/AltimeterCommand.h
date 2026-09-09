#pragma once

#include "CoreMinimal.h"
#include "Command/ChatCommandInstance.h"
#include "AltimeterCommand.generated.h"

/**
 * Chat command "/floorcheck" (alias "/fc"): shows or changes the site zero.
 *
 *   /floorcheck              show the zero in use
 *   /floorcheck zero         site zero = the height you are standing on right now
 *   /floorcheck zero 12.5    site zero = 12.5 m above sea level
 *   /floorcheck sea          back to sea level
 *
 * SML runs chat commands on the server, so the change goes straight into the replicated, saved subsystem state.
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
