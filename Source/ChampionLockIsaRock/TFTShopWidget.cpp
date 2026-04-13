#include "TFTShopWidget.h"
#include "TFTPieceSlotWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "SHIN/Subsystem/TFT_UISubsystem.h"
#include "Dong/Public/TFTPlayerState.h"
#include "SHIN/GameFramework/TFT_GameInstance.h"
#include "SHIN/Struct/FTFT_ChampionData.h"
#include "Engine/DataTable.h"
#include "Math/RandomStream.h"

static const float SHOP_ROLL_ODDS[10][5] = {
    { 1.00f, 0.00f, 0.00f, 0.00f, 0.00f }, // Lv.1
    { 1.00f, 0.00f, 0.00f, 0.00f, 0.00f }, // Lv.2
    { 0.75f, 0.25f, 0.00f, 0.00f, 0.00f }, // Lv.3
    { 0.55f, 0.30f, 0.15f, 0.00f, 0.00f }, // Lv.4
    { 0.45f, 0.33f, 0.20f, 0.02f, 0.00f }, // Lv.5
    { 0.30f, 0.40f, 0.25f, 0.05f, 0.00f }, // Lv.6
    { 0.16f, 0.30f, 0.43f, 0.10f, 0.01f }, // Lv.7
    { 0.15f, 0.20f, 0.32f, 0.30f, 0.03f }, // Lv.8
    { 0.10f, 0.17f, 0.25f, 0.33f, 0.15f }, // Lv.9
    { 0.05f, 0.10f, 0.20f, 0.40f, 0.25f }  // Lv.10
};

void UTFTShopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UTFT_UISubsystem* UISub = GetUISubsystem()) {
        UISub->OnGoldChanged.AddDynamic(this, &UTFTShopWidget::OnGoldChanged);
        UISub->OnLevelInfoUpdated.AddDynamic(this, &UTFTShopWidget::OnLevelInfoUpdated);
    }

    if (WBP_ExpBuy) {
        if (UButton* RealBtn = Cast<UButton>(WBP_ExpBuy->GetWidgetFromName(TEXT("Button_1")))) {
            RealBtn->OnClicked.AddDynamic(this, &UTFTShopWidget::OnExpBuyClicked);
        }
    }

    if (WBP_Reroll) {
        if (UButton* RealBtn = Cast<UButton>(WBP_Reroll->GetWidgetFromName(TEXT("Button_1")))) {
            RealBtn->OnClicked.AddDynamic(this, &UTFTShopWidget::OnRerollClicked);
        }
    }

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]() {
        if (auto* PS = GetOwningPlayerState<ATFTPlayerState>()) {
            OnGoldChanged(PS->PlayerGold);
            OnLevelInfoUpdated(PS->PlayerLevel, PS->CurrentXP, PS->MaxXP);
            RollShop();
        }
    }, 0.1f, false);
}

void UTFTShopWidget::NativeDestruct()
{
    if (UTFT_UISubsystem* UISub = GetUISubsystem()) {
        UISub->OnGoldChanged.RemoveDynamic(this, &UTFTShopWidget::OnGoldChanged);
        UISub->OnLevelInfoUpdated.RemoveDynamic(this, &UTFTShopWidget::OnLevelInfoUpdated);
    }
    Super::NativeDestruct();
}

void UTFTShopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (pb_EXPBar) {
        CurrentXPPercent = FMath::FInterpConstantTo(CurrentXPPercent, TargetXPPercent, InDeltaTime, 9.5f);
        pb_EXPBar->SetPercent(CurrentXPPercent);
    }
}

void UTFTShopWidget::OnExpBuyClicked()
{
    if (auto* PS = GetOwningPlayerState<ATFTPlayerState>()) {
        PS->BuyXP();
    }
}
 
void UTFTShopWidget::OnRerollClicked()
{
    if (auto* PS = GetOwningPlayerState<ATFTPlayerState>()) {
        if (PS->SpendGold(2)) { 
            RollShop();
        }
    }
}

void UTFTShopWidget::OnGoldChanged(int32 NewGold)
{
    // 1. 골드 텍스트 업데이트
    if (txt_Gold) txt_Gold->SetText(FText::AsNumber(NewGold));

    // 2. 버튼 활성화 제어 (경험치 4골드, 리롤 2골드)
    if (WBP_ExpBuy) {
        if (UButton* RealBtn = Cast<UButton>(WBP_ExpBuy->GetWidgetFromName(TEXT("Button_1")))) {
            RealBtn->SetIsEnabled(NewGold >= 4);
        }
    }

    if (WBP_Reroll) {
        if (UButton* RealBtn = Cast<UButton>(WBP_Reroll->GetWidgetFromName(TEXT("Button_1")))) {
            RealBtn->SetIsEnabled(NewGold >= 2);
        }
    }

    // 3. 기물 슬롯 정밀 제어 (변수명을 PieceSlots로 변경하여 엔진 내장 변수 Slot과의 충돌 회피)
    TArray<UTFTPieceSlotWidget*> PieceSlots = { 
        WBP_PieceSlot, WBP_PieceSlot_1, WBP_PieceSlot_2, WBP_PieceSlot_3, WBP_PieceSlot_4 
    };

    for (auto* PieceSlot : PieceSlots) {
        if (PieceSlot) {
            // 정밀 제어: 슬롯 기물의 가격과 소지 금액 비교
            int32 PieceCost = PieceSlot->GetPieceCost();
            PieceSlot->SetIsEnabled(NewGold >= PieceCost);
        }
    }
}

void UTFTShopWidget::OnLevelInfoUpdated(int32 NewLevel, int32 NewCurrentXP, int32 NewMaxXP)
{
    if (txt_Level) txt_Level->SetText(FText::AsNumber(NewLevel));
    if (txt_Exp) {
        FString ExpString = FString::Printf(TEXT("%d / %d"), NewCurrentXP, NewMaxXP);
        txt_Exp->SetText(FText::FromString(ExpString));
    }

    UTextBlock* ProbTexts[] = { txt_Prob_1, txt_Prob_2, txt_Prob_3, txt_Prob_4, txt_Prob_5 };
    FLinearColor TierColors[] = { 
        FLinearColor(0.5f, 0.5f, 0.5f), FLinearColor(0.12f, 0.56f, 0.12f),
        FLinearColor(0.12f, 0.34f, 0.85f), FLinearColor(0.66f, 0.12f, 0.82f),
        FLinearColor(0.95f, 0.65f, 0.05f)
    };

    int32 LevelIdx = FMath::Clamp(NewLevel - 1, 0, 9);
    for (int32 i = 0; i < 5; ++i) {
        if (ProbTexts[i]) {
            int32 ProbPercent = FMath::RoundToInt(SHOP_ROLL_ODDS[LevelIdx][i] * 100.f);
            ProbTexts[i]->SetText(FText::Format(FText::FromString("{0}%"), FText::AsNumber(ProbPercent)));
            ProbTexts[i]->SetColorAndOpacity(FSlateColor(TierColors[i]));
        }
    }
    TargetXPPercent = (NewMaxXP > 0) ? (float)NewCurrentXP / NewMaxXP : 1.0f;
}

void UTFTShopWidget::RollShop()
{
    UTFT_GameInstance* GI = Cast<UTFT_GameInstance>(GetGameInstance());
    UDataTable* DT = GI ? GI->ChampionDataTable.LoadSynchronous() : nullptr;
    if (!DT) return;

    int32 CurrentLevel = 1;
    if (auto* PS = GetOwningPlayerState<ATFTPlayerState>()) {
        CurrentLevel = PS->PlayerLevel;
    }

    TArray<FTFT_ChampionData*> AllRows;
    DT->GetAllRows<FTFT_ChampionData>(TEXT("RollShop"), AllRows);
    
    TArray<FTFT_ChampionData*> ByTier[5];
    for (auto* Row : AllRows) {
        FString RowName = Row->Name.ToString();
        if (RowName.Contains(TEXT("Scuttle")) || RowName.IsEmpty() || Row->Cost <= 0) continue;
        int32 TierIdx = FMath::Clamp(Row->Cost - 1, 0, 4);
        ByTier[TierIdx].Add(Row);
    }

    FRandomStream Stream; Stream.GenerateNewSeed();
    TArray<UTFTPieceSlotWidget*> Slots = { 
        WBP_PieceSlot, WBP_PieceSlot_1, WBP_PieceSlot_2, WBP_PieceSlot_3, WBP_PieceSlot_4 
    };

    for (auto* PieceSlot : Slots) {
        if (!PieceSlot) continue;
        int32 LevelIdx = FMath::Clamp(CurrentLevel - 1, 0, 9);
        float Roll = Stream.FRand();
        float Cumulative = 0.f;
        int32 PickedTierIdx = 0;

        for (int32 i = 0; i < 5; i++) {
            Cumulative += SHOP_ROLL_ODDS[LevelIdx][i];
            if (Roll < Cumulative) { PickedTierIdx = i; break; }
        }

        if (ByTier[PickedTierIdx].Num() > 0) {
            int32 RandIdx = Stream.RandRange(0, ByTier[PickedTierIdx].Num() - 1);
            PieceSlot->SetChampionData(*ByTier[PickedTierIdx][RandIdx]);
        }
    }
    
    if (auto* PS = GetOwningPlayerState<ATFTPlayerState>()) {
        OnGoldChanged(PS->PlayerGold);
    }
}

UTFT_UISubsystem* UTFTShopWidget::GetUISubsystem() const
{
    if (APlayerController* PC = GetOwningPlayer()) {
        if (ULocalPlayer* LP = PC->GetLocalPlayer()) return LP->GetSubsystem<UTFT_UISubsystem>();
    }
    return nullptr;
}