#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFTShopWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UTFT_UISubsystem;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFTShopWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> pb_EXPBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> txt_Gold;

private:
    UFUNCTION()
    void OnGoldChanged(int32 NewGold);

    UFUNCTION()
    void OnLevelInfoUpdated(int32 NewLevel, int32 NewCurrentXP, int32 NewMaxXP);

    UTFT_UISubsystem* GetUISubsystem() const;

    bool bInitialized = false;
};