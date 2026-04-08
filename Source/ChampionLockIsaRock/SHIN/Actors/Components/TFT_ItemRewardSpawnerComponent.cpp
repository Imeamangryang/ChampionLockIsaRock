#include "TFT_ItemRewardSpawnerComponent.h"
#include "SHIN/Actors/TFT_ItemOrb.h"
#include "SHIN/Actors/TFT_CoinOrb.h"
#include "Engine/World.h"

UTFT_ItemRewardSpawnerComponent::UTFT_ItemRewardSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ItemOrbClass = ATFT_ItemOrb::StaticClass();
	CoinOrbClass = ATFT_CoinOrb::StaticClass();
}

void UTFT_ItemRewardSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTFT_ItemRewardSpawnerComponent::ClearSpawnedRewards()
{
	for (AActor* RewardActor : SpawnedRewardActors)
	{
		if (IsValid(RewardActor))
		{
			RewardActor->Destroy();
		}
	}

	SpawnedRewardActors.Empty();
}

void UTFT_ItemRewardSpawnerComponent::SpawnStageItemRewards(int32 StageIndex)
{
	ClearSpawnedRewards();

	if (!GetWorld())
	{
		return;
	}

	if (!ItemOrbClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemRewardSpawnerComponent: ItemOrbClass is not set."));
		return;
	}
	
	if (StageIndex >= 3)
	{
		StageIndex = 3; // 최대 3개까지만 보상 오브를 스폰하도록 제한
	}

	const FVector SpawnLocation = FVector::ZeroVector + FVector(0, 0, 130.0f);
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	// 아이템 구체
	for (int32 i = 0; i < StageIndex; ++i)
	{
		if (ATFT_ItemOrb* SpawnedOrb = GetWorld()->SpawnActor<ATFT_ItemOrb>(ItemOrbClass, SpawnLocation, SpawnRotation))
		{
			SpawnedRewardActors.Add(SpawnedOrb);
		}
	}
	
	if (StageIndex == 0) return; // 0스테이지 일때는 코인 구체도 스폰하지 않음
	
	// 코인 구체 (2개씩 고정 생성)
	for (int32 i = 0; i < 2; ++i)
	{
		if (ATFT_CoinOrb* SpawnedCoinOrb = GetWorld()->SpawnActor<ATFT_CoinOrb>(CoinOrbClass, SpawnLocation, SpawnRotation))
		{
			SpawnedRewardActors.Add(SpawnedCoinOrb);
		}
	}
}