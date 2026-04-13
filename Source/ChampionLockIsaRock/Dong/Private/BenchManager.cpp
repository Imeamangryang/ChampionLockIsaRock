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
	
	// 시작할 때 9칸의 빈 바구니를 미리 준비합니다.
	BenchUnits.Init(nullptr, MaxBenchSlots);
}


// 대기석 만드는 함수
void ABenchManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
    
	if (!BenchISM) return;

	BenchISM->ClearInstances(); 
	
	// 대기석은 지그재그가 아니라 가로로 쭉 한 줄만 깝니다.
	for (int32 Col = 0; Col < MaxBenchSlots; ++Col)
	{
		float XPos = ( (MaxBenchSlots - 1) - Col ) * (TileSize + TileGap);
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
            
			// 2. 마우스와 '순수 타일 위치' 사이의 거리를 잽니다. (이래야 정확한 칸이 잡힘)
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
	
	// GridManager에 있던 ISM 찾기 로직 그대로 적용
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


// 1. 가장 왼쪽의 빈 칸 찾기
int32 ABenchManager::GetFirstEmptySlotIndex()
{
	for (int32 i = 0; i < BenchUnits.Num(); i++)
	{
		if (BenchUnits[i] == nullptr) // nullptr이면 비어있다는 뜻!
		{
			return i; // 찾자마자 해당 번호 반환
		}
	}
	return -1; // 9칸 다 돌았는데 nullptr이 없으면 꽉 찬 상태
}

// 2. 해당 칸에 기물 등록
void ABenchManager::RegisterUnitToSlot(int32 Index, AActor* Unit)
{
	if (BenchUnits.IsValidIndex(Index))
	{
		BenchUnits[Index] = Unit;
		UE_LOG(LogTemp, Warning, TEXT("%d번 대기석에 기물이 등록되었습니다."), Index);
	}
}
 
// 3. 특정 인덱스의 월드 좌표 구하기
FVector ABenchManager::GetSlotWorldLocation(int32 Index)
{
	return GetBenchSlotCenterByIndex(Index);
}

// 4. 인덱스 번호로 타일 중앙 좌표 가져오기
FVector ABenchManager::GetBenchSlotCenterByIndex(int32 Index)
{
	if (!BenchISM || !BenchUnits.IsValidIndex(Index)) return GetActorLocation();

	FTransform InstanceTransform;
	if (BenchISM->GetInstanceTransform(Index, InstanceTransform, true))
	{
		// 타일 위치 + 사용자 지정 오프셋을 더해 반환
		return InstanceTransform.GetLocation() + BenchTileOffset;
	}
	return GetActorLocation();
}

// 5. 대기석 유닛 데이터 삭제
void ABenchManager::ClearUnitFromBench(AActor* TargetUnit)
{
	if (!TargetUnit) return; // 애초에 지울 대상이 없으면 리턴

	// 대기석 배열을 싹 뒤져서 이 유닛이 앉아있는 '의자'를 찾습니다.
	for (int32 i = 0; i < BenchUnits.Num(); i++)
	{
		if (BenchUnits[i] == TargetUnit)
		{
			BenchUnits[i] = nullptr; // 의자를 부수지 않고, 빈자리로만 만듭니다
			UE_LOG(LogTemp, Log, TEXT("%d번 대기석 칸이 비워졌습니다."), i);
          
			return; // 찾아서 지웠으니 더 뒤질 필요 없이 함수를 끝냅니다.
		}
	}
}

int32 ABenchManager::GetClosestBenchSlotIndex(FVector WorldLocation)
{
	if (!BenchISM) return -1;

	int32 ClosestIndex = -1;
	float MinDistanceSq = 1e+20f;

	int32 InstanceCount = BenchISM->GetInstanceCount();
	for (int32 i = 0; i < InstanceCount; ++i)
	{
		FTransform InstanceTransform;
		if (BenchISM->GetInstanceTransform(i, InstanceTransform, true))
		{
			// 타일의 순수 위치에 오프셋을 더한 '실제 보드 위 좌표' 계산
			FVector ActualTileLoc = InstanceTransform.GetLocation() + BenchTileOffset;
          
			float DistSq = FVector::DistSquared(WorldLocation, ActualTileLoc);

			if (DistSq < MinDistanceSq)
			{
				MinDistanceSq = DistSq;
				ClosestIndex = i; // 가장 가까운 인덱스 번호를 기억
			}
		}
	}
	return ClosestIndex; // 찾은 인덱스 반환
}