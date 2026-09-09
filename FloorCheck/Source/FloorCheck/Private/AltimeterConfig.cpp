#include "AltimeterConfig.h"
#include "AltimeterLocalSettings.h"
#include "FloorCheck.h"

#include "Configuration/ConfigManager.h"
#include "Configuration/Properties/ConfigPropertyInteger.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Configuration/Properties/WidgetExtension/CP_Integer.h"
#include "Configuration/Properties/WidgetExtension/CP_Section.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "FloorCheck"

namespace
{
	/**
	 * SML's native property classes draw no editor widget, so the settings page is built from SML's own
	 * Blueprint subclasses. They live inside the SML plugin, which is already a dependency of this mod.
	 */
	const TCHAR* const SectionClassPath = TEXT( "/SML/Interface/UI/Menu/Mods/ConfigProperties/BP_ConfigPropertySection.BP_ConfigPropertySection_C" );
	const TCHAR* const IntegerClassPath = TEXT( "/SML/Interface/UI/Menu/Mods/ConfigProperties/BP_ConfigPropertyInteger.BP_ConfigPropertyInteger_C" );

	const TCHAR* const ModReference = TEXT( "FloorCheck" );

	/** One-switch flip: CPI_Spinbox gives a box the player can both drag and type into instead of a slider. */
	constexpr ECP_IntegerWidgetType SizeWidgetType = ECP_IntegerWidgetType::CPI_Slider;
}

FConfigId UFloorCheckConfig::Id()
{
	FConfigId configId;
	configId.ModReference = ModReference;
	configId.ConfigCategory = FString();
	return configId;
}

bool UFloorCheckConfig::BuildDefaults()
{
	UFloorCheckConfig* configDefaults = GetMutableDefault< UFloorCheckConfig >();
	if( !configDefaults )
	{
		UE_LOG( LogFloorCheck, Error, TEXT( "BuildDefaults: no class default object for UFloorCheckConfig" ) );
		return false;
	}
	if( configDefaults->RootSection )
	{
		return true;
	}

	UClass* sectionClass = LoadClass< UConfigPropertySection >( nullptr, SectionClassPath );
	UClass* integerClass = LoadClass< UConfigPropertyInteger >( nullptr, IntegerClassPath );
	if( !sectionClass || !integerClass )
	{
		UE_LOG( LogFloorCheck, Warning, TEXT( "SML configuration property classes not found; the readout size setting will not be shown" ) );
		return false;
	}

	configDefaults->ConfigId = Id();
	configDefaults->DisplayName = LOCTEXT( "Config_DisplayName", "Floor Check" );
	configDefaults->Description = LOCTEXT( "Config_Description", "Settings for the height readout shown while you place foundations and conveyor lifts." );

	UConfigPropertySection* rootSection = NewObject< UConfigPropertySection >(
		configDefaults, sectionClass, TEXT( "RootSection" ), RF_ArchetypeObject | RF_Public );

	UConfigPropertyInteger* sizeProperty = NewObject< UConfigPropertyInteger >(
		rootSection, integerClass, SizePropertyName, RF_ArchetypeObject | RF_Public );

	// A Blueprint-authored page sets these explicitly; a page built in C++ would otherwise inherit whatever the
	// SML Blueprint default object happens to carry, and a property that requires a world reload is drawn but
	// refuses to be edited outside the main menu.
	rootSection->bRequiresWorldReload = false;
	rootSection->bHidden = false;
	rootSection->bAllowUserReset = true;

	if( UCP_Section* sectionWidget = Cast< UCP_Section >( rootSection ) )
	{
		sectionWidget->WidgetType = ECP_SectionWidgetType::CPS_Vertical;
		sectionWidget->HasHeader = false;
	}

	sizeProperty->DisplayName = LOCTEXT( "Config_SizeName", "Readout size" );
	sizeProperty->Tooltip = LOCTEXT( "Config_SizeTooltip",
		"How big the height readout next to the crosshair is drawn, in percent." );
	sizeProperty->DefaultValue = DefaultSizePercent;
	sizeProperty->Value = DefaultSizePercent;
	sizeProperty->bRequiresWorldReload = false;
	sizeProperty->bHidden = false;
	sizeProperty->bAllowUserReset = true;

	if( UCP_Integer* sizeWidget = Cast< UCP_Integer >( sizeProperty ) )
	{
		sizeWidget->WidgetType = SizeWidgetType;
		sizeWidget->MinValue = MinSizePercent;
		sizeWidget->MaxValue = MaxSizePercent;
	}
	else
	{
		UE_LOG( LogFloorCheck, Warning, TEXT( "SML integer property has no widget extension; the readout size will be drawn as a plain text box" ) );
	}

	rootSection->SectionProperties.Add( SizePropertyName, sizeProperty );
	configDefaults->RootSection = rootSection;

	UE_LOG( LogFloorCheck, Log, TEXT( "Floor Check settings page built (readout size %d-%d%%, default %d%%)" ),
		MinSizePercent, MaxSizePercent, DefaultSizePercent );
	return true;
}

int32 UFloorCheckConfig::GetSizePercent( UObject* worldContext )
{
	// A size set from chat is stored on this machine and wins, so the player has a way in even if the
	// settings page cannot be used.
	const int32 localPercent = UFloorCheckLocalSettings::GetSizePercent();
	if( localPercent > 0 )
	{
		return FMath::Clamp( localPercent, MinSizePercent, MaxSizePercent );
	}

	int32 sizePercent = DefaultSizePercent;

	const UWorld* world = worldContext ? worldContext->GetWorld() : nullptr;
	const UGameInstance* gameInstance = world ? world->GetGameInstance() : nullptr;
	UConfigManager* configManager = gameInstance ? gameInstance->GetSubsystem< UConfigManager >() : nullptr;
	if( configManager )
	{
		if( const UConfigPropertySection* rootSection = configManager->GetConfigurationRootSection( Id() ) )
		{
			UConfigProperty* sizeProperty = rootSection->SectionProperties.FindRef( SizePropertyName );
			if( const UConfigPropertyInteger* sizeInteger = Cast< UConfigPropertyInteger >( sizeProperty ) )
			{
				sizePercent = sizeInteger->Value;
			}
		}
	}

	return FMath::Clamp( sizePercent, MinSizePercent, MaxSizePercent );
}

float UFloorCheckConfig::GetHudScale( UObject* worldContext )
{
	return GetSizePercent( worldContext ) / 100.f;
}

#undef LOCTEXT_NAMESPACE
