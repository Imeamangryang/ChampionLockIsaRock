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
	
	// [핵심!] 이 액터에 붙어있는 모든 ISM 중에서 '진짜 메쉬가 들어있는 놈'을 찾습니다.
	TArray<UInstancedStaticMeshComponent*> ISMComponents;
	GetComponents<UInstancedStaticMeshComponent>(ISMComponents);

	for (UInstancedStaticMeshComponent* TargetISM : ISMComponents)
	{
		// 메쉬가 할당되어 있고 인스턴스가 0개보다 많은 녀석이 '진짜'입니다.
		if (TargetISM && (TargetISM->GetStaticMesh() != nullptr || TargetISM->GetInstanceCount() > 0))
		{
			HexGridISM = TargetISM; // C++ 변수를 실제 작동하는 컴포넌트로 연결!
			UE_LOG(LogTemp, Warning, TEXT("진짜 그리드 ISM을 찾아서 연결했습니다: %s"), *TargetISM->GetName());
			break;
		}
	}

	// 시작할 때 무조건 꺼둡니다.
	ToggleGridVisibility(false);
	
}

// Called every frame
void AGirdManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 
}

void AGirdManager::ToggleGridVisibility(bool bShowGrid)
{
	if (HexGridISM)
	{
		HexGridISM->SetHiddenInGame(!bShowGrid);
	}
}

FVector AGirdManager::GetSnappedLocation(FVector WorldLocation)
{
	if (!HexGridISM) return WorldLocation;

	int32 ClosestIndex = INDEX_NONE;
	float MinDistanceSq = 1e+20f; // 아주 큰 값으로 시작
	FVector ClosestLocation = WorldLocation;

	// 1. 모든 인스턴스(타일)를 하나씩 검사합니다.
	int32 InstanceCount = HexGridISM->GetInstanceCount();
	for (int32 i = 0; i < InstanceCount; ++i)
	{
		FTransform InstanceTransform;
		// 2. i번째 타일의 위치 정보를 가져옵니다. (WorldSpace = true)
		if (HexGridISM->GetInstanceTransform(i, InstanceTransform, true))
		{
			FVector InstanceLoc = InstanceTransform.GetLocation();
            
			// 3. 현재 마우스 위치와 타일 중심 사이의 거리를 계산합니다.
			// (성능을 위해 루트를 씌우지 않은 제곱 거리 DistSquared를 사용합니다)
			float DistSq = FVector::DistSquared(WorldLocation, InstanceLoc);

			// 4. 지금까지 찾은 것 중에 가장 가깝다면 정보를 갱신합니다.
			if (DistSq < MinDistanceSq)
			{
				MinDistanceSq = DistSq;
				ClosestIndex = i;
				ClosestLocation = InstanceLoc;
			}
		}
	}

	// 최종적으로 가장 가까웠던 타일의 중심 좌표를 돌려줍니다.
	return ClosestLocation;
}

