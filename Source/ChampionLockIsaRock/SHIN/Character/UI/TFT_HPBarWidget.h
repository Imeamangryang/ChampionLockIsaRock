#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFT_HPBarWidget.generated.h"

class UProgressBar;
class UCanvasPanel;
class UHorizontalBox;
class UImage;
class ATFT_UnitCharacter;


UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_HPBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetOwnerCharacter(ATFT_UnitCharacter* InOwnerCharacter);
	void UpdateHPBar() const;
	void UpdateMPBar() const;
	void RefreshDividers() const;
	
	void RefreshItemSlots();
	
	UFUNCTION(BlueprintImplementableEvent, Category="TFT|UI")
	void BP_UpdateStarFrame(int32 InStarLevel);
	
	void ApplyBarVerticalOffset(float OffsetY) const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* MPProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* DividerCanvas;
	
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* HPBarImage;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* HPBarImage_1;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* HPBarImage_2;
	
	UPROPERTY(meta = (BindWidgetOptional))
	UHorizontalBox* ItemSlotBox;

	UPROPERTY()
	ATFT_UnitCharacter* OwnerCharacter;
	
public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HP Bar")
	int32 HPPerDivider = 150;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Slot")
	float EquippedItemOffsetY = -22.f;
};
