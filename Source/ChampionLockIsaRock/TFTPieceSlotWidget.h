#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SHIN/Struct/FTFT_ChampionData.h"
#include "SHIN/Struct/ETFT_ChampionList.h"
#include "TFTPieceSlotWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFTPieceSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "TFT|Shop")
    void SetChampionData(const FTFT_ChampionData& Data);

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

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button;

private:
    ETFT_ChampionKey CurrentChampionKey;
    int32 CurrentCost = 1;

    UFUNCTION()
    void OnSlotClicked();
};
