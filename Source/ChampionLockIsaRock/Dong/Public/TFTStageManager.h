// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SHIN/Struct/ETFT_ChampionList.h"
#include "SHIN/Struct/ETFT_ItemList.h"
#include "TFTStageManager.generated.h"
 
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStagePreparationStartedSignature);

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ATFT_UnitCharacter> EnemyClass;

	// 1. 팀원이 만든 Enum으로 타입 변경 에디터에서 드롭다운으로 편하게 고를 수 있게 됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETFT_ChampionKey ChampionKey = ETFT_ChampionKey::Garen;

	// 2. 적군이 1성인지 2성인지도 설정할 수 있게 추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StarLevel = 1; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MakeEditWidget = true))
	FVector EnemySpawnLocations;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ETFT_ItemKey> EquippedItems;
};
// 스테이지 전체 데이터
USTRUCT(BlueprintType)
struct FStageData
{
		GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StageName = TEXT("1-1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEnemySpawnInfo> Enemies;
};

// 스테이지 상황
UENUM(BlueprintType)
enum class EStageStatus : uint8
{
	NotStarted  UMETA(DisplayName = "시작 안 함"),
	Current     UMETA(DisplayName = "현재 진행 중"),
	Won         UMETA(DisplayName = "승리"),
	Lost        UMETA(DisplayName = "패배")
};

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFTStageManager : public AActor
{
	GENERATED_BODY()
	
public:
	
	// 각 스테이지의 현재 상태를 기록할 배열
	UPROPERTY(BlueprintReadOnly, Category = "Stage")
	TArray<EStageStatus> StageHistory;
	
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FOnStagePreparationStartedSignature OnPreparationStarted;
	
	// 아군과 적군을 각각 담아둘 명단 (TArray)
	UPROPERTY()
	TArray<class ATFT_UnitCharacter*> ActivePlayerUnits;

	UPROPERTY()
	TArray<class ATFT_UnitCharacter*> ActiveEnemyUnits;
	
	// 지금 전투 중인지 확인하는 스위치
	UPROPERTY(BlueprintReadOnly, Category = "Stage")
	bool bIsCombatActive = false;
	
public:
	// Sets default values for this actor's properties
	ATFTStageManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	// 필드 조작이 제한되는 상태인지 확인 (전투 중이거나 정산 중일 때 true 반환)
	UFUNCTION(BlueprintPure, Category = "Stage")
	bool IsBoardLocked() const;
	
	// 에디터에서 직접 스테이지별 적들을 리스트로 만듭니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	TArray<FStageData> StageDataList;

	UPROPERTY(BlueprintReadOnly, Category = "Stage")
	int32 CurrentStageIndex = 0;

	// 현재 살아있는 아군/적군 리스트
	UPROPERTY()
	TArray<class ATFT_UnitCharacter*> ActivePlayerUnitCharacters;

	UPROPERTY()
	TArray<class ATFT_UnitCharacter*> ActiveEnemyUnitsCharacters;
	
	// 핵심 함수들
	UFUNCTION(BlueprintCallable)
	void StartRound();
	UFUNCTION(BlueprintCallable)
	void EndRound(bool bIsVictory);

	UFUNCTION()
	void OnUnitDied(class ATFT_UnitCharacter* DeadUnit);

private:
	void SetupStage(int32 Index);
	void ResetUnits();
	// 3초 뒤에 실제로 스테이지를 넘길 함수
	void ProceedToNextRound();

	// 승리했는지 정보를 잠시 저장할 변수
	bool bLastRoundVictory;
	// 타이머 핸들
	FTimerHandle StageTransitionTimerHandle;
	
protected:
	// 30초 타이머용 핸들
	FTimerHandle OvertimeTimerHandle;

	float MaxRoundTime = 30.0f; // 기준 시간
	
	// 2배속을 시작하는 함수
	void StartOvertime();
	
	UPROPERTY()
	TArray<AActor*> SpawnedEnemies;
	// 보드를 청소하는 함수 선언
	void CleanupBoard();
	
public:
	// UI에서 호출해서 게이지 퍼센트(0.0 ~ 1.0)를 가져갈 함수
	UFUNCTION(BlueprintPure, Category = "UI")
	float GetRoundTimePct() const;
	// UI에서 "29", "28" 같은 숫자를 가져갈 때 사용
	UFUNCTION(BlueprintPure, Category = "UI")
	int32 GetRemainingRoundTimeInt() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage|Reward")
	class UTFT_ItemRewardSpawnerComponent* ItemRewardSpawnerComponent;
};

