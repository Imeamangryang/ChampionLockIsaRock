#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TFTPlayerController.generated.h"

UCLASS()
class ATFTPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> ShopWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 Gold = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 RerollCost = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 MaxGold = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 XP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 BuyXPCost = 4;

private:
	UPROPERTY()
	class UUserWidget* ShopWidget;
};