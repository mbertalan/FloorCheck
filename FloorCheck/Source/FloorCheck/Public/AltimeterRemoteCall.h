#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "AltimeterRemoteCall.generated.h"

/**
 * Carries a readout size or position from the chat command back to the one player who typed it.
 *
 * SML runs chat commands on the server, but the readout is drawn on the player's own screen and the choice is
 * stored on the player's own machine, so the command hands the value to this object and it travels to that
 * single client. Nothing here is shared with the other players in the session.
 */
UCLASS( NotBlueprintable )
class FLOORCHECK_API UFloorCheckRemoteCallObject : public UFGRemoteCallObject
{
	GENERATED_BODY()
public:
	UFUNCTION( Client, Reliable )
	void ApplyLocalReadoutSize( int32 SizePercent );

	UFUNCTION( Client, Reliable )
	void ApplyLocalReadoutPlacement( uint8 PlacementIndex );

	virtual void GetLifetimeReplicatedProps( TArray< FLifetimeProperty >& OutLifetimeProps ) const override;

private:
	/** A remote call object with nothing replicated is not considered net relevant; SML carries the same field. */
	UPROPERTY( Replicated )
	int32 DummyReplicatedField = 0;
};
