// Fill out your copyright notice in the Description page of Project Settings.

#include "Dong/Public/TopDownController.h"
#include "Dong/Public/BenchManager.h"
#include "Dong/Public/GirdManager.h"
#include "Dong/Public/UnitManager.h"
#include "Engine/HitResult.h"
#include "EngineUtils.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../SHIN/Character/TFT_UnitCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "../SHIN/Subsystem/TFT_UISubsystem.h"

ATopDownController::ATopDownController()
{
    bShowMouseCursor = true;
    PrimaryActorTick.bCanEverTick = true;
}
 
void ATopDownController::BeginPlay()
{
    Super::BeginPlay();
	
	// 1. 월드에서 "BoardCamera" 태그를 가진 액터를 찾습니다.
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("BoardCamera"), FoundActors);

	if (FoundActors.Num() > 0)
	{
		BoardCameraActor = FoundActors[0];
        
		// 2. [핵심] 화면 시점을 이 카메라로 고정합니다!
		SetViewTarget(BoardCameraActor);
	}

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
       Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }
    
    // [최적화] 매 프레임 매니저를 찾는 비용을 없애기 위해 시작 시 한 번만 캐싱(저장)
    CachedGridManager = Cast<AGirdManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGirdManager::StaticClass()));
    CachedBenchManager = Cast<ABenchManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABenchManager::StaticClass()));
    
    SaveInitialLocations();
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetPawn();

    // [버그 방지] 컴포넌트 꼬임 현상을 막기 위해 하이라이트 액터를 월드에 독립적으로 스폰
    HighlightActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::Zero(), FRotator::ZeroRotator, SpawnParams);

    if (HighlightActor)
    {
       UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(HighlightActor, TEXT("HighlightMesh"));
       if (MeshComp)
       {
          MeshComp->RegisterComponent();
          HighlightActor->SetRootComponent(MeshComp);
          
          // 하이라이트가 마우스 레이저(LineTrace)를 가로막지 않도록 충돌 완전 해제
          MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
          MeshComp->SetCastShadow(false);
          MeshComp->SetMobility(EComponentMobility::Movable);
          HighlightActor->SetActorHiddenInGame(true);
       }
    }
}

void ATopDownController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
       EnhancedInputComponent->BindAction(IA_Grabbed, ETriggerEvent::Started, this, &ATopDownController::OnGrabPressed);
       EnhancedInputComponent->BindAction(IA_Grabbed, ETriggerEvent::Completed, this, &ATopDownController::OnDropReleased);
       EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATopDownController::OnMoveInputReleased);
    	EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ATopDownController::OnZoom);
    }
}

void ATopDownController::OnZoom(const FInputActionValue& Value)
{
	float ZoomValue = Value.Get<float>();
	
	if (BoardCameraActor)
	{
		USpringArmComponent* SpringArm = BoardCameraActor->FindComponentByClass<USpringArmComponent>();
		if (SpringArm)
		{
			float NewLength = SpringArm->TargetArmLength + (ZoomValue * 100.0f);
			SpringArm->TargetArmLength = FMath::Clamp(NewLength, 100.0f, 1200.0f);
		}
	}
}

void ATopDownController::OnGrabPressed(const FInputActionValue& Value)
{
    if (bIsHoldingUnit)
    {
       PerformDrop();
       return;
    }
    
    FHitResult HitResult;
    if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
    {
       AActor* HitActor = HitResult.GetActor();
       ATFT_UnitCharacter* TargetUnit = Cast<ATFT_UnitCharacter>(HitActor);

       if (TargetUnit)
       {
          GrabbedUnit = TargetUnit;
          OriginalLocation = GrabbedUnit->GetActorLocation();

          FVector LiftedLocation = OriginalLocation;
          LiftedLocation.Z += 300.0f;
          GrabbedUnit->SetActorLocation(LiftedLocation);
          
          // 기물을 들고 있는 동안 해당 기물이 레이저를 가리지 않도록 충돌을 꺼줌
          GrabbedUnit->SetActorEnableCollision(false);

          bIsHoldingUnit = true;
          GrabStartTime = GetWorld()->GetTimeSeconds(); // 클릭 vs 드래그 판정을 위한 시간 기록

          if (CachedGridManager) CachedGridManager->ToggleGridVisibility(true);
          if (CachedBenchManager) CachedBenchManager->ToggleBenchVisibility(true);
       }
    }
}

void ATopDownController::OnDropReleased(const FInputActionValue& Value)
{
    if (!bIsHoldingUnit || !GrabbedUnit) return;
    
    // [UX] 0.2초 미만은 단순 클릭으로 간주하여 기물을 계속 들고 있게 유지함
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if ((CurrentTime - GrabStartTime) < 0.2f) return; 
    
    PerformDrop();
}

void ATopDownController::PerformDrop()
{
    if (!bIsHoldingUnit || !GrabbedUnit) return;

    FHitResult HitResult;
    // 챔피언 몸통을 무시하고 순수한 타일 바닥 좌표를 얻기 위해 ECC_WorldStatic 채널 사용
    if (GetHitResultUnderCursor(ECC_WorldStatic, false, HitResult))
    {
        FVector MouseLoc = HitResult.Location;
        FVector FinalSnappedLoc = OriginalLocation;
        bool bIsValidDrop = false;
        bool bGoingToBench = false;

        FVector ClosestGridLoc = CachedGridManager ? CachedGridManager->GetSnappedLocation(MouseLoc) : FVector::ZeroVector;
        FVector ClosestBenchLoc = CachedBenchManager ? CachedBenchManager->GetSnappedBenchLocation(MouseLoc) : FVector::ZeroVector;

        float DistToGrid = CachedGridManager ? FVector::Dist2D(MouseLoc, ClosestGridLoc) : 999999.0f;
        float DistToBench = CachedBenchManager ? FVector::Dist2D(MouseLoc, ClosestBenchLoc) : 999999.0f;

        // [조작감] 벤치는 타일이 좁으므로 판정 거리를 1.5배로 늘려 쉽게 스냅되도록 설정
        if (CachedBenchManager && DistToBench <= (CachedBenchManager->TileSize * 1.5f) && DistToBench < DistToGrid)
        {
            FinalSnappedLoc = ClosestBenchLoc;
            bGoingToBench = true;
            bIsValidDrop = true;
        }
        else if (CachedGridManager && DistToGrid <= (CachedGridManager->HexSize * 1.5f))
        {
            FinalSnappedLoc = ClosestGridLoc;
            bGoingToBench = false;
            bIsValidDrop = true;
        }

    	// 5. 유효한 타일에 놓았을 때
    	if (bIsValidDrop)
    	{
    		// [무조건 실행] 일단 어디에 있든 대기석과 그리드 장부에서 이 유닛을 지웁니다 (이동 전 세탁)
    		if (CachedBenchManager) CachedBenchManager->ClearUnitFromBench(GrabbedUnit);
    		if (CachedGridManager) CachedGridManager->ClearUnitFromGrid(GrabbedUnit);
   
    		FVector DropLoc = FVector(FinalSnappedLoc.X, FinalSnappedLoc.Y, OriginalLocation.Z);
    		TArray<AActor*> OverlappedActors;
    		UKismetSystemLibrary::SphereOverlapActors(GetWorld(), DropLoc, 60.0f, {UEngineTypes::ConvertToObjectType(ECC_Pawn)}, ATFT_UnitCharacter::StaticClass(), {GrabbedUnit}, OverlappedActors);
    		
    		if (OverlappedActors.Num() > 0)
    		{
    			// 서로의 위치를 맞바꿈
    			ATFT_UnitCharacter* OtherUnit = Cast<ATFT_UnitCharacter>(OverlappedActors[0]);
      
    			// 상대방 유닛도 장부에서 일단 지워줍니다 (그래야 꼬이지 않음)
    			if (CachedBenchManager) CachedBenchManager->ClearUnitFromBench(OtherUnit);
    			if (CachedGridManager) CachedGridManager->ClearUnitFromGrid(OtherUnit);

    			FVector OtherUnitLoc = OtherUnit->GetActorLocation();
    			GrabbedUnit->SetActorLocation(FVector(OtherUnitLoc.X, OtherUnitLoc.Y, OriginalLocation.Z));
    			OtherUnit->SetActorLocation(OriginalLocation);

    			// 상대방이 원래 내가 있던 자리(OriginalLocation)로 갔으니, 그 위치에 맞게 다시 등록
    			// (이 부분은 'OriginalLocation'이 벤치였는지 그리드였는지 판별하는 로직이 필요하지만, 
    			// 일단은 GrabbedUnit의 등록 로직을 따라가면 해결됩니다.)
      
    			// 스왑된 두 기물의 주소록 최신화
    			UnitHomeRegistry.Add(GrabbedUnit, GrabbedUnit->GetActorTransform());
    			UnitHomeRegistry.Add(OtherUnit, OtherUnit->GetActorTransform());
    		}
    		else
    		{
    			// 빈칸 이동 로직
    			GrabbedUnit->SetActorLocation(DropLoc);
    		}
    		
    		// [최종 등록] 이제 GrabbedUnit이 최종적으로 도착한 곳에 따라 장부에 기입!
    		if (bGoingToBench && CachedBenchManager)
    		{
    			int32 NewSlotIndex = CachedBenchManager->GetFirstEmptySlotIndex();
    			if (NewSlotIndex != -1)
    			{
    				CachedBenchManager->RegisterUnitToSlot(NewSlotIndex, GrabbedUnit);
    			}
    		}
    		else if (!bGoingToBench && CachedGridManager)
    		{
    			// [이게 꼭 불려야 공격을 함!] 그리드 장부에 등록
    			CachedGridManager->RegisterUnitToGrid(GrabbedUnit);
    		}
   
    		// 주소록 최신화
    		UnitHomeRegistry.Add(GrabbedUnit, GrabbedUnit->GetActorTransform());
    	}
        else
        {
            // 허공에 놓았을 때 제자리로 튕겨냄
            GrabbedUnit->SetActorLocation(OriginalLocation);
        }
    }

    // 상태 초기화 및 시각적 요소(가이드라인, 하이라이트) 숨김
    if (HighlightActor) HighlightActor->SetActorHiddenInGame(true);
    if (CachedGridManager) CachedGridManager->ToggleGridVisibility(false);
    if (CachedBenchManager) CachedBenchManager->ToggleBenchVisibility(false);
    
    GrabbedUnit->SetActorEnableCollision(true); // 충돌 원상복구
    bIsHoldingUnit = false;
    GrabbedUnit = nullptr;
}

void ATopDownController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsHoldingUnit && GrabbedUnit)
    {
       FHitResult HitResult;
       if (GetHitResultUnderCursor(ECC_WorldStatic, false, HitResult))
       {
          FVector MouseWorldLocation = HitResult.Location;
          // 들고 있는 기물을 공중(Z)에 고정시킨 채 마우스를 따라가게 함
          FVector NewLocation = FVector(MouseWorldLocation.X, MouseWorldLocation.Y, 270.0f);
          
          GrabbedUnit->SetActorLocation(NewLocation);
          UpdateHighlightPosition(MouseWorldLocation); // 미리보기 하이라이트 갱신
       }
    }
    else
    {
       // 기물을 들고 있지 않은데 하이라이트가 켜져있다면 강제로 끔
       if (HighlightActor && !HighlightActor->IsHidden())
       {
          HighlightActor->SetActorHiddenInGame(true);
       }
    }
}

// =========================================================================
// 이동(Move), 초기화(Save/Return) 로직 복구
// =========================================================================

// 우클릭(이동 액션)을 뗐을 때 이펙트 재생
void ATopDownController::OnMoveInputReleased()
{
	UTFT_UISubsystem* UISubsystem = nullptr;
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		UISubsystem = LP->GetSubsystem<UTFT_UISubsystem>();
	}

	// 1. 먼저 유닛 클릭 여부 확인
	FHitResult UnitHit;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	if (GetHitResultUnderCursorForObjects(ObjectTypes, true, UnitHit))
	{
		if (AActor* HitActor = UnitHit.GetActor())
		{
			if (HitActor->IsA<ATFT_UnitCharacter>())
			{
				UISubsystem->BroadcastStatUIOpen(true, Cast<ATFT_UnitCharacter>(HitActor)->ChampionData, Cast<ATFT_UnitCharacter>(HitActor));
				return;
			}
		}
	}

	// 아니면  UI 닫기
	UISubsystem->BroadcastStatUIOpen(false, FStruct_TFT_Champion{}, nullptr);
	
	FHitResult GroundHit;
	if (GetHitResultUnderCursor(ECC_WorldStatic, true, GroundHit))
	{
		CachedDestination = GroundHit.Location;
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);

		if (FXCursor)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination);
		}
	}
}

// 게임 시작 시 레벨에 배치된 모든 기물의 위치를 TMap(주소록)에 등록
void ATopDownController::SaveInitialLocations()
{
    // 월드 내 모든 ATFT_UnitCharacter 클래스를 순회하는 이터레이터
    for (TActorIterator<ATFT_UnitCharacter> It(GetWorld()); It; ++It)
    {
       ATFT_UnitCharacter* Unit = *It;
       if (IsValid(Unit))
       {
          // 유닛 포인터를 Key로, 현재 위치/회전(Transform)을 Value로 저장
          UnitHomeRegistry.Add(Unit, Unit->GetActorTransform());
       }
    }
}

// 전투가 끝났을 때 모든 유닛을 UnitHomeRegistry에 저장된 위치로 즉시 복귀시킴
void ATopDownController::ReturnAllUnitsToHome()
{
    // TMap에 등록된 모든 유닛을 하나씩 꺼내어 처리
    for (auto& Elem : UnitHomeRegistry)
    {
       ATFT_UnitCharacter* Unit = Elem.Key;
       FTransform HomeTransform = Elem.Value;

       if (IsValid(Unit))
       {
          // 저장된 초기 데이터(Transform)를 유닛에게 다시 적용
          Unit->SetActorTransform(HomeTransform);
       }
    }
}

// =========================================================================
// 2. 하이라이트 및 상태 확인 로직
// =========================================================================

// 마우스 좌표를 기반으로 하이라이트 액터의 메쉬 종류와 위치를 결정
void ATopDownController::UpdateHighlightPosition(FVector MouseLoc)
{
    if (!HighlightActor) return;
    
    UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(HighlightActor->GetRootComponent());
    if (!MeshComp || !CachedGridManager || !CachedBenchManager) return;

    // 그리드와 벤치 매니저로부터 가장 가까운 스냅 좌표를 받아옴
    FVector ClosestGridLoc = CachedGridManager->GetSnappedLocation(MouseLoc);
    FVector ClosestBenchLoc = CachedBenchManager->GetSnappedBenchLocation(MouseLoc);

    // 마우스와 각 타일 사이의 평면(2D) 거리를 계산
    float DistToGrid = FVector::Dist2D(MouseLoc, ClosestGridLoc);
    float DistToBench = FVector::Dist2D(MouseLoc, ClosestBenchLoc);

    // 판정 범위 설정 (그리드 1.5배, 벤치 2.0배)
    float GridThreshold = CachedGridManager->HexSize * 1.5f; 
    float BenchThreshold = CachedBenchManager->TileSize * 2.0f; 

    FVector FinalLocation = FVector::ZeroVector;
    bool bIsDetected = false;
    bool bGoingToBench = false;

    // [우선순위] 벤치 범위 안에 있으면 벤치로 판정, 아니면 그리드 판정
    if (DistToBench <= BenchThreshold && DistToBench < DistToGrid)
    {
    	FinalLocation = ClosestBenchLoc + FVector(0.0f, 0.0f, HighlightZOffset); 
    	bIsDetected = true;
    	bGoingToBench = true;
    }
    else if (DistToGrid <= GridThreshold)
    {
       FinalLocation = ClosestGridLoc + FVector(0, 0, 20.0f);
       bIsDetected = true;
       bGoingToBench = false;
    }
	// 판정 결과에 따라 하이라이트 메쉬(사각형/육각형)를 실시간으로 교체
    if (bIsDetected)
    {
       // 판정 결과에 따라 하이라이트 메쉬(사각형/육각형)를 실시간으로 교체
       UStaticMesh* TargetMesh = bGoingToBench ? BenchHighlightMesh : GridHighlightMesh;
       if (MeshComp->GetStaticMesh() != TargetMesh)
       {
          MeshComp->SetStaticMesh(TargetMesh);
       }

       HighlightActor->SetActorLocation(FinalLocation);
       HighlightActor->SetActorHiddenInGame(false);
    }
    else
    {
       // 범위를 벗어나면 숨김
       HighlightActor->SetActorHiddenInGame(true);
    }
}

void ATopDownController::SpawnUnitFromBP()
{
	// UnitClass(스폰할 BP)가 안 비어있는지, 대기석 매니저가 있는지 확인
	if (!UnitClass || !CachedBenchManager) return;

	// Get First Empty Slot Index
	int32 EmptyIndex = CachedBenchManager->GetFirstEmptySlotIndex();

	// Branch (Condition: Index > -1)
	if (EmptyIndex > -1)
	{
		FVector BenchSlotLoc = CachedBenchManager->GetBenchSlotCenterByIndex(EmptyIndex);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; 
        
		// 스폰할 때도 바뀐 이름(BenchSlotLoc)을 넣어줍니다.
		ATFT_UnitCharacter* SpawnedUnit = GetWorld()->SpawnActor<ATFT_UnitCharacter>(UnitClass, BenchSlotLoc, FRotator::ZeroRotator, SpawnParams);

		if (SpawnedUnit)
		{
			CachedBenchManager->RegisterUnitToSlot(EmptyIndex, SpawnedUnit);

			AUnitManager* UnitManager = Cast<AUnitManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AUnitManager::StaticClass()));
			if (UnitManager)
			{
				// 스폰된 유닛의 챔피언 키와 성급(기본 1성)으로 합체 체크
				UnitManager->TryUpgradeUnit(SpawnedUnit->ChampionKey, 1);
			}

			// 컨트롤러 주소록에 등록
			UnitHomeRegistry.Add(SpawnedUnit, SpawnedUnit->GetActorTransform());
		}
	}
	else
	{
		// Branch False -> Print String
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("대기석이 꽉 찼습니다."));
	}
}

bool ATopDownController::IsUnitOnBench(AActor* Unit) const
{
	// 대기석 매니저한테 "얘 네 명단에 아직 있냐?" 라고 물어봄
	if (CachedBenchManager && Unit)
	{
		return CachedBenchManager->BenchUnits.Contains(Unit);
	}
	return false;
}