#include "TFTPieceSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "Dong/Public/TopDownController.h"
#include "Dong/Public/TFTPlayerState.h"
#include "Dong/Public/BenchManager.h"
#include "Kismet/GameplayStatics.h"

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
    if (Button) Button->SetIsEnabled(true);
    
    // 데이터와 가격을 모두 저장합니다.
    CurrentChampionData = Data;
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

    // 블루프린트 이벤트 호출 (배경색 변경 등)
    UpdateButtonStyleByCost(Data.Cost);
}

void UTFTPieceSlotWidget::ClearSlot()
{
    // 슬롯을 비울 때 가격도 0으로 초기화합니다.
    CurrentCost = 0;
    if (Button) Button->SetIsEnabled(false);
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
    if (!PC) return;

    ATopDownController* TDC = Cast<ATopDownController>(PC);
    if (!TDC) return;

    ATFTPlayerState* PS = TDC->GetPlayerState<ATFTPlayerState>();
    if (!PS) return;
    
    ABenchManager* BenchManager = Cast<ABenchManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABenchManager::StaticClass()));
    if (BenchManager)
    {
        // GetFirstEmptySlotIndex()가 -1이면 자리가 없다는 뜻
        if (BenchManager->GetFirstEmptySlotIndex() == -1)
        {
            return; // 여기서 함수를 강제 종료시켜서 아래의 골드 차감 로직을 막아냅니다.
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("=== 골드: %d, 코스트: %d ==="), PS->PlayerGold, CurrentCost);

    // 자리가 있는 게 확인되었으니 돈을 뺍니다.
    if (!PS->SpendGold(CurrentCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("=== 골드 부족 ==="));
        return;
    }

    // 유닛을 스폰하고 상점 슬롯을 비웁니다.
    TDC->SpawnUnitFromBP(CurrentChampionKey);
    ClearSlot();
}