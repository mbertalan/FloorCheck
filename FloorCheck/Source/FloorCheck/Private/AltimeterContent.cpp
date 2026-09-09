#include "AltimeterContent.h"
#include "FloorCheck.h"

#include "ItemAmount.h"
#include "Resources/FGItemDescriptor.h"
#include "Engine/Texture2D.h"
#include "Math/Vector2D.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "FloorCheck"

namespace
{
	/** Builds an FItemAmount by member assignment so no particular FItemAmount constructor signature is relied on. */
	FItemAmount MakeItemAmount( TSubclassOf< UFGItemDescriptor > itemClass, int32 amount )
	{
		FItemAmount result;
		result.ItemClass = itemClass;
		result.Amount = amount;
		return result;
	}

	/**
	 * Loads a Blueprint-generated class from the game content. FClassFinder expects the asset path without
	 * the ".Name_C" suffix and appends it itself. Only valid while a CDO constructor is running.
	 */
	template< class T >
	TSubclassOf< T > FindGameClass( const TCHAR* assetPath )
	{
		ConstructorHelpers::FClassFinder< T > finder( assetPath );
		if( finder.Succeeded() )
		{
			return finder.Class;
		}
		UE_LOG( LogFloorCheck, Warning, TEXT( "Floor Check content: class not found at '%s'" ), assetPath );
		return nullptr;
	}

	/** Loads a texture from the game content. Only valid while a CDO constructor is running. */
	UTexture2D* FindGameTexture( const TCHAR* objectPath )
	{
		ConstructorHelpers::FObjectFinder< UTexture2D > finder( objectPath );
		if( finder.Succeeded() )
		{
			return finder.Object;
		}
		UE_LOG( LogFloorCheck, Warning, TEXT( "Floor Check content: texture not found at '%s'" ), objectPath );
		return nullptr;
	}

	/** Adds one cost line, skipping items whose descriptor could not be loaded. */
	void AddItemAmount( TArray< FItemAmount >& target, TSubclassOf< UFGItemDescriptor > itemClass, int32 amount )
	{
		if( itemClass )
		{
			target.Add( MakeItemAmount( itemClass, amount ) );
		}
	}

	const TCHAR* const PATH_IRON_ROD = TEXT( "/Game/FactoryGame/Resource/Parts/IronRod/Desc_IronRod" );
	const TCHAR* const PATH_IRON_PLATE = TEXT( "/Game/FactoryGame/Resource/Parts/IronPlate/Desc_IronPlate" );
	const TCHAR* const PATH_SCHEMATIC_ICON = TEXT( "/Game/FactoryGame/Buildable/Factory/TradingPost/UI/SchematicIcons/TXUI_SIcon_BaseBuilding.TXUI_SIcon_BaseBuilding" );
}

// ---------------------------------------------------------------------------
// UAltimeterUnlockInfo
// ---------------------------------------------------------------------------

UAltimeterUnlockInfo::UAltimeterUnlockInfo()
{
	mUnlockName = LOCTEXT( "Unlock_Name", "Floor Check" );
	mUnlockDescription = LOCTEXT( "Unlock_Description",
		"While placing a foundation or a conveyor lift, the build gun shows its height in metres beside the crosshair: the top of the floor and the surface it stacks on, or the start and end of the lift. Heights count from sea level, or from a site zero you set with the chat command /floorcheck zero. The size of the readout is under Mods -> Floor Check in the pause menu." );
	mUnlockIconBig = FindGameTexture( PATH_SCHEMATIC_ICON );
	mUnlockIconSmall = mUnlockIconBig;
}

// ---------------------------------------------------------------------------
// UAltimeterSchematic
// ---------------------------------------------------------------------------

UAltimeterSchematic::UAltimeterSchematic()
{
	mType = ESchematicType::EST_MAM;
	mDisplayName = LOCTEXT( "Schematic_DisplayName", "Floor Check" );
	mDescription = LOCTEXT( "Schematic_Description",
		"Guessing the height of a floor is not efficient. Checking is. Calibrates the build gun's hologram projector against sea level and shows the height of foundations and conveyor lifts in metres while you place them, so every floor of a factory lines up." );
	mTechTier = 1;
	mMenuPriority = 0.0f;
	mTimeToComplete = 30.0f;

	AddItemAmount( mCost, FindGameClass< UFGItemDescriptor >( PATH_IRON_ROD ), 10 );
	AddItemAmount( mCost, FindGameClass< UFGItemDescriptor >( PATH_IRON_PLATE ), 10 );

	if( UAltimeterUnlockInfo* infoUnlock = CreateDefaultSubobject< UAltimeterUnlockInfo >( TEXT( "AltimeterInfoUnlock" ) ) )
	{
		mUnlocks.Add( infoUnlock );
	}

	// No dependencies: the node is visible and buyable from the moment the MAM is built.
	mDependenciesBlocksSchematicAccess = false;
	mHiddenUntilDependenciesMet = false;

	if( UTexture2D* schematicIcon = FindGameTexture( PATH_SCHEMATIC_ICON ) )
	{
		mSchematicIcon.SetResourceObject( schematicIcon );
		mSchematicIcon.SetImageSize( FVector2D( 256.0f, 256.0f ) );
		mSmallSchematicIcon = schematicIcon;
	}
}

void UAltimeterSchematic::ConfigureAsHubMilestone()
{
	UAltimeterSchematic* schematicDefaults = GetMutableDefault< UAltimeterSchematic >();
	if( !schematicDefaults )
	{
		UE_LOG( LogFloorCheck, Error, TEXT( "ConfigureAsHubMilestone: no class default object for UAltimeterSchematic" ) );
		return;
	}

	schematicDefaults->mType = ESchematicType::EST_Milestone;
	UE_LOG( LogFloorCheck, Log, TEXT( "Floor Check schematic configured as a tier %d HUB milestone" ), schematicDefaults->mTechTier );
}

#undef LOCTEXT_NAMESPACE
