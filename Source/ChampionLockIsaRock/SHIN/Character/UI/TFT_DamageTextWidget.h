#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFT_DamageTextWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_DamageTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="TFT|DamageText")
	void SetDamageText(int32 Damage, bool bCritical);

protected:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* DamageText;
	
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* CriticalIcon;
};
