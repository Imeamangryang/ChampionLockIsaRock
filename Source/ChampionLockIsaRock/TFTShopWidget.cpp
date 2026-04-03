#include "TFTShopWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "SHIN/Subsystem/TFT_UISubsystem.h"
#include "Dong/Public/TFTPlayerState.h"

void UTFTShopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UTFT_UISubsystem* UISub = GetUISubsystem())
    {
        UISub->OnGoldChanged.AddDynamic(this, &UTFTShopWidget::OnGoldChanged);
        UISub->OnLevelInfoUpdated.AddDynamic(this, &UTFTShopWidget::OnLevelInfoUpdated);
    }
}

void UTFTShopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bInitialized) return;

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
        {
            OnGoldChanged(PS->PlayerGold);
            OnLevelInfoUpdated(PS->PlayerLevel, PS->CurrentXP, PS->MaxXP);
            bInitialized = true;
        }
    }
}

void UTFTShopWidget::NativeDestruct()
{
    // 위젯 제거될 때 델리게이트 해제
    if (UTFT_UISubsystem* UISub = GetUISubsystem())
    {
        UISub->OnGoldChanged.RemoveDynamic(this, &UTFTShopWidget::OnGoldChanged);
        UISub->OnLevelInfoUpdated.RemoveDynamic(this, &UTFTShopWidget::OnLevelInfoUpdated);
    }

    Super::NativeDestruct();
}

UTFT_UISubsystem* UTFTShopWidget::GetUISubsystem() const
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            return LP->GetSubsystem<UTFT_UISubsystem>();
        }
    }
    return nullptr;
}

void UTFTShopWidget::OnGoldChanged(int32 NewGold)
{
    if (txt_Gold)
    {
        txt_Gold->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewGold)));
    }
}

void UTFTShopWidget::OnLevelInfoUpdated(int32 NewLevel, int32 NewCurrentXP, int32 NewMaxXP)
{
    // EXP 바 갱신
    if (pb_EXPBar)
    {
        float Percent = NewMaxXP > 0 ? static_cast<float>(NewCurrentXP) / static_cast<float>(NewMaxXP) : 0.f;
        pb_EXPBar->SetPercent(Percent);
    }

    UE_LOG(LogTemp, Log, TEXT("레벨: %d | XP: %d / %d"), NewLevel, NewCurrentXP, NewMaxXP);
}