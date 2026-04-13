// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "BenchManager.generated.h"

UCLASS()
class CHAMPIONLOCKISAROCK_API ABenchManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABenchManager();
	
	// 비어있는 가장 왼쪽 칸의 인덱스를 찾는 함수 (0~8, 꽉 차면 -1)
	UFUNCTION(BlueprintCallable, Category = "Bench|Logic")
	int32 GetFirstEmptySlotIndex();

	// 특정 인덱스에 기물을 등록하는 함수
	UFUNCTION(BlueprintCallable, Category = "Bench|Logic")
	void RegisterUnitToSlot(int32 Index, AActor* Unit);

	// 특정 인덱스의 '스냅된' 실제 좌표를 가져오는 함수 (스폰 위치용)
	UFUNCTION(BlueprintCallable, Category = "Bench|Logic")
	FVector GetSlotWorldLocation(int32 Index);
	
	// 인덱스 번호만 넣으면 해당 타일의 정중앙(오프셋 포함) 좌표를 돌려주는 함수
	UFUNCTION(BlueprintCallable, Category = "Bench Grid")
	FVector GetBenchSlotCenterByIndex(int32 Index);
	
	UFUNCTION(BlueprintCallable, Category = "TFT|Bench")
	void ClearUnitFromBench(AActor* TargetUnit);

	// 그리드와 똑같은 투명화 함수
	void ToggleBenchVisibility(bool bShowBench);
    
	// 그리드와 똑같은 스냅 좌표 구하기 함수
	FVector GetSnappedBenchLocation(FVector WorldLocation);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bench")
	UInstancedStaticMeshComponent* BenchISM;

	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// 특정 월드 좌표와 가장 가까운 대기석의 인덱스 번호를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "TFT|Bench")
	int32 GetClosestBenchSlotIndex(FVector WorldLocation);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench Grid")
	int32 MaxBenchSlots = 9; // 대기석 칸 수 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench Grid")
	float TileSize = 100.0f; // 타일 크기

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench Grid")
	float TileGap = 15.0f; // 타일 간격

	// 타일 중심점을 미세조정하기 위한 오프셋 (에디터에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench Grid")
	FVector BenchTileOffset = FVector(50.0f, 75.0f, 0.0f);
	
	// 대기석 9칸의 데이터를 담는 배열 (기물이 없으면 nullptr)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bench|Data")
	TArray<AActor*> BenchUnits;
};

