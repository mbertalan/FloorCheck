#include "AltimeterWidget.h"
#include "FloorCheck.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FloorCheck"

namespace AltimeterWidgetStyle
{
	static const FLinearColor PanelColour( 0.03f, 0.03f, 0.04f, 0.78f );
	static const FLinearColor AccentColour( 1.0f, 0.62f, 0.15f, 1.0f );
	static const FLinearColor TextColour( 0.92f, 0.92f, 0.92f, 1.0f );
	static const FLinearColor ShadowColour( 0.0f, 0.0f, 0.0f, 0.85f );

	/** Pixels between the crosshair and the near edge of the box, to the left or the right. */
	static constexpr float SideOffsetX = 120.f;

	/** Pixels down from the screen centre for the left and right positions. 0 keeps the box level with the crosshair. */
	static constexpr float SideOffsetY = 0.f;

	/** Pixels above the crosshair where the box ends. Clears the crosshair itself. */
	static constexpr float AboveOffsetY = 70.f;

	/** Pixels below the crosshair where the box starts. Clears the game's own build hints, which grow downwards. */
	static constexpr float BelowOffsetY = 200.f;

	static constexpr float PanelPaddingX = 14.f;
	static constexpr float PanelPaddingY = 8.f;
	static constexpr float LineGapY = 4.f;
	static constexpr int32 PrimaryFontSize = 26;
	static constexpr int32 SecondaryFontSize = 18;

	/** Below this the text stops being readable, so shrinking stops here. */
	static constexpr int32 MinFontSize = 10;

	/** Drop shadow behind the text at full size; it is scaled with the rest and never falls below one pixel. */
	static constexpr float ShadowOffsetPx = 2.f;

	/**
	 * The pivot on the box and the offset from the screen centre for each position, in the order of
	 * EFloorCheckPlacement. The pivot sits on the edge facing the crosshair, so a bigger readout grows away
	 * from it and the gap stays the same at every size.
	 */
	struct FPlacementGeometry
	{
		FVector2D Alignment;
		FVector2D Offset;
	};

	static const FPlacementGeometry Placements[] = {
		{ FVector2D( 0.f, 0.5f ), FVector2D( SideOffsetX, SideOffsetY ) },
		{ FVector2D( 1.f, 0.5f ), FVector2D( -SideOffsetX, SideOffsetY ) },
		{ FVector2D( 0.5f, 1.f ), FVector2D( 0.f, -AboveOffsetY ) },
		{ FVector2D( 0.5f, 0.f ), FVector2D( 0.f, BelowOffsetY ) }
	};

	int32 ScaledFontSize( int32 baseSize, float scale )
	{
		return FMath::Max( MinFontSize, FMath::RoundToInt( baseSize * scale ) );
	}

	float ScaledShadowOffset( float scale )
	{
		return FMath::Max( 1.f, FMath::RoundToFloat( ShadowOffsetPx * scale ) );
	}
}

UAltimeterWidget::UAltimeterWidget( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
}

UTextBlock* UAltimeterWidget::MakeText( const FLinearColor& colour ) const
{
	if( !WidgetTree )
	{
		return nullptr;
	}

	UTextBlock* block = WidgetTree->ConstructWidget< UTextBlock >( UTextBlock::StaticClass() );
	if( block )
	{
		block->SetColorAndOpacity( FSlateColor( colour ) );
		block->SetShadowColorAndOpacity( AltimeterWidgetStyle::ShadowColour );
		block->SetJustification( ETextJustify::Center );
	}
	return block;
}

void UAltimeterWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if( !WidgetTree )
	{
		UE_LOG( LogFloorCheck, Error, TEXT( "AltimeterWidget: no WidgetTree, HUD cannot be built" ) );
		return;
	}

	using namespace AltimeterWidgetStyle;

	UCanvasPanel* canvas = WidgetTree->ConstructWidget< UCanvasPanel >( UCanvasPanel::StaticClass() );
	WidgetTree->RootWidget = canvas;

	mPanel = WidgetTree->ConstructWidget< UBorder >( UBorder::StaticClass() );
	mPanel->SetBrushColor( PanelColour );

	UVerticalBox* lines = WidgetTree->ConstructWidget< UVerticalBox >( UVerticalBox::StaticClass() );
	mPanel->SetContent( lines );

	mPrimaryText = MakeText( AccentColour );
	if( UVerticalBoxSlot* slot = lines->AddChildToVerticalBox( mPrimaryText ) )
	{
		slot->SetHorizontalAlignment( HAlign_Center );
	}

	mSecondaryText = MakeText( TextColour );
	if( UVerticalBoxSlot* slot = lines->AddChildToVerticalBox( mSecondaryText ) )
	{
		slot->SetHorizontalAlignment( HAlign_Center );
	}

	// Anchored to the screen centre so every position is measured from the crosshair; the box sizes itself.
	mPanelSlot = canvas->AddChildToCanvas( mPanel );
	if( mPanelSlot )
	{
		mPanelSlot->SetAnchors( FAnchors( 0.5f, 0.5f ) );
		mPanelSlot->SetAutoSize( true );
	}

	ApplyUiScale();
	ApplyPlacement();

	// Never block mouse or keyboard; start hidden until the first valid reading arrives.
	SetVisibility( ESlateVisibility::Collapsed );
}

void UAltimeterWidget::SetUiScale( float scale )
{
	const float clamped = FMath::Clamp( scale, AAltimeterSubsystem::MinHudScale, AAltimeterSubsystem::MaxHudScale );
	if( FMath::IsNearlyEqual( clamped, mUiScale, 0.001f ) )
	{
		return;
	}

	mUiScale = clamped;
	ApplyUiScale();
}

void UAltimeterWidget::SetPlacement( EFloorCheckPlacement placement )
{
	if( placement == mPlacement )
	{
		return;
	}

	mPlacement = placement;
	ApplyPlacement();
}

void UAltimeterWidget::ApplyUiScale()
{
	using namespace AltimeterWidgetStyle;

	const FVector2D shadowOffset( ScaledShadowOffset( mUiScale ) );

	if( mPrimaryText )
	{
		mPrimaryText->SetFont( FCoreStyle::GetDefaultFontStyle( "Bold", ScaledFontSize( PrimaryFontSize, mUiScale ) ) );
		mPrimaryText->SetShadowOffset( shadowOffset );
	}

	if( mSecondaryText )
	{
		mSecondaryText->SetFont( FCoreStyle::GetDefaultFontStyle( "Regular", ScaledFontSize( SecondaryFontSize, mUiScale ) ) );
		mSecondaryText->SetShadowOffset( shadowOffset );
		if( UVerticalBoxSlot* slot = Cast< UVerticalBoxSlot >( mSecondaryText->Slot ) )
		{
			slot->SetPadding( FMargin( 0.f, LineGapY * mUiScale, 0.f, 0.f ) );
		}
	}

	if( mPanel )
	{
		mPanel->SetPadding( FMargin( PanelPaddingX * mUiScale, PanelPaddingY * mUiScale ) );
	}
}

void UAltimeterWidget::ApplyPlacement()
{
	using namespace AltimeterWidgetStyle;

	if( !mPanelSlot )
	{
		return;
	}

	// The gaps are deliberately not scaled: they clear the crosshair and the game's own hints, which do not
	// change with the readout size.
	const int32 index = static_cast< int32 >( mPlacement );
	const FPlacementGeometry& geometry = Placements[ index < static_cast< int32 >( UE_ARRAY_COUNT( Placements ) ) ? index : 0 ];

	mPanelSlot->SetAlignment( geometry.Alignment );
	mPanelSlot->SetPosition( geometry.Offset );
}

FString UAltimeterWidget::FormatMeters( float meters, bool withSign )
{
	float rounded = FMath::RoundToFloat( meters * 10.f ) / 10.f;
	if( FMath::Abs( rounded ) < 0.05f )
	{
		rounded = 0.f;
	}
	return withSign ? FString::Printf( TEXT( "%+.1f" ), rounded ) : FString::Printf( TEXT( "%.1f" ), rounded );
}

void UAltimeterWidget::SetReading( const FAltimeterReading& reading )
{
	if( !reading.bValid || !mPrimaryText )
	{
		SetVisibility( ESlateVisibility::Collapsed );
		return;
	}

	mPrimaryText->SetText( FText::Format( LOCTEXT( "PrimaryLine", "{0}  {1} m" ),
		reading.PrimaryLabel, FText::FromString( FormatMeters( reading.PrimaryMeters, reading.bRelativeToSiteZero ) ) ) );

	if( mSecondaryText )
	{
		if( reading.bHasSecondary )
		{
			mSecondaryText->SetText( FText::Format( LOCTEXT( "SecondaryLine", "{0}  {1} m" ),
				reading.SecondaryLabel, FText::FromString( FormatMeters( reading.SecondaryMeters, reading.bRelativeToSiteZero ) ) ) );
			mSecondaryText->SetVisibility( ESlateVisibility::HitTestInvisible );
		}
		else
		{
			mSecondaryText->SetVisibility( ESlateVisibility::Collapsed );
		}
	}

	SetVisibility( ESlateVisibility::HitTestInvisible );
}

#undef LOCTEXT_NAMESPACE
