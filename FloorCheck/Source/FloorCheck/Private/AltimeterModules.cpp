#include "AltimeterModules.h"
#include "AltimeterConfig.h"
#include "AltimeterContent.h"
#include "AltimeterRemoteCall.h"
#include "AltimeterSubsystem.h"
#include "AltimeterCommand.h"
#include "FloorCheck.h"

#include "FGResearchTree.h"
#include "UObject/UObjectGlobals.h"

UAltimeterGameWorldModule::UAltimeterGameWorldModule()
{
	bRootModule = true;
	mSchematics.Add( UAltimeterSchematic::StaticClass() );
	mChatCommands.Add( AAltimeterCommand::StaticClass() );
	ModSubsystems.Add( AAltimeterSubsystem::StaticClass() );
}

void UAltimeterGameWorldModule::DispatchLifecycleEvent( ELifecyclePhase Phase )
{
	// SML registers mSchematics / mResearchTrees during INITIALIZATION, so the tree must be decided at CONSTRUCTION.
	if( Phase == ELifecyclePhase::CONSTRUCTION )
	{
		TSubclassOf< UFGResearchTree > researchTree = nullptr;
		if( !bForceHubMilestone )
		{
			// A missing asset is expected until the tree has been created in the editor; keep the load quiet.
			researchTree = LoadClass< UFGResearchTree >( nullptr, ResearchTreePath, nullptr, LOAD_NoWarn | LOAD_Quiet );
		}

		if( researchTree )
		{
			mResearchTrees.AddUnique( researchTree );
			UE_LOG( LogFloorCheck, Log, TEXT( "Registering MAM research tree '%s'" ), *researchTree->GetPathName() );
		}
		else
		{
			UAltimeterSchematic::ConfigureAsHubMilestone();
			if( bForceHubMilestone )
			{
				UE_LOG( LogFloorCheck, Log, TEXT( "bForceHubMilestone is set: Floor Check is a tier-1 HUB milestone" ) );
			}
			else
			{
				UE_LOG( LogFloorCheck, Warning, TEXT( "MAM research tree asset '%s' not found; falling back to a tier-1 HUB milestone" ), ResearchTreePath );
			}
		}
	}

	Super::DispatchLifecycleEvent( Phase );
}

UAltimeterGameInstanceModule::UAltimeterGameInstanceModule()
{
	bRootModule = true;

	// SML reads RemoteCallObjects during INITIALIZATION, so the class has to be listed in the constructor.
	RemoteCallObjects.Add( UFloorCheckRemoteCallObject::StaticClass() );
}

void UAltimeterGameInstanceModule::DispatchLifecycleEvent( ELifecyclePhase Phase )
{
	// SML reads ModConfigurations during INITIALIZATION, so the page has to be built at CONSTRUCTION.
	// It is only listed once it exists, because registering a configuration without a root section crashes SML.
	if( Phase == ELifecyclePhase::CONSTRUCTION && !bDisableSettingsPage )
	{
		if( UFloorCheckConfig::BuildDefaults() )
		{
			ModConfigurations.AddUnique( UFloorCheckConfig::StaticClass() );
		}
	}

	Super::DispatchLifecycleEvent( Phase );
}
