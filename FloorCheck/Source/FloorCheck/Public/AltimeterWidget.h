#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AltimeterSubsystem.h"
#include "AltimeterWidget.generated.h"

class UBorder;
class UCanvasPanelSlot;
class UTextBlock;

/**
 * The on-screen readout: a small dark box beside the crosshair with the height of the hologram being placed.
 * It sits to the side rather than underneath so it never lands on the vanilla build hints, which grow downwards
 * from the crosshair as build mode, zoop count and "can't build here" lines appear.
 *
 * The whole widget tree is built in C++ so no Blueprint asset is needed. It is a pure view: the subsystem pushes
 * a FAltimeterReading into it many times per second and it only formats text.
 */
UCLASS()
class FLOORCHECK_API UAltimeterWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UAltimeterWidget( const FObjectInitializer& ObjectInitializer );

	/** Shows the reading, or hides the box when the reading is not valid. */
	void SetReading( const FAltimeterReading& reading );

	/**
	 * Sets how big the readout is drawn, 1.0 being the full-size design. The value is clamped to the
	 * subsystem's range. Cheap to call every frame; it only does work when the size actually changed.
	 */
	void SetUiScale( float scale );

	/**
	 * Sets which side of the crosshair the readout sits on. Cheap to call every frame; it only does work
	 * when the position actually changed.
	 */
	void SetPlacement( EFloorCheckPlacement placement );

protected:
	virtual void NativeOnInitialized() override;

private:
	UTextBlock* MakeText( const FLinearColor& colour ) const;

	/** Rebuilds the fonts, the line gap, the padding and the text shadow for the current size. */
	void ApplyUiScale();

	/** Pins the box to the current side of the crosshair, so it grows away from it whatever size it is. */
	void ApplyPlacement();

	/** "12.5" for sea-level readings, "+12.5" / "-3.0" when a site zero is set. Never "-0.0". */
	static FString FormatMeters( float meters, bool withSign );

	UPROPERTY()
	TObjectPtr< UBorder > mPanel;

	/** The panel's slot on the canvas; the gap from the crosshair is set through it. */
	UPROPERTY()
	TObjectPtr< UCanvasPanelSlot > mPanelSlot;

	UPROPERTY()
	TObjectPtr< UTextBlock > mPrimaryText;

	UPROPERTY()
	TObjectPtr< UTextBlock > mSecondaryText;

	float mUiScale = AAltimeterSubsystem::DefaultHudScale;

	EFloorCheckPlacement mPlacement = AAltimeterSubsystem::DefaultPlacement;
};
