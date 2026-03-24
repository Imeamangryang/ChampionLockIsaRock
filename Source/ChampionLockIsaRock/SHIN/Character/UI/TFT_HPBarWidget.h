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
	void RefreshDividers();


protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* DividerCanvas;

	UPROPERTY()
	ATFT_UnitCharacter* OwnerCharacter;
	
public: 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MaxHealth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HP Bar")
	int32 HPPerDivider = 150;
};
