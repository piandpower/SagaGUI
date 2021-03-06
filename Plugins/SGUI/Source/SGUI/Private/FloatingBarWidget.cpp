#include "SGUI.h"
#include "FloatingBarWidget.h"

UFloatingBarWidget* UFloatingBarWidget::Create(UObject* worldContextObject, AActor* followTarget, AActor* lookPoint, FVector offset, FLinearColor fillColor)
{
	auto widget = InstantiateWidget<UFloatingBarWidget>(worldContextObject, -100);
	if (!widget) return nullptr;

	widget->followTarget = followTarget;
	widget->lookPoint = lookPoint;
	widget->offset = offset;

	widget->floatingPanel = Cast<UCanvasPanel>(widget->GetWidgetFromName(TEXT("FloatingPanel")));
	widget->floatingBar = Cast<UProgressBar>(widget->GetWidgetFromName(TEXT("FloatingBar")));
	widget->SetFillColor(fillColor);
	widget->hpLabel = Cast<UTextBlock>(widget->GetWidgetFromName(TEXT("HPLabel")));
	widget->nameLabel = Cast<UTextBlock>(widget->GetWidgetFromName(TEXT("NameLabel")));

	widget->SetOpacity(0);

	return widget;
}

void UFloatingBarWidget::SetPlayerName(FText value)
{
	nameLabel->SetText(value);
}

void UFloatingBarWidget::SetFillAmount(const float& value)
{
	floatingBar->SetPercent(value);
	hpLabel->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), (uint8)(floatingBar->Percent * 100))));
}

void UFloatingBarWidget::SetFillColor(FLinearColor fillColor)
{
	floatingBar->SetFillColorAndOpacity(fillColor);
}

void UFloatingBarWidget::NativeTick(const FGeometry& myGeometry, float deltaTime)
{
	Super::NativeTick(myGeometry, deltaTime);

	if (!MasterController || !followTarget || followTarget->IsPendingKillPending()) { RemoveFromViewport(); return; }

	FVector targetLocation = followTarget->GetActorLocation() + offset;
	if (SetPositionFromWorld(targetLocation, GetSlot(floatingPanel)))
	{
		floatingBar->SetVisibility(ESlateVisibility::Visible);
		hpLabel->SetVisibility(ESlateVisibility::Visible);
		nameLabel->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		floatingBar->SetVisibility(ESlateVisibility::Hidden);
		hpLabel->SetVisibility(ESlateVisibility::Hidden);
		nameLabel->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	if (MasterController->GetPawn())
	{
		float opacity;
		opacity = FMath::InterpExpoOut(floatingBar->FillColorAndOpacity.A,
			(!MasterController->LineOfSightTo(followTarget, lookPoint ? lookPoint->GetActorLocation() : FVector::ZeroVector) || floatingBar->Percent <= .01f) ? 0 
			: FMath::Clamp((VisibleRadius - FVector::Dist(lookPoint ? lookPoint->GetActorLocation() 
			: MasterController->GetPawn()->GetActorLocation(), followTarget->GetActorLocation())) / VisibleRadius, 0.f, 1.f),
			deltaTime * VisibilityTransitionSpeed);

		SetOpacity(opacity);
	}
	else SetOpacity(0);

	if (onTick.IsBound()) SetFillAmount(onTick.Execute());
}

void UFloatingBarWidget::SetOpacity(float opacity)
{
	FLinearColor color = floatingBar->FillColorAndOpacity;
	color.A = opacity;
	floatingBar->SetFillColorAndOpacity(color);

	color = floatingBar->WidgetStyle.BackgroundImage.TintColor.GetSpecifiedColor();
	floatingBar->WidgetStyle.BackgroundImage.TintColor = FSlateColor(FLinearColor(color.R, color.G, color.B, opacity));

	color = hpLabel->ColorAndOpacity.GetSpecifiedColor();
	color.A = opacity;
	hpLabel->SetColorAndOpacity(color);
	nameLabel->SetColorAndOpacity(color);
	color = hpLabel->ShadowColorAndOpacity;
	color.A = FMath::Clamp(color.A - .2f, 0.f, 1.f);
	hpLabel->SetShadowColorAndOpacity(color);
	nameLabel->SetShadowColorAndOpacity(color);
}
