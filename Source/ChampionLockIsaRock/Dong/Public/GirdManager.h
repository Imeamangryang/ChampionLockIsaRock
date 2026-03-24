// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GirdManager.generated.h"

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

	// ★ 질문자님이 원하시던 바로 그 기능! 간격 조절 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
	float TileGap = 15.0f;
 
};
