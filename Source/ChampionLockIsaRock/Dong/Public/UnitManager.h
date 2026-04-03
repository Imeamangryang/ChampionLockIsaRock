// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../../../../../Program Files/Epic Games/UE_5.7/Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "GameFramework/Actor.h"
#include "UnitManager.generated.h"


class AGirdManager;
class ABenchManager;
class ATFT_UnitCharacter;
enum class ETFT_ChampionKey : uint8;

UCLASS()
class CHAMPIONLOCKISAROCK_API AUnitManager : public AActor
{
	GENERATED_BODY()

public:
	AUnitManager();
	virtual void Tick(float DeltaTime) override;

	// 기물 합체 체크 (외부 호출용)
	UFUNCTION(BlueprintCallable, Category = "TFT|Evolution")
	void TryUpgradeUnit(ETFT_ChampionKey UnitKey, int32 StarLevel);

protected:
	virtual void BeginPlay() override;

	// 합체 완료 시 데이터 처리 및 파괴
	void FinishUpgrade(ATFT_UnitCharacter* TargetUnit, ATFT_UnitCharacter* F1, ATFT_UnitCharacter* F2, ETFT_ChampionKey UnitKey, int32 NewStarLevel);

	// [이펙트용] 나중에 파티클 터뜨릴 때 사용
	UFUNCTION(BlueprintImplementableEvent, Category = "TFT|Evolution")
	void BP_OnFusionComplete(ATFT_UnitCharacter* FinalUnit);
 
private:
	// 매니저 참조
	UPROPERTY() ABenchManager* BenchManager; 
	UPROPERTY() AGirdManager* GirdManager;

	// 애니메이션 상태 변수
	bool bIsEvolving = false;
	float EvolutionAlpha = 0.0f;
	const float EvolutionSpeed = 2.5f; // 숫자가 클수록 빨라집니다

	// 가비지 컬렉터 보호를 위해 UPROPERTY() 유지
	UPROPERTY() ATFT_UnitCharacter* MainUnit_Ptr;
	UPROPERTY() ATFT_UnitCharacter* Fodder1_Ptr;
	UPROPERTY() ATFT_UnitCharacter* Fodder2_Ptr;

	FVector F1_StartLoc;
	FVector F2_StartLoc;
	ETFT_ChampionKey PendingKey;
	int32 PendingLevel;
};
