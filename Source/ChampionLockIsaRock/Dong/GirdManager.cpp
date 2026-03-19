// Fill out your copyright notice in the Description page of Project Settings.


#include "Dong/GirdManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AGirdManager::AGirdManager()
{
	HexGridISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HexGridISM"));
	RootComponent = HexGridISM;
	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void AGirdManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (HexGridISM->GetNumMaterials() > 0)
	{
		GridMaterialMID = HexGridISM->CreateDynamicMaterialInstance(0);
	}
	
}

// Called every frame
void AGirdManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGirdManager::ToggleGridVisibility(bool bShowGrid)
{
	if (GridMaterialMID)
	{
		float TargetOpacity = bShowGrid ? 1.0f : 0.0f;
		
		GridMaterialMID->SetScalarParameterValue(FName("GridOpacity"), TargetOpacity);
	}
}

