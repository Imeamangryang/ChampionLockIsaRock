#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFTShopWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UTFT_UISubsystem;
class UTFTPieceSlotWidget;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFTShopWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 샵 리롤 (랜덤 5개 다시 뽑기)
    UFUNCTION(BlueprintCallable, Category = "TFT|Shop")
    void RollShop();

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> pb_EXPBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> txt_Gold;

    // PieceSlot 5개
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot_1;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot_2;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot_3;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot_4;

private:
    UFUNCTION()
    void OnGoldChanged(int32 NewGold);

    UFUNCTION()
    void OnLevelInfoUpdated(int32 NewLevel, int32 NewCurrentXP, int32 NewMaxXP);

    UTFT_UISubsystem* GetUISubsystem() const;
};
