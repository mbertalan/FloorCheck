#include "AltimeterRemoteCall.h"
#include "AltimeterLocalSettings.h"

#include "Net/UnrealNetwork.h"

void UFloorCheckRemoteCallObject::ApplyLocalReadoutSize_Implementation( int32 SizePercent )
{
	UFloorCheckLocalSettings::SetSizePercent( SizePercent );
}

void UFloorCheckRemoteCallObject::ApplyLocalReadoutPlacement_Implementation( uint8 PlacementIndex )
{
	if( PlacementIndex > static_cast< uint8 >( EFloorCheckPlacement::Below ) )
	{
		return;
	}

	UFloorCheckLocalSettings::SetPlacement( static_cast< EFloorCheckPlacement >( PlacementIndex ) );
}

void UFloorCheckRemoteCallObject::GetLifetimeReplicatedProps( TArray< FLifetimeProperty >& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( UFloorCheckRemoteCallObject, DummyReplicatedField );
}
