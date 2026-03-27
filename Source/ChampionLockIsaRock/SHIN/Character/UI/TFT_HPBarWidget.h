#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFT_HPBarWidget.generated.h"

class UProgressBar;
class UCanvasPanel;
class ATFT_UnitCharacter;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_HPBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetOwnerCharacter(ATFT_UnitCharacter* InOwnerCharacter);
	void UpdateHPBar();
	void UpdateMPBar();
	void RefreshDividers();
	
	UFUNCTION(BlueprintImplementableEvent, Category="TFT|UI")
	void BP_UpdateStarFrame(int32 InStarLevel);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* MPProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* DividerCanvas;

	UPROPERTY()
	ATFT_UnitCharacter* OwnerCharacter;
	
public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HP Bar")
	int32 HPPerDivider = 150;
};
