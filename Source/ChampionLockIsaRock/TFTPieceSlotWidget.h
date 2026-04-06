#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SHIN/Struct/FTFT_ChampionData.h"
#include "TFTPieceSlotWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFTPieceSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 챔피언 데이터 세팅 (TFTShopWidget에서 호출)
    UFUNCTION(BlueprintCallable, Category = "TFT|Shop")
    void SetChampionData(const FTFT_ChampionData& Data);

    // 슬롯 비우기
    UFUNCTION(BlueprintCallable, Category = "TFT|Shop")
    void ClearSlot();

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_0;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TextBlock_ChampionName;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TextBlock_Origins;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TextBlock_Classes;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TextBlock_Cost;
};
