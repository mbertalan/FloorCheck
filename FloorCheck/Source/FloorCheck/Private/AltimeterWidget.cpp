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

	/** Pixels right of the screen centre (the crosshair) where the box starts. Negative puts it on the left. */
	static constexpr float SideOffsetX = 120.f;

	/** Pixels down from the screen centre. 0 keeps the box level with the crosshair. */
	static constexpr float SideOffsetY = 0.f;

	static constexpr float PanelPaddingX = 14.f;
	static constexpr float PanelPaddingY = 8.f;
	static constexpr float LineGapY = 2.f;
	static constexpr int32 PrimaryFontSize = 26;
	static constexpr int32 SecondaryFontSize = 16;

	/** Below this the text stops being readable, so shrinking stops here. */
	static constexpr int32 MinFontSize = 8;

	int32 ScaledFontSize( int32 baseSize, float scale )
	{
		return FMath::Max( MinFontSize, FMath::RoundToInt( baseSize * scale ) );
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
		block->SetShadowOffset( FVector2D( 1.f, 1.f ) );
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

	// Anchored to the screen centre, growing to the side: the vanilla build hints own the column below the crosshair.
	mPanelSlot = canvas->AddChildToCanvas( mPanel );
	if( mPanelSlot )
	{
		mPanelSlot->SetAnchors( FAnchors( 0.5f, 0.5f ) );
		mPanelSlot->SetAlignment( FVector2D( 0.f, 0.5f ) );
		mPanelSlot->SetAutoSize( true );
	}

	ApplyUiScale();

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

void UAltimeterWidget::ApplyUiScale()
{
	using namespace AltimeterWidgetStyle;

	if( mPrimaryText )
	{
		mPrimaryText->SetFont( FCoreStyle::GetDefaultFontStyle( "Bold", ScaledFontSize( PrimaryFontSize, mUiScale ) ) );
	}

	if( mSecondaryText )
	{
		mSecondaryText->SetFont( FCoreStyle::GetDefaultFontStyle( "Regular", ScaledFontSize( SecondaryFontSize, mUiScale ) ) );
		if( UVerticalBoxSlot* slot = Cast< UVerticalBoxSlot >( mSecondaryText->Slot ) )
		{
			slot->SetPadding( FMargin( 0.f, LineGapY * mUiScale, 0.f, 0.f ) );
		}
	}

	if( mPanel )
	{
		mPanel->SetPadding( FMargin( PanelPaddingX * mUiScale, PanelPaddingY * mUiScale ) );
	}

	if( mPanelSlot )
	{
		// The gap is fixed: it clears the column the vanilla build hints use, whatever size the readout is.
		mPanelSlot->SetPosition( FVector2D( SideOffsetX, SideOffsetY ) );
	}
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
