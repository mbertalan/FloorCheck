#include "AltimeterCommand.h"
#include "AltimeterConfig.h"
#include "AltimeterLocalSettings.h"
#include "AltimeterRemoteCall.h"
#include "AltimeterSubsystem.h"
#include "FloorCheck.h"

#include "Command/CommandSender.h"
#include "FGPlayerController.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

#define LOCTEXT_NAMESPACE "FloorCheck"

namespace
{
	FString MetersText( float cm )
	{
		return FString::Printf( TEXT( "%.1f m" ), cm / 100.f );
	}

	/** The readout itself shows numbers only, so this reply is where the player is told how to change it. */
	const TCHAR* const SizeHint = TEXT( "Size: '/floorcheck size 70', or the pause menu -> Mods -> Floor Check." );

	FString DescribeZero( const AAltimeterSubsystem* subsystem )
	{
		if( subsystem->HasSiteZero() )
		{
			return FString::Printf( TEXT( "Floor Check zero: site zero at %s above sea level. '/floorcheck sea' returns to sea level. %s" ),
				*MetersText( subsystem->GetSiteZeroCm() ), SizeHint );
		}
		return FString::Printf( TEXT( "Floor Check zero: sea level. Stand on your ground floor and type '/floorcheck zero' to set a site zero. %s" ),
			SizeHint );
	}

	/** The words the player may type for each position, in the order of EFloorCheckPlacement. */
	const TCHAR* const PlacementWords[] = { TEXT( "right" ), TEXT( "left" ), TEXT( "top" ), TEXT( "bottom" ) };

	/** A second spelling of the same four, so "above" and "below" work as well as "top" and "bottom". */
	const TCHAR* const PlacementAliases[] = { TEXT( "east" ), TEXT( "west" ), TEXT( "above" ), TEXT( "below" ) };

	bool ParsePlacement( const FString& word, EFloorCheckPlacement& out_placement )
	{
		for( int32 index = 0; index < static_cast< int32 >( UE_ARRAY_COUNT( PlacementWords ) ); ++index )
		{
			if( word == PlacementWords[ index ] || word == PlacementAliases[ index ] )
			{
				out_placement = static_cast< EFloorCheckPlacement >( index );
				return true;
			}
		}
		return false;
	}

	FString PlacementWord( EFloorCheckPlacement placement )
	{
		const int32 index = static_cast< int32 >( placement );
		return PlacementWords[ index < static_cast< int32 >( UE_ARRAY_COUNT( PlacementWords ) ) ? index : 0 ];
	}
}

AAltimeterCommand::AAltimeterCommand()
{
	CommandName = TEXT( "floorcheck" );
	Aliases.Add( TEXT( "fc" ) );
	Usage = LOCTEXT( "Usage", "floorcheck [zero [metres] | sea | size [percent] | pos [right|left|top|bottom]]  -  show or change what Floor Check measures from, how big the readout is and where it sits" );
	MinNumberOfArguments = 0;
	bOnlyUsableByPlayer = true;
}

EExecutionStatus AAltimeterCommand::ExecuteCommand_Implementation( UCommandSender* Sender, const TArray< FString >& Arguments, const FString& Label )
{
	AAltimeterSubsystem* subsystem = AAltimeterSubsystem::Get( this );
	if( !subsystem )
	{
		Sender->SendChatMessage( TEXT( "Floor Check is not ready yet, try again in a moment." ), FLinearColor::Red );
		return EExecutionStatus::UNCOMPLETED;
	}

	if( Arguments.Num() == 0 )
	{
		Sender->SendChatMessage( DescribeZero( subsystem ) );
		return EExecutionStatus::COMPLETED;
	}

	const FString action = Arguments[ 0 ].ToLower();

	if( action == TEXT( "sea" ) || action == TEXT( "reset" ) || action == TEXT( "clear" ) )
	{
		subsystem->ClearSiteZero();
		Sender->SendChatMessage( TEXT( "Floor Check zero is sea level again." ) );
		return EExecutionStatus::COMPLETED;
	}

	if( action == TEXT( "zero" ) || action == TEXT( "set" ) )
	{
		float zeroCm = 0.f;
		if( Arguments.Num() >= 2 )
		{
			if( !Arguments[ 1 ].IsNumeric() )
			{
				Sender->SendChatMessage( TEXT( "The height must be a number of metres, for example: /floorcheck zero 12.5" ), FLinearColor::Red );
				return EExecutionStatus::BAD_ARGUMENTS;
			}
			zeroCm = FCString::Atof( *Arguments[ 1 ] ) * 100.f;
		}
		else
		{
			AFGPlayerController* playerController = Sender->GetPlayer();
			ACharacter* character = playerController ? Cast< ACharacter >( playerController->GetPawn() ) : nullptr;
			UCapsuleComponent* capsule = character ? character->GetCapsuleComponent() : nullptr;
			if( !character || !capsule )
			{
				Sender->SendChatMessage( TEXT( "Could not find where you are standing. Use '/floorcheck zero <metres>' instead." ), FLinearColor::Red );
				return EExecutionStatus::UNCOMPLETED;
			}
			const float feetCm = static_cast< float >( character->GetActorLocation().Z ) - capsule->GetScaledCapsuleHalfHeight();
			zeroCm = FMath::RoundToFloat( feetCm / FeetRoundingCm ) * FeetRoundingCm;
		}

		subsystem->SetSiteZeroCm( zeroCm );
		Sender->SendChatMessage( FString::Printf( TEXT( "Site zero set: %s above sea level. Readings are now relative to it." ), *MetersText( zeroCm ) ) );
		return EExecutionStatus::COMPLETED;
	}

	if( action == TEXT( "size" ) || action == TEXT( "scale" ) )
	{
		AFGPlayerController* playerController = Sender->GetPlayer();
		if( !playerController )
		{
			Sender->SendChatMessage( TEXT( "Only a player can change the readout size." ), FLinearColor::Red );
			return EExecutionStatus::UNCOMPLETED;
		}

		if( Arguments.Num() < 2 )
		{
			// The size belongs to the player's own machine, so it can only be read back when that machine is this one.
			if( playerController->IsLocalController() )
			{
				Sender->SendChatMessage( FString::Printf(
					TEXT( "Floor Check readout size: %d %% (%d-%d). Type '/floorcheck size 70' to change it, or '/floorcheck size default' to hand it back to the settings page." ),
					UFloorCheckConfig::GetSizePercent( this ), UFloorCheckConfig::MinSizePercent, UFloorCheckConfig::MaxSizePercent ) );
			}
			else
			{
				Sender->SendChatMessage( FString::Printf(
					TEXT( "Floor Check readout size is %d-%d %%. Type '/floorcheck size 70' to set it on your own computer." ),
					UFloorCheckConfig::MinSizePercent, UFloorCheckConfig::MaxSizePercent ) );
			}
			return EExecutionStatus::COMPLETED;
		}

		const FString sizeArgument = Arguments[ 1 ].ToLower();
		const bool bBackToSettingsPage = sizeArgument == TEXT( "default" ) || sizeArgument == TEXT( "auto" ) || sizeArgument == TEXT( "off" );

		if( !bBackToSettingsPage && !sizeArgument.IsNumeric() )
		{
			Sender->SendChatMessage( TEXT( "The size must be a number of percent, for example: /floorcheck size 70" ), FLinearColor::Red );
			return EExecutionStatus::BAD_ARGUMENTS;
		}

		const int32 requested = bBackToSettingsPage ? UFloorCheckLocalSettings::NoSizePercent : FCString::Atoi( *sizeArgument );
		const int32 sizePercent = bBackToSettingsPage
			? UFloorCheckLocalSettings::NoSizePercent
			: FMath::Clamp( requested, UFloorCheckConfig::MinSizePercent, UFloorCheckConfig::MaxSizePercent );

		if( playerController->IsLocalController() )
		{
			// Single player and the player hosting the session: the screen showing the readout is this machine.
			UFloorCheckLocalSettings::SetSizePercent( sizePercent );
		}
		else if( UFloorCheckRemoteCallObject* remoteCall = playerController->GetRemoteCallObjectOfClass< UFloorCheckRemoteCallObject >() )
		{
			remoteCall->ApplyLocalReadoutSize( sizePercent );
		}
		else
		{
			Sender->SendChatMessage( TEXT( "Could not reach your own game to change the size. Use the pause menu -> Mods -> Floor Check instead." ), FLinearColor::Red );
			return EExecutionStatus::UNCOMPLETED;
		}

		if( bBackToSettingsPage )
		{
			Sender->SendChatMessage( TEXT( "Floor Check readout size follows the settings page again." ) );
		}
		else if( requested == sizePercent )
		{
			Sender->SendChatMessage( FString::Printf( TEXT( "Floor Check readout size: %d %%. Saved on this computer." ), sizePercent ) );
		}
		else
		{
			Sender->SendChatMessage( FString::Printf( TEXT( "Floor Check readout size: %d %%, because %d is outside %d-%d." ),
				sizePercent, requested, UFloorCheckConfig::MinSizePercent, UFloorCheckConfig::MaxSizePercent ) );
		}
		return EExecutionStatus::COMPLETED;
	}

	if( action == TEXT( "pos" ) || action == TEXT( "position" ) || action == TEXT( "place" ) )
	{
		if( !AAltimeterSubsystem::bAllowPlacementChoice )
		{
			Sender->SendChatMessage( TEXT( "This build of Floor Check always draws the readout beside the crosshair." ) );
			return EExecutionStatus::COMPLETED;
		}

		AFGPlayerController* playerController = Sender->GetPlayer();
		if( !playerController )
		{
			Sender->SendChatMessage( TEXT( "Only a player can move the readout." ), FLinearColor::Red );
			return EExecutionStatus::UNCOMPLETED;
		}

		if( Arguments.Num() < 2 )
		{
			// Same as the size: the choice lives on the player's own machine and can only be read back there.
			if( playerController->IsLocalController() )
			{
				Sender->SendChatMessage( FString::Printf(
					TEXT( "Floor Check readout is at the %s of the crosshair. Type '/floorcheck pos left', 'right', 'top' or 'bottom' to move it." ),
					*PlacementWord( UFloorCheckLocalSettings::GetPlacement() ) ) );
			}
			else
			{
				Sender->SendChatMessage( TEXT( "Type '/floorcheck pos left', 'right', 'top' or 'bottom' to move the readout on your own computer." ) );
			}
			return EExecutionStatus::COMPLETED;
		}

		EFloorCheckPlacement placement = AAltimeterSubsystem::DefaultPlacement;
		if( !ParsePlacement( Arguments[ 1 ].ToLower(), placement ) )
		{
			Sender->SendChatMessage( TEXT( "The position must be left, right, top or bottom, for example: /floorcheck pos left" ), FLinearColor::Red );
			return EExecutionStatus::BAD_ARGUMENTS;
		}

		if( playerController->IsLocalController() )
		{
			UFloorCheckLocalSettings::SetPlacement( placement );
		}
		else if( UFloorCheckRemoteCallObject* remoteCall = playerController->GetRemoteCallObjectOfClass< UFloorCheckRemoteCallObject >() )
		{
			remoteCall->ApplyLocalReadoutPlacement( static_cast< uint8 >( placement ) );
		}
		else
		{
			Sender->SendChatMessage( TEXT( "Could not reach your own game to move the readout." ), FLinearColor::Red );
			return EExecutionStatus::UNCOMPLETED;
		}

		Sender->SendChatMessage( FString::Printf( TEXT( "Floor Check readout moved to the %s of the crosshair. Saved on this computer." ),
			*PlacementWord( placement ) ) );
		return EExecutionStatus::COMPLETED;
	}

	PrintCommandUsage( Sender );
	return EExecutionStatus::BAD_ARGUMENTS;
}

#undef LOCTEXT_NAMESPACE
