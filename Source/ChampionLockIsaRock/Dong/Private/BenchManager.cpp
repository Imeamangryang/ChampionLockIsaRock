// Fill out your copyright notice in the Description page of Project Settings.


#include "Dong/Public/BenchManager.h"


// Sets default values
ABenchManager::ABenchManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// 대기석 타일들을 담을 빈 바구니(컴포넌트)를 만들고, 이 액터의 중심으로 설정합니다.
	BenchISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BenchISM"));
	RootComponent = BenchISM;
}

// 에디터에서 값을 바꾸거나 액터를 옮길 때마다 실행되는 함수
void ABenchManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
    
	if (!BenchISM) return;

	BenchISM->ClearInstances(); 

	// 대기석은 지그재그가 아니라 가로로 쭉 한 줄만 깝니다.
	for (int32 Col = 0; Col < MaxBenchSlots; ++Col)
	{
		float XPos = Col * (TileSize + TileGap);
		float YPos = 0.0f; // 한 줄이니까 Y는 고정

		FTransform NewTransform;
		NewTransform.SetLocation(FVector(XPos, YPos, 0.0f));
		BenchISM->AddInstance(NewTransform);
	}
}

// 마우스 좌표를 주면, 대기석 타일 중 가장 가까운 타일의 정중앙 좌표를 돌려주는 함수
FVector ABenchManager::GetSnappedBenchLocation(FVector WorldLocation)
{
	if (!BenchISM) return WorldLocation;

	int32 ClosestIndex = INDEX_NONE;
	float MinDistanceSq = 1e+20f;
	// 타일 좌표를 저장할 변수
	FVector ClosestBaseLocation = WorldLocation;

	int32 InstanceCount = BenchISM->GetInstanceCount();
	for (int32 i = 0; i < InstanceCount; ++i)
	{
		FTransform InstanceTransform;
		if (BenchISM->GetInstanceTransform(i, InstanceTransform, true))
		{
			// 1. 타일의 처음 위치(0,0,0)를 가져옵니다.
			FVector PureInstanceLoc = InstanceTransform.GetLocation();
            
			// 2. 마우스와 '순수 타일 위치' 사이의 거리를 잽니다. (이래야 정확한 칸이 잡힙니다)
			float DistSq = FVector::DistSquared(WorldLocation, PureInstanceLoc);

			if (DistSq < MinDistanceSq)
			{
				MinDistanceSq = DistSq;
				
				// 가장 가까운 타일의 '순수 좌표'를 일단 확보합니다.
				ClosestBaseLocation = PureInstanceLoc;
			}
		}
	}

	// 3. 어떤 타일인지 찾는 작업이 다 끝난후에 그 타일의 순수 좌표에 
	// 우리가 원하는 보정값(BenchTileOffset)을 딱 더해서 돌려줍니다.
	return ClosestBaseLocation + BenchTileOffset;
}

// Called when the game starts or when spawned
void ABenchManager::BeginPlay()
{
	Super::BeginPlay();
	
	// ★ GridManager에 있던 완벽한 ISM 찾기 로직 그대로 적용!
	TArray<UInstancedStaticMeshComponent*> ISMComponents;
	GetComponents<UInstancedStaticMeshComponent>(ISMComponents);

	for (UInstancedStaticMeshComponent* TargetISM : ISMComponents)
	{
		if (TargetISM && (TargetISM->GetStaticMesh() != nullptr || TargetISM->GetInstanceCount() > 0))
		{
			BenchISM = TargetISM; 
			UE_LOG(LogTemp, Warning, TEXT("진짜 대기석 ISM을 찾아서 연결했습니다: %s"), *TargetISM->GetName());
			break;
		}
	}

	// 시작할 때 무조건 꺼둡니다.
	ToggleBenchVisibility(false);
	
}

void ABenchManager::ToggleBenchVisibility(bool bShowBench)
{
	if (BenchISM)
	{
		BenchISM->SetHiddenInGame(!bShowBench); // GridManager와 동일한 방식!
	}
}

// Called every frame
void ABenchManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

