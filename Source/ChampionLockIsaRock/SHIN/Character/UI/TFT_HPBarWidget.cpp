#include "TFT_HPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Engine/Texture2D.h"
#include "Blueprint/WidgetTree.h"
#include "../TFT_UnitCharacter.h"
#include "../Components/TFT_StatComponent.h"
#include "SHIN/Subsystem/TFT_UISubsystem.h"
#include "Styling/SlateTypes.h"

void UTFT_HPBarWidget::ApplyBarVerticalOffset(float OffsetY) const
{
	const FVector2D Offset(0.f, OffsetY);

	if (HPBarImage)
	{
		HPBarImage->SetRenderTranslation(Offset);
	}

	if (HPBarImage_1)
	{
		HPBarImage_1->SetRenderTranslation(Offset);
	}

	if (HPBarImage_2)
	{
		HPBarImage_2->SetRenderTranslation(Offset);
	}

	if (HPProgressBar)
	{
		HPProgressBar->SetRenderTranslation(Offset);
	}

	if (MPProgressBar)
	{
		MPProgressBar->SetRenderTranslation(Offset);
	}

	if (DividerCanvas)
	{
		DividerCanvas->SetRenderTranslation(Offset);
	}
}

void UTFT_HPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (HPProgressBar)
	{
		FProgressBarStyle ProgressBarStyle = HPProgressBar->GetWidgetStyle();
		
		if (OwnerCharacter && OwnerCharacter->bIsEnemy)
		{
			HPProgressBar->SetFillColorAndOpacity(FLinearColor(0.242281f, 0.05448f, 0.039546f, 1.f));
		}
		ProgressBarStyle.BackgroundImage.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		HPProgressBar->SetWidgetStyle(ProgressBarStyle);
	}
	
	UpdateHPBar();
	UpdateMPBar();
	RefreshDividers();
	RefreshItemSlots();
}

void UTFT_HPBarWidget::SetOwnerCharacter(ATFT_UnitCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
	
	if (HPProgressBar)
	{
		FProgressBarStyle ProgressBarStyle = HPProgressBar->GetWidgetStyle();
		
		if (OwnerCharacter && OwnerCharacter->bIsEnemy)
		{
			HPProgressBar->SetFillColorAndOpacity(FLinearColor(0.242281f, 0.05448f, 0.039546f, 1.f));
		}
		ProgressBarStyle.BackgroundImage.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		HPProgressBar->SetWidgetStyle(ProgressBarStyle);
	}
	UpdateHPBar();
	UpdateMPBar();
	RefreshDividers();
	RefreshItemSlots();
}

void UTFT_HPBarWidget::UpdateHPBar() const
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
	
	if (UISubsystem)
	{
		UISubsystem->BroadcastHPUpdate(OwnerCharacter, MaxHP, CurrentHP);
	}
	
	const float Percent = MaxHP > 0.f ? (CurrentHP / MaxHP) : 0.f;
	HPProgressBar->SetPercent(Percent);
}

void UTFT_HPBarWidget::UpdateMPBar() const
{
	if (!MPProgressBar || !OwnerCharacter || !OwnerCharacter->StatComponent)
	{
		return;
	}

	const float MaxMP = static_cast<float>(OwnerCharacter->StatComponent->MaxMana);
	const float CurrentMP = static_cast<float>(OwnerCharacter->StatComponent->StartingMana);

	UTFT_UISubsystem* UISubsystem = nullptr;
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		UISubsystem = LP->GetSubsystem<UTFT_UISubsystem>();
	}
	
	if (UISubsystem)
	{
		UISubsystem->BroadcastMPUpdate(OwnerCharacter, MaxMP, CurrentMP);
	}

	const float Percent = MaxMP > 0.f ? (CurrentMP / MaxMP) : 0.f;
	MPProgressBar->SetPercent(Percent);
}

void UTFT_HPBarWidget::RefreshDividers() const
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

	const float BarWidth = 70.f;   // WidgetComponent DrawSize와 맞춰주는 값
	const float BarHeight = 6.f;
	const float DividerThickness = 2.f; // 기존 2.f보다 더 두껍게

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

void UTFT_HPBarWidget::RefreshItemSlots()
{
	if (!ItemSlotBox || !OwnerCharacter)
	{
		return;
	}

	ItemSlotBox->ClearChildren();

	int32 EquippedCount = 0;

	for (const FStruct_TFTEquippedItemSlot& SlotData : OwnerCharacter->EquippedItemSlots)
	{
		USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		if (!SizeBox)
		{
			continue;
		}

		SizeBox->SetWidthOverride(25.f);
		SizeBox->SetHeightOverride(25.f);

		UBorder* SlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		if (!SlotBorder)
		{
			continue;
		}

		SlotBorder->SetPadding(FMargin(0.5f));

		if (SlotData.bOccupied)
		{
			++EquippedCount;
			SlotBorder->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));

			if (UTexture2D* ItemIcon = OwnerCharacter->GetItemIconByItemId(SlotData.ItemInstance.ItemId))
			{
				UImage* IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
				if (IconImage)
				{
					FSlateBrush Brush;
					Brush.SetResourceObject(ItemIcon);
					Brush.ImageSize = FVector2D(32.f, 32.f);
					IconImage->SetBrush(Brush);
					SlotBorder->SetContent(IconImage);
				}
			}
		}
		else
		{
			// 빈 슬롯은 공간만 차지하고 완전 투명
			SlotBorder->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
		}

		SizeBox->SetContent(SlotBorder);

		if (UHorizontalBoxSlot* BoxSlot = ItemSlotBox->AddChildToHorizontalBox(SizeBox))
		{
			BoxSlot->SetPadding(FMargin(0.5f, 0.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Center);
			BoxSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	if (EquippedCount == 0)
	{
		ItemSlotBox->SetVisibility(ESlateVisibility::Collapsed);
		ApplyBarVerticalOffset(0.f);
	}
	else
	{
		ItemSlotBox->SetVisibility(ESlateVisibility::Visible);
		ApplyBarVerticalOffset(EquippedItemOffsetY);
	}
}
