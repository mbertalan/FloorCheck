#include "AltimeterLocalSettings.h"
#include "AltimeterConfig.h"

int32 UFloorCheckLocalSettings::GetSizePercent()
{
	if( !bAllowChatOverride )
	{
		return NoSizePercent;
	}

	const UFloorCheckLocalSettings* settings = GetDefault< UFloorCheckLocalSettings >();
	const int32 stored = settings ? settings->ReadoutSizePercent : NoSizePercent;
	if( stored <= NoSizePercent )
	{
		return NoSizePercent;
	}

	return FMath::Clamp( stored, UFloorCheckConfig::MinSizePercent, UFloorCheckConfig::MaxSizePercent );
}

void UFloorCheckLocalSettings::SetSizePercent( int32 sizePercent )
{
	UFloorCheckLocalSettings* settings = GetMutableDefault< UFloorCheckLocalSettings >();
	if( !settings )
	{
		return;
	}

	settings->ReadoutSizePercent = sizePercent > NoSizePercent
		? FMath::Clamp( sizePercent, UFloorCheckConfig::MinSizePercent, UFloorCheckConfig::MaxSizePercent )
		: NoSizePercent;
	settings->SaveConfig();
}

EFloorCheckPlacement UFloorCheckLocalSettings::GetPlacement()
{
	if( !bAllowChatOverride || !AAltimeterSubsystem::bAllowPlacementChoice )
	{
		return AAltimeterSubsystem::DefaultPlacement;
	}

	const UFloorCheckLocalSettings* settings = GetDefault< UFloorCheckLocalSettings >();
	const int32 stored = settings ? settings->ReadoutPlacement : NoPlacement;
	if( stored < 0 || stored > static_cast< int32 >( EFloorCheckPlacement::Below ) )
	{
		return AAltimeterSubsystem::DefaultPlacement;
	}

	return static_cast< EFloorCheckPlacement >( stored );
}

void UFloorCheckLocalSettings::SetPlacement( EFloorCheckPlacement placement )
{
	UFloorCheckLocalSettings* settings = GetMutableDefault< UFloorCheckLocalSettings >();
	if( !settings )
	{
		return;
	}

	settings->ReadoutPlacement = static_cast< int32 >( placement );
	settings->SaveConfig();
}
