#include "AltimeterSubsystem.h"
#include "AltimeterConfig.h"
#include "AltimeterLocalSettings.h"
#include "AltimeterWidget.h"
#include "AltimeterContent.h"
#include "FloorCheck.h"

#include "FGCharacterPlayer.h"
#include "FGSchematicManager.h"
#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Hologram/FGHologram.h"
#include "Hologram/FGBuildableHologram.h"
#include "Hologram/FGFoundationHologram.h"
#include "Hologram/FGConveyorLiftHologram.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Buildables/FGBuildableWall.h"
#include "Subsystem/SubsystemActorManager.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "FloorCheck"

namespace
{
	constexpr float CmPerMeter = 100.f;

	float ToMeters( float cm )
	{
		return cm / CmPerMeter;
	}

	/**
	 * Height above sea level (cm) of the top surface of a placed building. False when the building has no
	 * usable surface. Foundations put their origin half way up their height and half way up a ramp's rise;
	 * walls put theirs at the bottom; anything else falls back to the top of its clearance volume.
	 */
	bool GetBuildableTopCm( const AFGBuildable* buildable, float& out_topCm )
	{
		if( !IsValid( buildable ) )
		{
			return false;
		}

		if( const AFGBuildableFoundation* foundation = Cast< AFGBuildableFoundation >( buildable ) )
		{
			out_topCm = static_cast< float >( foundation->GetActorLocation().Z )
				+ foundation->mHeight * 0.5f
				+ foundation->mElevation * 0.5f;
			return true;
		}

		if( const AFGBuildableWall* wall = Cast< AFGBuildableWall >( buildable ) )
		{
			out_topCm = static_cast< float >( wall->GetActorLocation().Z ) + wall->mHeight;
			return true;
		}

		const FBox clearance = buildable->GetCombinedClearanceBox();
		if( clearance.IsValid )
		{
			out_topCm = static_cast< float >( clearance.Max.Z );
			return true;
		}

		return false;
	}
}

AAltimeterSubsystem::AAltimeterSubsystem()
{
	// Server owns the site zero; clients receive a copy so their HUD can use it.
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer_Replicate;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = false;
}

AAltimeterSubsystem* AAltimeterSubsystem::Get( UObject* worldContext )
{
	UWorld* world = worldContext ? worldContext->GetWorld() : nullptr;
	if( !world )
	{
		return nullptr;
	}
	USubsystemActorManager* manager = world->GetSubsystem< USubsystemActorManager >();
	return manager ? manager->GetSubsystemActor< AAltimeterSubsystem >() : nullptr;
}

// ---------------------------------------------------------------------------
// Site zero
// ---------------------------------------------------------------------------

void AAltimeterSubsystem::SetSiteZeroCm( float zeroCm )
{
	if( !HasAuthority() )
	{
		UE_LOG( LogFloorCheck, Warning, TEXT( "SetSiteZeroCm called without authority; ignored" ) );
		return;
	}
	mHasSiteZero = true;
	mSiteZeroCm = zeroCm;
	UE_LOG( LogFloorCheck, Log, TEXT( "Site zero set to %.2f m above sea level" ), ToMeters( zeroCm ) );
}

void AAltimeterSubsystem::ClearSiteZero()
{
	if( !HasAuthority() )
	{
		UE_LOG( LogFloorCheck, Warning, TEXT( "ClearSiteZero called without authority; ignored" ) );
		return;
	}
	mHasSiteZero = false;
	mSiteZeroCm = 0.f;
	UE_LOG( LogFloorCheck, Log, TEXT( "Site zero cleared; readings are above sea level again" ) );
}

// ---------------------------------------------------------------------------
// Readings
// ---------------------------------------------------------------------------

bool AAltimeterSubsystem::FindSnappedTopCm( AFGBuildableHologram* hologram, float& out_topCm ) const
{
	if( !hologram )
	{
		return false;
	}

	// mSnappedFloor and mSnappedWall carry the vertical and side snap results; mSnappedBuilding the common case.
	const AFGBuildable* candidates[] =
	{
		hologram->GetSnappedBuilding(),
		hologram->mSnappedFloor.Get(),
		hologram->mSnappedWall.Get()
	};

	for( const AFGBuildable* candidate : candidates )
	{
		if( GetBuildableTopCm( candidate, out_topCm ) )
		{
			return true;
		}
	}
	return false;
}

bool AAltimeterSubsystem::ReadHologram( const AFGHologram* hologram, FAltimeterReading& out_reading ) const
{
	out_reading = FAltimeterReading();
	if( !IsValid( hologram ) )
	{
		return false;
	}

	const float zeroCm = GetActiveZeroCm();
	const FVector location = hologram->GetActorLocation();

	if( const AFGConveyorLiftHologram* liftHologram = Cast< AFGConveyorLiftHologram >( hologram ) )
	{
		// mTopTransform is the moving end of the lift in the hologram's local space. Holograms only rotate around Z,
		// so the local Z offset is also the world height difference (negative when the lift goes downwards).
		const float startCm = location.Z;
		const float endCm = location.Z + static_cast< float >( liftHologram->mTopTransform.GetTranslation().Z );

		out_reading.PrimaryLabel = LOCTEXT( "LiftEnd", "Lift end" );
		out_reading.PrimaryMeters = ToMeters( endCm - zeroCm );
		out_reading.bHasSecondary = true;
		out_reading.SecondaryLabel = LOCTEXT( "LiftStart", "Lift start" );
		out_reading.SecondaryMeters = ToMeters( startCm - zeroCm );
	}
	else if( Cast< AFGFoundationHologram >( hologram ) )
	{
		// A foundation's origin sits half way up its height and, on a ramp, half way up its rise
		// (see AFGBuildableFoundation::mHeight and ::mElevation), so the walkable surface of its high end
		// is origin + height / 2 + elevation / 2. That is the number builders care about.
		float heightCm = 0.f;
		float elevationCm = 0.f;
		if( const TSubclassOf< AActor > buildClass = hologram->GetBuildClass() )
		{
			if( const AFGBuildableFoundation* foundationDefaults = Cast< AFGBuildableFoundation >( buildClass->GetDefaultObject() ) )
			{
				heightCm = foundationDefaults->mHeight;
				elevationCm = foundationDefaults->mElevation;
			}
		}

		// The snap results and the zoop amount are not reachable through a const hologram.
		AFGBuildableHologram* buildableHologram = const_cast< AFGBuildableHologram* >( Cast< AFGBuildableHologram >( hologram ) );

		// Stacking a column of foundations upwards puts the top-most piece above the hologram's own origin.
		float topCm = static_cast< float >( location.Z );
		if( buildableHologram && buildableHologram->IsInZoopBuildMode() )
		{
			const FVector zoopEnd = buildableHologram->ConvertZoopToWorldLocation( buildableHologram->mDesiredZoop );
			topCm = FMath::Max( topCm, static_cast< float >( zoopEnd.Z ) );
		}

		out_reading.PrimaryLabel = LOCTEXT( "FloorTop", "Floor top" );
		out_reading.PrimaryMeters = ToMeters( topCm + heightCm * 0.5f + elevationCm * 0.5f - zeroCm );

		// Mirrors the lift: the second number is the height being measured from. Absent on free placement.
		float referenceTopCm = 0.f;
		if( bShowFoundationReference && FindSnappedTopCm( buildableHologram, referenceTopCm ) )
		{
			out_reading.bHasSecondary = true;
			out_reading.SecondaryLabel = LOCTEXT( "FloorBase", "Floor base" );
			out_reading.SecondaryMeters = ToMeters( referenceTopCm - zeroCm );
		}
	}
	else if( bShowForAllHolograms && Cast< AFGBuildableHologram >( hologram ) )
	{
		out_reading.PrimaryLabel = LOCTEXT( "Base", "Base" );
		out_reading.PrimaryMeters = ToMeters( location.Z - zeroCm );
	}
	else
	{
		return false;
	}

	out_reading.bRelativeToSiteZero = mHasSiteZero;
	out_reading.bValid = true;
	return true;
}

bool AAltimeterSubsystem::IsUnlockedFor( APlayerController* playerController ) const
{
	if( !bRequireResearch )
	{
		return true;
	}

	const double now = FPlatformTime::Seconds();
	if( mLastResearchCheckSeconds >= 0.0 && now - mLastResearchCheckSeconds < ResearchRecheckSeconds )
	{
		return mCachedUnlocked;
	}
	mLastResearchCheckSeconds = now;

	AFGSchematicManager* schematicManager = AFGSchematicManager::Get( GetWorld() );
	mCachedUnlocked = schematicManager && schematicManager->IsSchematicPurchased( UAltimeterSchematic::StaticClass(), playerController );
	return mCachedUnlocked;
}

// ---------------------------------------------------------------------------
// HUD
// ---------------------------------------------------------------------------

void AAltimeterSubsystem::BeginPlay()
{
	Super::BeginPlay();

	// Dedicated servers have no screen; everything else (single player, listen host, remote client) has a local player.
	if( GetNetMode() != NM_DedicatedServer )
	{
		GetWorldTimerManager().SetTimer( mUpdateTimer, this, &AAltimeterSubsystem::UpdateHud, UpdateIntervalSeconds, true );
		UE_LOG( LogFloorCheck, Log, TEXT( "Altimeter HUD updates started (%s)" ), HasAuthority() ? TEXT( "authority" ) : TEXT( "client" ) );
	}
}

void AAltimeterSubsystem::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	if( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().ClearTimer( mUpdateTimer );
	}
	if( mWidget )
	{
		mWidget->RemoveFromParent();
		mWidget = nullptr;
	}
	Super::EndPlay( endPlayReason );
}

void AAltimeterSubsystem::GetLifetimeReplicatedProps( TArray< FLifetimeProperty >& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( AAltimeterSubsystem, mHasSiteZero );
	DOREPLIFETIME( AAltimeterSubsystem, mSiteZeroCm );
}

AFGHologram* AAltimeterSubsystem::FindLocalHologram( APlayerController*& out_playerController ) const
{
	out_playerController = nullptr;

	UWorld* world = GetWorld();
	if( !world )
	{
		return nullptr;
	}

	APlayerController* playerController = world->GetFirstPlayerController();
	if( !playerController || !playerController->IsLocalController() )
	{
		return nullptr;
	}
	out_playerController = playerController;

	AFGCharacterPlayer* character = Cast< AFGCharacterPlayer >( playerController->GetPawn() );
	if( !character )
	{
		return nullptr;
	}

	AFGBuildGun* buildGun = character->GetBuildGun();
	if( !buildGun || !buildGun->IsInState( EBuildGunState::BGS_BUILD ) )
	{
		return nullptr;
	}

	UFGBuildGunStateBuild* buildState = Cast< UFGBuildGunStateBuild >( buildGun->GetBuildGunStateFor( EBuildGunState::BGS_BUILD ) );
	return buildState ? buildState->GetHologram() : nullptr;
}

UAltimeterWidget* AAltimeterSubsystem::GetOrCreateWidget( APlayerController* playerController )
{
	if( mWidget && mWidget->GetOwningPlayer() == playerController )
	{
		return mWidget;
	}
	if( mWidget )
	{
		mWidget->RemoveFromParent();
		mWidget = nullptr;
	}

	mWidget = CreateWidget< UAltimeterWidget >( playerController, UAltimeterWidget::StaticClass() );
	if( mWidget )
	{
		mWidget->AddToViewport( WidgetZOrder );
		UE_LOG( LogFloorCheck, Log, TEXT( "Altimeter HUD widget created" ) );
	}
	else
	{
		UE_LOG( LogFloorCheck, Error, TEXT( "Altimeter HUD widget could not be created" ) );
	}
	return mWidget;
}

void AAltimeterSubsystem::UpdateHud()
{
	APlayerController* playerController = nullptr;
	AFGHologram* hologram = FindLocalHologram( playerController );
	if( !playerController )
	{
		return;
	}

	FAltimeterReading reading;
	if( hologram && IsUnlockedFor( playerController ) )
	{
		ReadHologram( hologram, reading );
	}

	if( !reading.bValid )
	{
		// Nothing to show: hide an existing widget, but never create one just to hide it.
		if( mWidget )
		{
			mWidget->SetReading( reading );
		}
		return;
	}

	if( UAltimeterWidget* widget = GetOrCreateWidget( playerController ) )
	{
		// Read every update so a change in the mod settings or from chat takes effect without a restart.
		widget->SetUiScale( UFloorCheckConfig::GetHudScale( this ) );
		widget->SetPlacement( UFloorCheckLocalSettings::GetPlacement() );
		widget->SetReading( reading );
	}
}

#undef LOCTEXT_NAMESPACE
