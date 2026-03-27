#include "TFT_HPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"
#include "../TFT_UnitCharacter.h"
#include "../Components/TFT_StatComponent.h"
#include "SHIN/Subsystem/TFT_UISubsystem.h"
#include "Styling/SlateTypes.h"

void UTFT_HPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (HPProgressBar)
	{
		FProgressBarStyle ProgressBarStyle = HPProgressBar->GetWidgetStyle();
		ProgressBarStyle.BackgroundImage.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		//ProgressBarStyle.FillImage.TintColor = FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.f));
		HPProgressBar->SetWidgetStyle(ProgressBarStyle);
	}
	
	UpdateHPBar();
	UpdateMPBar();
	RefreshDividers();
}

void UTFT_HPBarWidget::SetOwnerCharacter(ATFT_UnitCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
	UpdateHPBar();
	UpdateMPBar();
	RefreshDividers();
}

void UTFT_HPBarWidget::UpdateHPBar()
{
	if (!HPProgressBar || !OwnerCharacter || !OwnerCharacter->StatComponent)
	{
		return;
	}

	const float MaxHP = static_cast<float>(OwnerCharacter->StatComponent->MaxHealth);
	const float CurrentHP = static_cast<float>(OwnerCharacter->StatComponent->Health);
	
	UTFT_UISubsystem* UISubsystem = nullptr;
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		UISubsystem = LP->GetSubsystem<UTFT_UISubsystem>();
	}
	
	UISubsystem->BroadcastHPUpdate(OwnerCharacter, MaxHP, CurrentHP);
	
	const float Percent = CurrentHP / MaxHP;
	HPProgressBar->SetPercent(Percent);
}

void UTFT_HPBarWidget::UpdateMPBar()
{
	const float MaxMP = static_cast<float>(OwnerCharacter->StatComponent->MaxMana);
	const float CurrentMP = static_cast<float>(OwnerCharacter->StatComponent->StartingMana);
	
	UTFT_UISubsystem* UISubsystem = nullptr;
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		UISubsystem = LP->GetSubsystem<UTFT_UISubsystem>();
	}
	
	UISubsystem->BroadcastMPUpdate(OwnerCharacter, MaxMP, CurrentMP);
	
	const float Percent = CurrentMP / MaxMP;
	MPProgressBar->SetPercent(Percent);
}

void UTFT_HPBarWidget::RefreshDividers()
{
	if (!DividerCanvas || !OwnerCharacter || !OwnerCharacter->StatComponent)
	{
		return;
	}

	DividerCanvas->ClearChildren();

	const int32 MaxHP = OwnerCharacter->StatComponent->MaxHealth;
	if (MaxHP <= HPPerDivider)
	{
		return;
	}

	const float BarWidth = 124.f;   // WidgetComponent DrawSize와 맞춰주는 값
	const float BarHeight = 11.f;
	const float DividerThickness = 3.f; // 기존 2.f보다 더 두껍게

	for (int32 HPMark = HPPerDivider; HPMark < MaxHP; HPMark += HPPerDivider)
	{
		const float Ratio = static_cast<float>(HPMark) / static_cast<float>(MaxHP);
		const float XPos = BarWidth * Ratio;

		UBorder* Divider = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		if (!Divider)
		{
			continue;
		}

		Divider->SetBrushColor(FLinearColor(0.0f, 0.007f, 0.015f, 1.0f));

		UCanvasPanelSlot* CanvasSlot = DividerCanvas->AddChildToCanvas(Divider);
		if (!CanvasSlot)
		{
			continue;
		}

		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(FVector2D(DividerThickness, BarHeight));
		CanvasSlot->SetPosition(FVector2D(XPos - (DividerThickness * 0.5f), 0.f));
	}
}
