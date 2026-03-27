// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GirdManager.generated.h"

class ATFT_UnitCharacter;
class ATopDownController;

UCLASS()
class CHAMPIONLOCKISAROCK_API AGirdManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGirdManager();
	
	void ToggleGridVisibility(bool bShowGrid);
	
	FVector GetSnappedLocation(FVector WorldLocation);
	 
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gird")
	UInstancedStaticMeshComponent* HexGridISM;
	
	UPROPERTY()
	UMaterialInstanceDynamic* GridMaterialMID;
	
	// 실제로 필드에 서 있는 유닛들을 담아둘 명단
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid|Data")
	TArray<class ATFT_UnitCharacter*> DeployedUnits;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	// 블루프린트의 Construction Script 역할을 하는 함수
	virtual void OnConstruction(const FTransform& Transform) override;

	// 에디터에서 값을 마음대로 바꿀 수 있게 노출합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
	int32 Rows = 4; // 줄 수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
	int32 Columns = 7; // 칸 수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
	float HexSize = 100.0f; // 육각형 크기 (반지름)

	// 간격 조절 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
	float TileGap = 15.0f;
	
	// 필드에 배치된 유닛 리스트 반환
	TArray<class ATFT_UnitCharacter*> GetDeployedUnits();
	
	// 필드 기물 등록 함수 (나중에 드롭할 때 사용)
	void RegisterUnitToGrid(ATFT_UnitCharacter* Unit);
	
	// 그리드 데이터에서 유닛 제거
	void ClearUnitFromGrid(ATFT_UnitCharacter* TargetUnit);
};
