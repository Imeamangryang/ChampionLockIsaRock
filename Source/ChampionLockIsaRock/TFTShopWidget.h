#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFTShopWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UButton;
class UTFT_UISubsystem;
class UTFTPieceSlotWidget;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFTShopWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "TFT|Shop")
    void RollShop();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> pb_EXPBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> txt_Gold;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> txt_Level;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> txt_Exp;

    // 코스트별 확률 표시 텍스트 위젯
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> txt_Prob_1;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> txt_Prob_2;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> txt_Prob_3;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> txt_Prob_4;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> txt_Prob_5;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUserWidget> WBP_ExpBuy;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUserWidget> WBP_Reroll;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot_1;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot_2;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot_3;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTFTPieceSlotWidget> WBP_PieceSlot_4;

private:
    UFUNCTION() void OnExpBuyClicked();
    UFUNCTION() void OnRerollClicked();

    UFUNCTION() void OnGoldChanged(int32 NewGold);
    UFUNCTION() void OnLevelInfoUpdated(int32 NewLevel, int32 NewCurrentXP, int32 NewMaxXP);

    UTFT_UISubsystem* GetUISubsystem() const;

    float TargetXPPercent = 0.f;
    float CurrentXPPercent = 0.f;
};