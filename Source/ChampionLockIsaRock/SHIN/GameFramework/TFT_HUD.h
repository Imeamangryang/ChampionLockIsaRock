#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TFT_HUD.generated.h"

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_HUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ATFT_HUD();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UUserWidget> StatUIClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> StatUIInstance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UUserWidget> ShopClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> ShopUIInstance;
	
	
};
