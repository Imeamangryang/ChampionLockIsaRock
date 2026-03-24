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

	// 대기석은 한 줄이니까 칸 수만 정해줍니다 (TFT는 보통 9칸)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench Grid")
	int32 MaxBenchSlots = 9; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench Grid")
	float TileSize = 100.0f; // 타일 크기

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench Grid")
	float TileGap = 15.0f; // 타일 간격

	// 타일 중심점을 미세조정하기 위한 오프셋 (에디터에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench Grid")
	FVector BenchTileOffset = FVector(50.0f, 75.0f, 0.0f);
};
