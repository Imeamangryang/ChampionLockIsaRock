#include "TFTPieceSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "Dong/Public/TopDownController.h"
#include "Dong/Public/TFTPlayerState.h"

void UTFTPieceSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button)
    {
        Button->OnClicked.AddDynamic(this, &UTFTPieceSlotWidget::OnSlotClicked);
    }
}

void UTFTPieceSlotWidget::SetChampionData(const FTFT_ChampionData& Data)
{
    SetVisibility(ESlateVisibility::Visible);
    CurrentCost = Data.Cost;

    const UEnum* EnumPtr = StaticEnum<ETFT_ChampionKey>();
    if (EnumPtr)
    {
        int64 EnumVal = EnumPtr->GetValueByNameString(Data.Key);
        if (EnumVal != INDEX_NONE)
        {
            CurrentChampionKey = static_cast<ETFT_ChampionKey>(EnumVal);
        }
    }

    if (TextBlock_ChampionName)
        TextBlock_ChampionName->SetText(Data.Name);

    if (TextBlock_Origins)
        TextBlock_Origins->SetText(FText::FromString(Data.Origins));

    if (TextBlock_Classes)
        TextBlock_Classes->SetText(FText::FromString(Data.Classes));

    if (TextBlock_Cost)
        TextBlock_Cost->SetText(FText::FromString(FString::Printf(TEXT("%d"), Data.Cost)));

    if (Image_0 && !Data.Image.IsNull())
    {
        UTexture2D* Tex = Data.Image.LoadSynchronous();
        if (Tex) Image_0->SetBrushFromTexture(Tex);
    }
}

void UTFTPieceSlotWidget::ClearSlot()
{
    CurrentCost = 0;
    SetVisibility(ESlateVisibility::Hidden);
}

void UTFTPieceSlotWidget::OnSlotClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("=== 슬롯 클릭됨, Cost: %d ==="), CurrentCost);

    if (CurrentCost <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== Cost 0이라 리턴 ==="));
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== PC 없음 ==="));
        return;
    }

    ATopDownController* TDC = Cast<ATopDownController>(PC);
    if (!TDC)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== TDC 캐스트 실패 ==="));
        return;
    }

    ATFTPlayerState* PS = TDC->GetPlayerState<ATFTPlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== PS 없음 ==="));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("=== 골드: %d, 코스트: %d ==="), PS->PlayerGold, CurrentCost);

    if (!PS->SpendGold(CurrentCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("=== 골드 부족 ==="));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("=== SpawnUnitFromBP 호출 ==="));
    TDC->SpawnUnitFromBP(CurrentChampionKey);
    ClearSlot();
}