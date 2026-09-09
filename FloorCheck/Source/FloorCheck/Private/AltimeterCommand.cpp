#include "AltimeterCommand.h"
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
	const TCHAR* const SizeHint = TEXT( "Readout size: pause menu -> Mods -> Floor Check." );

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
}

AAltimeterCommand::AAltimeterCommand()
{
	CommandName = TEXT( "floorcheck" );
	Aliases.Add( TEXT( "fc" ) );
	Usage = LOCTEXT( "Usage", "floorcheck [zero [metres] | sea]  -  show or set the height Floor Check measures from" );
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

	PrintCommandUsage( Sender );
	return EExecutionStatus::BAD_ARGUMENTS;
}

#undef LOCTEXT_NAMESPACE
