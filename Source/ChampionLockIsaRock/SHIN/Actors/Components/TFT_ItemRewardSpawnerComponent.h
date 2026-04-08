#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFT_ItemRewardSpawnerComponent.generated.h"

class ATFT_ItemOrb;
class ATFT_CoinOrb;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHAMPIONLOCKISAROCK_API UTFT_ItemRewardSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTFT_ItemRewardSpawnerComponent();

	UFUNCTION(BlueprintCallable, Category="TFT|ItemReward")
	void SpawnStageItemRewards(int32 StageIndex);

	UFUNCTION(BlueprintCallable, Category="TFT|ItemReward")
	void ClearSpawnedRewards();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|ItemReward")
	TSubclassOf<ATFT_ItemOrb> ItemOrbClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|ItemReward")
	TSubclassOf<ATFT_CoinOrb> CoinOrbClass;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedRewardActors;
};