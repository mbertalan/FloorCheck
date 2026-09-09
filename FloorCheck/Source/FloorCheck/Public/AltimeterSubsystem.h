#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "Engine/EngineTypes.h"
#include "AltimeterSubsystem.generated.h"

class AFGBuildableHologram;
class AFGHologram;
class APlayerController;
class UAltimeterWidget;

/** Where the readout is drawn relative to the crosshair. The box always grows away from the crosshair. */
UENUM()
enum class EFloorCheckPlacement : uint8
{
	Right UMETA( DisplayName = "Right of crosshair" ),
	Left UMETA( DisplayName = "Left of crosshair" ),
	Above UMETA( DisplayName = "Above crosshair" ),
	Below UMETA( DisplayName = "Below crosshair" )
};

/** One HUD reading for the hologram the local player is currently placing. All heights are metres relative to the active zero. */
USTRUCT( BlueprintType )
struct FLOORCHECK_API FAltimeterReading
{
	GENERATED_BODY()

	/** False when there is nothing to show (no hologram, unsupported building, research not done). */
	UPROPERTY( BlueprintReadOnly )
	bool bValid = false;

	/** What the main number refers to, e.g. "Floor top" or "Lift end". */
	UPROPERTY( BlueprintReadOnly )
	FText PrimaryLabel;

	UPROPERTY( BlueprintReadOnly )
	float PrimaryMeters = 0.f;

	UPROPERTY( BlueprintReadOnly )
	bool bHasSecondary = false;

	/** Second number: the height being measured from, e.g. "Lift start" or "Floor base". */
	UPROPERTY( BlueprintReadOnly )
	FText SecondaryLabel;

	UPROPERTY( BlueprintReadOnly )
	float SecondaryMeters = 0.f;

	/** True while a site zero is set, so numbers are shown with an explicit +/- sign. */
	UPROPERTY( BlueprintReadOnly )
	bool bRelativeToSiteZero = false;
};

/**
 * World subsystem of the Floor Check.
 *
 * Owns the one piece of state the mod has, the optional "site zero" (a height above sea level that
 * readings are measured from). It lives on the server, is replicated to clients and saved with the game.
 *
 * On every machine that has a local player it also drives the HUD: a small timer looks at the build gun,
 * computes a reading for the current hologram and pushes it into UAltimeterWidget.
 *
 * Sea level is world Z = 0 in Satisfactory, so "above sea level" is simply Z / 100.
 */
UCLASS()
class FLOORCHECK_API AAltimeterSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()
public:
	AAltimeterSubsystem();

	// ---- One-switch flags -------------------------------------------------------------------

	/** false: the HUD works from the first minute without buying the MAM research. */
	static constexpr bool bRequireResearch = true;

	/** true: every building type shows the height of its placement point, not only foundations and lifts. */
	static constexpr bool bShowForAllHolograms = false;

	/** false: foundations show only the new floor top, never the surface they are stacked on. */
	static constexpr bool bShowFoundationReference = true;

	/** Readout size used until the player picks one. 1.0 is the full-size design. */
	static constexpr float DefaultHudScale = 0.70f;

	/** Smallest readout size the player can pick. */
	static constexpr float MinHudScale = 0.50f;

	/** Largest readout size the player can pick. */
	static constexpr float MaxHudScale = 2.00f;

	/** One-switch flip: false drops the position choice and pins the readout to DefaultPlacement. */
	static constexpr bool bAllowPlacementChoice = true;

	/** Where the readout sits until the player moves it. */
	static constexpr EFloorCheckPlacement DefaultPlacement = EFloorCheckPlacement::Right;

	/** How often the HUD reading is refreshed (seconds). */
	static constexpr float UpdateIntervalSeconds = 0.05f;

	/** How often the "is the research bought" answer is re-checked (seconds). */
	static constexpr float ResearchRecheckSeconds = 1.0f;

	/** Viewport z-order of the HUD widget; higher draws on top of the vanilla HUD. */
	static constexpr int32 WidgetZOrder = 50;

	// ---- Access ---------------------------------------------------------------------------------

	/** The subsystem of the world the context object lives in, or nullptr before it exists. */
	static AAltimeterSubsystem* Get( UObject* worldContext );

	// ---- Site zero (server authority, replicated, saved) ---------------------------------------

	UFUNCTION( BlueprintPure, Category = "Altimeter" )
	bool HasSiteZero() const { return mHasSiteZero; }

	/** Site zero as height above sea level in centimetres. Meaningless when HasSiteZero() is false. */
	UFUNCTION( BlueprintPure, Category = "Altimeter" )
	float GetSiteZeroCm() const { return mSiteZeroCm; }

	/** Height above sea level (cm) of the zero currently in use: the site zero, or 0 for sea level. */
	UFUNCTION( BlueprintPure, Category = "Altimeter" )
	float GetActiveZeroCm() const { return mHasSiteZero ? mSiteZeroCm : 0.f; }

	/** SERVER ONLY. Sets the site zero (height above sea level, cm). */
	UFUNCTION( BlueprintCallable, Category = "Altimeter" )
	void SetSiteZeroCm( float zeroCm );

	/** SERVER ONLY. Goes back to sea level. */
	UFUNCTION( BlueprintCallable, Category = "Altimeter" )
	void ClearSiteZero();

	// ---- Readings -------------------------------------------------------------------------------

	/** Computes the HUD reading for a hologram. Returns false (and an invalid reading) when there is nothing to show. */
	bool ReadHologram( const AFGHologram* hologram, FAltimeterReading& out_reading ) const;

	/** True when the HUD may be shown to this player (research bought, or bRequireResearch is off). */
	bool IsUnlockedFor( APlayerController* playerController ) const;

	// ---- AActor ---------------------------------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;
	virtual void GetLifetimeReplicatedProps( TArray< FLifetimeProperty >& OutLifetimeProps ) const override;

	// ---- IFGSaveInterface -----------------------------------------------------------------------
	virtual void PreSaveGame_Implementation( int32 saveVersion, int32 gameVersion ) override {}
	virtual void PostSaveGame_Implementation( int32 saveVersion, int32 gameVersion ) override {}
	virtual void PreLoadGame_Implementation( int32 saveVersion, int32 gameVersion ) override {}
	virtual void PostLoadGame_Implementation( int32 saveVersion, int32 gameVersion ) override {}
	virtual void GatherDependencies_Implementation( TArray< UObject* >& out_dependentObjects ) override {}
	virtual bool NeedTransform_Implementation() override { return false; }
	virtual bool ShouldSave_Implementation() const override { return true; }

private:
	/** Timer callback: reads the local player's hologram and updates the HUD widget. */
	void UpdateHud();

	/** The hologram the local player is placing, or nullptr. out_playerController is the local player when one exists. */
	AFGHologram* FindLocalHologram( APlayerController*& out_playerController ) const;

	UAltimeterWidget* GetOrCreateWidget( APlayerController* playerController );

	/**
	 * Height above sea level (cm) of the top surface the hologram has snapped to. False when it is placed freely.
	 * A member rather than a free function because the snap results are protected on AFGBuildableHologram,
	 * which Config/AccessTransformers.ini opens up to this class alone.
	 */
	bool FindSnappedTopCm( AFGBuildableHologram* hologram, float& out_topCm ) const;

	UPROPERTY( SaveGame, Replicated )
	bool mHasSiteZero = false;

	UPROPERTY( SaveGame, Replicated )
	float mSiteZeroCm = 0.f;

	UPROPERTY()
	TObjectPtr< UAltimeterWidget > mWidget;

	FTimerHandle mUpdateTimer;

	mutable double mLastResearchCheckSeconds = -1.0;
	mutable bool mCachedUnlocked = false;
};
