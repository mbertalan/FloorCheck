#include "AltimeterContent.h"
#include "FloorCheck.h"

#include "ItemAmount.h"
#include "Resources/FGItemDescriptor.h"
#include "Engine/Texture2D.h"
#include "Math/Vector2D.h"
#include "UObject/UnrealType.h"
#include "UObject/TextProperty.h"

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

	/** Loads a Blueprint-generated class from the game content, naming the path in the log when it is not there. */
	template< class T >
	TSubclassOf< T > LoadGameClass( const TCHAR* classPath )
	{
		UClass* loaded = LoadClass< T >( nullptr, classPath );
		if( !loaded )
		{
			UE_LOG( LogFloorCheck, Warning, TEXT( "Floor Check content: class not found at '%s'" ), classPath );
		}
		return loaded;
	}

	/** Loads a texture from the game content, naming the path in the log when it is not there. */
	UTexture2D* LoadGameTexture( const TCHAR* objectPath )
	{
		UTexture2D* loaded = LoadObject< UTexture2D >( nullptr, objectPath );
		if( !loaded )
		{
			UE_LOG( LogFloorCheck, Warning, TEXT( "Floor Check content: texture not found at '%s'" ), objectPath );
		}
		return loaded;
	}

	/** Adds one cost line, skipping items whose descriptor could not be loaded. */
	void AddItemAmount( TArray< FItemAmount >& target, TSubclassOf< UFGItemDescriptor > itemClass, int32 amount )
	{
		if( itemClass )
		{
			target.Add( MakeItemAmount( itemClass, amount ) );
		}
	}

	/**
	 * Writes one field of the reward card by name. The card is an instance of the game's own BP_UnlockInfoOnly
	 * whenever that Blueprint can be loaded, and its fields are protected on the game's class, so they are set by
	 * reflection rather than by member assignment. The names are the C++ field names on UFGUnlockInfoOnly.
	 */
	void SetUnlockText( UObject* unlock, const TCHAR* propertyName, const FText& value )
	{
		if( FTextProperty* property = FindFProperty< FTextProperty >( unlock->GetClass(), propertyName ) )
		{
			property->SetPropertyValue_InContainer( unlock, value );
		}
		else
		{
			UE_LOG( LogFloorCheck, Warning, TEXT( "Floor Check content: '%s' has no text property '%s'" ),
				*unlock->GetClass()->GetName(), propertyName );
		}
	}

	/** As SetUnlockText, for the card's icon fields. */
	void SetUnlockObject( UObject* unlock, const TCHAR* propertyName, UObject* value )
	{
		if( FObjectProperty* property = FindFProperty< FObjectProperty >( unlock->GetClass(), propertyName ) )
		{
			property->SetObjectPropertyValue_InContainer( unlock, value );
		}
		else
		{
			UE_LOG( LogFloorCheck, Warning, TEXT( "Floor Check content: '%s' has no object property '%s'" ),
				*unlock->GetClass()->GetName(), propertyName );
		}
	}

	const TCHAR* const PATH_IRON_ROD = TEXT( "/Game/FactoryGame/Resource/Parts/IronRod/Desc_IronRod.Desc_IronRod_C" );
	const TCHAR* const PATH_IRON_PLATE = TEXT( "/Game/FactoryGame/Resource/Parts/IronPlate/Desc_IronPlate.Desc_IronPlate_C" );
	const TCHAR* const PATH_SCHEMATIC_ICON = TEXT( "/Game/FactoryGame/Buildable/Factory/TradingPost/UI/SchematicIcons/TXUI_SIcon_BaseBuilding.TXUI_SIcon_BaseBuilding" );

	/**
	 * The game's own info-only reward card. The milestone screen builds its reward entries from the Blueprint
	 * side of an unlock, so a card made from this class is drawn while one made from a C++ class alone is not.
	 */
	const TCHAR* const PATH_UNLOCK_INFO = TEXT( "/Game/FactoryGame/Unlocks/BP_UnlockInfoOnly.BP_UnlockInfoOnly_C" );
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

	// No dependencies: the node is visible and buyable from the moment the MAM is built.
	mDependenciesBlocksSchematicAccess = false;
	mHiddenUntilDependenciesMet = false;
}

void UAltimeterSchematic::ConfigureContent()
{
	UAltimeterSchematic* schematicDefaults = GetMutableDefault< UAltimeterSchematic >();
	if( !schematicDefaults )
	{
		UE_LOG( LogFloorCheck, Error, TEXT( "ConfigureContent: no class default object for UAltimeterSchematic" ) );
		return;
	}

	schematicDefaults->mCost.Empty();
	AddItemAmount( schematicDefaults->mCost, LoadGameClass< UFGItemDescriptor >( PATH_IRON_ROD ), 10 );
	AddItemAmount( schematicDefaults->mCost, LoadGameClass< UFGItemDescriptor >( PATH_IRON_PLATE ), 10 );

	UTexture2D* icon = LoadGameTexture( PATH_SCHEMATIC_ICON );
	if( icon )
	{
		schematicDefaults->mSchematicIcon.SetResourceObject( icon );
		schematicDefaults->mSchematicIcon.SetImageSize( FVector2D( 256.0f, 256.0f ) );
		schematicDefaults->mSmallSchematicIcon = icon;
	}

	UClass* unlockClass = LoadClass< UFGUnlock >( nullptr, PATH_UNLOCK_INFO );
	if( !unlockClass )
	{
		UE_LOG( LogFloorCheck, Warning,
			TEXT( "Floor Check content: '%s' not found; the reward card falls back to this mod's own class" ), PATH_UNLOCK_INFO );
		unlockClass = UAltimeterUnlockInfo::StaticClass();
	}

	schematicDefaults->mUnlocks.Empty();
	if( UFGUnlock* infoUnlock = NewObject< UFGUnlock >( schematicDefaults, unlockClass, NAME_None, RF_Public ) )
	{
		SetUnlockText( infoUnlock, TEXT( "mUnlockName" ), LOCTEXT( "Unlock_Name", "Floor Check" ) );
		SetUnlockText( infoUnlock, TEXT( "mUnlockDescription" ), LOCTEXT( "Unlock_Description",
			"While placing a foundation or a conveyor lift, the build gun shows its height in metres beside the crosshair: the top of the floor and the surface it stacks on, or the start and end of the lift. Heights count from sea level, or from a site zero you set with the chat command /floorcheck zero. Type /floorcheck size 70 to make the readout bigger or smaller, and /floorcheck pos left to move it to the other side of the crosshair." ) );
		SetUnlockObject( infoUnlock, TEXT( "mUnlockIconBig" ), icon );
		SetUnlockObject( infoUnlock, TEXT( "mUnlockIconSmall" ), icon );
		SetUnlockObject( infoUnlock, TEXT( "mUnlockIconCategory" ), icon );

		schematicDefaults->mUnlocks.Add( infoUnlock );
	}

	UE_LOG( LogFloorCheck, Log, TEXT( "Floor Check research card: %d cost items, %d reward cards (%s), icon %s" ),
		schematicDefaults->mCost.Num(), schematicDefaults->mUnlocks.Num(),
		*unlockClass->GetName(), icon ? TEXT( "set" ) : TEXT( "MISSING" ) );
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
