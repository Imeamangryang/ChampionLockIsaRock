#include "TFTShopWidget.h"
#include "TFTPieceSlotWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "SHIN/Subsystem/TFT_UISubsystem.h"
#include "Dong/Public/TFTPlayerState.h"
#include "SHIN/GameFramework/TFT_GameInstance.h"
#include "SHIN/Struct/FTFT_ChampionData.h"
#include "Engine/DataTable.h"

// 레벨별 티어(Cost) 확률 테이블 [레벨-1][Cost 1~5]
static const float ROLL_ODDS[10][5] = {
    { 1.00f, 0.00f, 0.00f, 0.00f, 0.00f }, // Lv1
    { 1.00f, 0.00f, 0.00f, 0.00f, 0.00f }, // Lv2
    { 0.75f, 0.25f, 0.00f, 0.00f, 0.00f }, // Lv3
    { 0.55f, 0.30f, 0.15f, 0.00f, 0.00f }, // Lv4
    { 0.45f, 0.33f, 0.20f, 0.02f, 0.00f }, // Lv5
    { 0.30f, 0.40f, 0.25f, 0.05f, 0.00f }, // Lv6
    { 0.16f, 0.30f, 0.43f, 0.10f, 0.01f }, // Lv7
    { 0.15f, 0.20f, 0.32f, 0.30f, 0.03f }, // Lv8
    { 0.10f, 0.17f, 0.25f, 0.33f, 0.15f }, // Lv9
    { 0.05f, 0.10f, 0.20f, 0.40f, 0.25f }, // Lv10
};

// 레벨 기반으로 Cost 1~5 중 하나 랜덤 선택
static int32 RollCostByLevel(int32 Level, FRandomStream& Stream)
{
    int32 Idx = FMath::Clamp(Level - 1, 0, 9);
    float Roll = Stream.FRand();
    float Cumulative = 0.f;
    for (int32 i = 0; i < 5; i++)
    {
        Cumulative += ROLL_ODDS[Idx][i];
        if (Roll < Cumulative) return i + 1;
    }
    return 1;
}

void UTFTShopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UTFT_UISubsystem* UISub = GetUISubsystem())
    {
        UISub->OnGoldChanged.AddDynamic(this, &UTFTShopWidget::OnGoldChanged);
        UISub->OnLevelInfoUpdated.AddDynamic(this, &UTFTShopWidget::OnLevelInfoUpdated);
    }

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
            {
                OnGoldChanged(PS->PlayerGold);
                OnLevelInfoUpdated(PS->PlayerLevel, PS->CurrentXP, PS->MaxXP);
            }
        }
        RollShop();
    }, 0.1f, false);
}

void UTFTShopWidget::NativeDestruct()
{
    if (UTFT_UISubsystem* UISub = GetUISubsystem())
    {
        UISub->OnGoldChanged.RemoveDynamic(this, &UTFTShopWidget::OnGoldChanged);
        UISub->OnLevelInfoUpdated.RemoveDynamic(this, &UTFTShopWidget::OnLevelInfoUpdated);
    }
    Super::NativeDestruct();
}

void UTFTShopWidget::RollShop()
{
    FRandomStream Stream;
    Stream.GenerateNewSeed();
    UTFT_GameInstance* GI = Cast<UTFT_GameInstance>(GetGameInstance());
    if (!GI) return;

    UDataTable* DT = GI->ChampionDataTable.LoadSynchronous();
    if (!DT) return;

    // 현재 레벨 가져오기
    int32 CurrentLevel = 1;
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
        {
            CurrentLevel = PS->PlayerLevel;
        }
    }

    // 전체 행에서 Cost별로 분류
    TArray<FTFT_ChampionData*> AllRows;
    DT->GetAllRows<FTFT_ChampionData>(TEXT("RollShop"), AllRows);
    if (AllRows.Num() == 0) return;

    TArray<FTFT_ChampionData*> ByTier[5]; // ByTier[0] = Cost1, ..., ByTier[4] = Cost5
    for (FTFT_ChampionData* Row : AllRows)
    {
        int32 TierIdx = FMath::Clamp(Row->Cost - 1, 0, 4);
        ByTier[TierIdx].Add(Row);
    }

    // 슬롯 배열
    TArray<UTFTPieceSlotWidget*> Slots = {
        WBP_PieceSlot, WBP_PieceSlot_1, WBP_PieceSlot_2,
        WBP_PieceSlot_3, WBP_PieceSlot_4
    };

    for (UTFTPieceSlotWidget* PieceSlot : Slots)
    {
        if (!PieceSlot) continue;

        // 레벨 기반 Cost 결정
        int32 Cost = RollCostByLevel(CurrentLevel, Stream);
        int32 TierIdx = Cost - 1;

        // 해당 티어에 챔피언이 없으면 Cost1으로 fallback
        if (ByTier[TierIdx].Num() == 0) TierIdx = 0;
        if (ByTier[TierIdx].Num() == 0) continue;

        int32 RandIdx = Stream.RandRange(0, ByTier[TierIdx].Num() - 1);
        PieceSlot->SetChampionData(*ByTier[TierIdx][RandIdx]);
    }
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
    UE_LOG(LogTemp, Warning, TEXT("=== OnGoldChanged: %d ==="), NewGold);
    if (txt_Gold)
    {
        txt_Gold->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewGold)));
    }
}

void UTFTShopWidget::OnLevelInfoUpdated(int32 NewLevel, int32 NewCurrentXP, int32 NewMaxXP)
{
    if (pb_EXPBar)
    {
        float Percent = NewMaxXP > 0 ? static_cast<float>(NewCurrentXP) / static_cast<float>(NewMaxXP) : 0.f;
        pb_EXPBar->SetPercent(Percent);
    }
}