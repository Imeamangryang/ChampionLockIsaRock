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
 
};
