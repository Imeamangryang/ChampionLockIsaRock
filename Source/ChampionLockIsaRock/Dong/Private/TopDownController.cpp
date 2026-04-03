#include "Dong/Public/TopDownController.h"
#include "Dong/Public/BenchManager.h"
#include "Dong/Public/GirdManager.h"
#include "Dong/Public/UnitManager.h"
#include "Dong/Public/TFTPlayerState.h"
#include "Dong/Public/TFTStageManager.h"
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
#include "Blueprint/UserWidget.h"
#include "SHIN/Character/UI/TFT_ItemDragDropOperation.h"
#include "SHIN/Character/Components/TFT_ItemInventoryComponent.h"
#include "SHIN/Character/UI/TFT_ItemDragLayerWidget.h"

ATopDownController::ATopDownController()
{
    bShowMouseCursor = true;
    PrimaryActorTick.bCanEverTick = true;
	
	ItemInventoryComponent = CreateDefaultSubobject<UTFT_ItemInventoryComponent>(TEXT("ItemInventoryComponent"));
	
	static ConstructorHelpers::FClassFinder<UTFT_ItemDragLayerWidget> ItemDragLayerBPClass(TEXT("/Game/SHIN/UI/Blueprints/WBP_ItemDragLayer.WBP_ItemDragLayer_C"));
	if (ItemDragLayerBPClass.Succeeded())
	{
		ItemDragLayerClass = ItemDragLayerBPClass.Class;
	}
}
 
void ATopDownController::BeginPlay()
{
    Super::BeginPlay();
	
	BroadcastUnitCount();
	
	// 1. 월드에서 BoardCamera 태그를 가진 액터를 찾습니다.
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("BoardCamera"), FoundActors);

	if (FoundActors.Num() > 0)
	{
		BoardCameraActor = FoundActors[0];
        
		// 2. 화면 시점을 이 카메라로 고정합니다
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
	
	if (ItemDragLayerClass)
	{
		ItemDragLayerWidget = CreateWidget<UTFT_ItemDragLayerWidget>(this, ItemDragLayerClass);
		if (ItemDragLayerWidget)
		{
			ItemDragLayerWidget->AddToViewport(9999);
			ItemDragLayerWidget->SetOwningController(this);
			ItemDragLayerWidget->SetVisibility(ESlateVisibility::Collapsed);
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
			// 적 기물인지 검사 적이면 집기 취소
			if (TargetUnit->bIsEnemy)
       		{
       			UE_LOG(LogTemp, Warning, TEXT("적 기물은 조작할 수 없습니다"));
       			return; 
			}
       		//현재 전투 중인지 확인합니다.
			ATFTStageManager* StageManager = Cast<ATFTStageManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATFTStageManager::StaticClass()));
       		bool bIsBoardLocked = StageManager && StageManager->IsBoardLocked();

       		// 전투 중인데 대기석에 있는 기물이 아니라면(즉, 필드에서 싸우는 기물이라면) 집기 취소
       		if (bIsBoardLocked && !IsUnitOnBench(TargetUnit))
       		{
       			return; 
       		}
			 GrabbedUnit = TargetUnit;
			OriginalLocation = GrabbedUnit->GetActorLocation();

			 FVector LiftedLocation = OriginalLocation;
			 LiftedLocation.Z += 300.0f;
			GrabbedUnit->SetActorLocation(LiftedLocation);
          
			 // 기물을 들고 있는 동안 해당 기물이 레이저를 가리지 않도록 충돌을 꺼줌
			GrabbedUnit->SetActorEnableCollision(false);

			bIsHoldingUnit = true;
			GrabStartTime = GetWorld()->GetTimeSeconds(); // 클릭 vs 드래그 판정을 위한 시간 기록

       		if (CachedGridManager && !bIsBoardLocked) CachedGridManager->ToggleGridVisibility(true);
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
    	// 판매 구역(SellArea) 감지
    	AActor* HitActor = HitResult.GetActor();
    	if (HitActor && HitActor->ActorHasTag(TEXT("SellArea")))
    	{
    		// 판매 실행
    		ExecuteSellUnit(GrabbedUnit);
            
    		// 들고 있던 상태 초기화 및 시각 효과 끄기
    		if (HighlightActor) HighlightActor->SetActorHiddenInGame(true);
    		if (CachedGridManager) CachedGridManager->ToggleGridVisibility(false);
    		if (CachedBenchManager) CachedBenchManager->ToggleBenchVisibility(false);
            
    		bIsHoldingUnit = false;
    		GrabbedUnit = nullptr;
    		
    		BroadcastUnitCount();
            
    		// 팔았으니까 아래의 타일 배치 로직은 실행하지 않고 여기서 함수 끝
    		return; 
    	}
    	
    	ATFTStageManager* StageManager = Cast<ATFTStageManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATFTStageManager::StaticClass()));
    	bool bIsBoardLocked = StageManager && StageManager->IsBoardLocked();
    	
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
        	// 전투 중이라면 필드에 놓는 것을 무효 처리(false) 합니다.
        	if (bIsBoardLocked)
        	{
        		bIsValidDrop = false;
        		UE_LOG(LogTemp, Warning, TEXT("전투 중에는 필드에 기물을 배치할 수 없습니다!"));
        	}
	        else
	        {
	        	FinalSnappedLoc = ClosestGridLoc;
	        	bGoingToBench = false;
	        	bIsValidDrop = true;	
	        }
        }
    		// 1. 함수 대신 수학적 거리로 벤치에서 왔는지 판별합니다.
    		bool bIsComingFromBench = false;
    		if (CachedBenchManager && CachedGridManager)
    		{
    			float DistFromBench = FVector::Dist2D(OriginalLocation, CachedBenchManager->GetSnappedBenchLocation(OriginalLocation));
    			float DistFromGrid = FVector::Dist2D(OriginalLocation, CachedGridManager->GetSnappedLocation(OriginalLocation));
        
    			// 원래 위치가 그리드보다 벤치에 더 가까웠다면, 벤치에서 온 기물로 판정!
    			bIsComingFromBench = (DistFromBench < DistFromGrid); 
    		}
       if (bIsValidDrop)
       {
    		// 2. 벤치에서 필드로 향하는 경우에만 인원수 체크!
    		if (!bGoingToBench && bIsComingFromBench)
    		{
    			ATFTPlayerState* PS = GetPlayerState<ATFTPlayerState>();
    			if (PS && CachedGridManager)
    			{
    				// 주의: GridManager.h와 .cpp에 GetUnitCountOnGrid() 함수가 구현되어 있어야 합니다!
    				int32 CurrentUnitsOnGrid = CachedGridManager->GetUnitCountOnGrid();
               
    				// 현재 필드 인원수가 플레이어 레벨과 같거나 크다면?
    				if (CurrentUnitsOnGrid >= PS->PlayerLevel)
    				{
    					// 배치 실패 처리 (아래의 else문으로 빠져서 제자리로 돌아가게 함)
    					UE_LOG(LogTemp, Warning, TEXT("레벨이 낮아 기물을 더 배치할 수 없습니다. 현재 필드 기물 수: %d, 레벨: %d"), CurrentUnitsOnGrid, PS->PlayerLevel);
    					bIsValidDrop = false; 
    				}
    			}
    		}
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

    			if (bIsComingFromBench)
    			{
    				// 내가 벤치에서 왔다면, 상대방은 이제 벤치로 간 것임
    				int32 EmptyIndex = CachedBenchManager->GetFirstEmptySlotIndex();
    				if (EmptyIndex != -1) CachedBenchManager->RegisterUnitToSlot(EmptyIndex, OtherUnit);
    			}
    			else
    			{
    				// 내가 그리드에서 왔다면, 상대방은 이제 그리드로 간 것임
    				CachedGridManager->RegisterUnitToGrid(OtherUnit);
    			}
      
    			// 스왑된 두 기물의 주소록 최신화
    			UnitHomeRegistry.Add(GrabbedUnit, GrabbedUnit->GetActorTransform());
    			UnitHomeRegistry.Add(OtherUnit, OtherUnit->GetActorTransform());
    		}
    		else
    		{
    			// 빈칸 이동 로직
    			GrabbedUnit->SetActorLocation(DropLoc);
    		}
    		
    		//이제 GrabbedUnit이 최종적으로 도착한 곳에 따라 장부에 기입
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
    			// 그리드 장부에 등록
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
	BroadcastUnitCount();
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

	ATFTStageManager* StageManager = Cast<ATFTStageManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATFTStageManager::StaticClass()));
	bool bIsBoardLocked = StageManager && StageManager->IsBoardLocked();
	
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
    else if (DistToGrid <= GridThreshold && !bIsBoardLocked)
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
    	
    	FVector TargetOffset = bGoingToBench ? BenchHighlightOffset : FVector::ZeroVector;
    	
    	FRotator TargetRotation = bGoingToBench ? FRotator::ZeroRotator : GridHighlightRotation;
    	
    	FVector TargetScale = bGoingToBench ? BenchHighlightScale : FVector(1.0f, 1.0f, 1.0f);
    	
    	HighlightActor->SetActorLocation(FinalLocation + TargetOffset);
    	HighlightActor->SetActorScale3D(TargetScale);
    	HighlightActor->SetActorRotation(TargetRotation);
    	
       HighlightActor->SetActorHiddenInGame(false);
    }
    else
    {
       // 범위를 벗어나면 숨김
       HighlightActor->SetActorHiddenInGame(true);
    }
}

void ATopDownController::SpawnUnitFromBP(ETFT_ChampionKey Key)
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
			SpawnedUnit->InitWithChampionKey(Key, 1); // 1성으로 스폰
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
 
void ATopDownController::ExecuteSellUnit(ATFT_UnitCharacter* UnitToSell)
{
	if (!UnitToSell) return;

	// 1. PlayerState에 돈 추가
	if (ATFTPlayerState* PS = GetPlayerState<ATFTPlayerState>())
	{
		// UnitToSell->Cost 가 없다면 임시로 1골드로 설정
		int32 Price = 1; 
		PS->AddGold(Price);
		UE_LOG(LogTemp, Warning, TEXT("기물 판매 완료! +%d 골드. 현재 잔액: %d"), Price, PS->PlayerGold);
	}

	// 2. 장부에서 지우기 (어디 있던 기물이든 일단 지움)
	if (CachedBenchManager) 
	{
		CachedBenchManager->ClearUnitFromBench(UnitToSell);
	}
	if (CachedGridManager) 
	{
		CachedGridManager->ClearUnitFromGrid(UnitToSell);
	}

	// 3. 컨트롤러의 위치 기억소(주소록)에서 제거
	if (UnitHomeRegistry.Contains(UnitToSell))
	{
		UnitHomeRegistry.Remove(UnitToSell);
	}

	// 4. (선택 사항) 스테이지 매니저가 있다면 전투 명단에서도 제거해야 함

	// 5. 기물 완전 파괴
	UnitToSell->Destroy();
	BroadcastUnitCount();
}

void ATopDownController::BroadcastUnitCount()
{
	int32 Current = 0;
	int32 Max = 0;

	// 1. 필드에 있는 현재 기물 수 가져오기
	if (CachedGridManager) 
	{
		Current = CachedGridManager->GetUnitCountOnGrid();
	}

	// 2. 내 레벨(최대 배치 가능 수) 가져오기
	if (ATFTPlayerState* PS = GetPlayerState<ATFTPlayerState>()) 
	{
		Max = PS->PlayerLevel;
	}

	// 3. UI 쪽으로 숫자 2개를 담아서 방송 송출
	OnUnitCountChanged.Broadcast(Current, Max);
}

void ATopDownController::HandleItemDragScreenPositionUpdated(FVector2D ScreenPosition)
{
	CurrentItemDropTargetUnit = nullptr;

	FHitResult HitResult;
	if (GetHitResultAtScreenPosition(ScreenPosition, ECC_Visibility, false, HitResult))
	{
		if (ATFT_UnitCharacter* HitUnit = Cast<ATFT_UnitCharacter>(HitResult.GetActor()))
		{
			if (!HitUnit->bIsEnemy)
			{
				CurrentItemDropTargetUnit = HitUnit;
			}
		}
	}
}

void ATopDownController::BeginItemDrag(const FStruct_TFTItemInstance& DraggedItem,
	UTFT_ItemDragDropOperation* DragOperation)
{
	if (ItemDragLayerWidget)
	{
		ItemDragLayerWidget->SetVisibility(ESlateVisibility::Visible);
	}
	
	bIsDraggingItem = true;
	CurrentDraggedItem = DraggedItem;
	CurrentItemDropTargetUnit = nullptr;

	if (DragOperation)
	{
		DragOperation->OnDragScreenPositionUpdated.RemoveDynamic(this, &ATopDownController::HandleItemDragScreenPositionUpdated);
		DragOperation->OnDragScreenPositionUpdated.AddDynamic(this, &ATopDownController::HandleItemDragScreenPositionUpdated);
	}
}

void ATopDownController::EndItemDrag(bool bDroppedSuccessfully)
{
	if (!bIsDraggingItem)
	{
		return;
	}

	if (bDroppedSuccessfully && CurrentItemDropTargetUnit && ItemInventoryComponent)
	{
		const bool bEquipSuccess = CurrentItemDropTargetUnit->TryEquipItem(CurrentDraggedItem);
		if (bEquipSuccess)
		{
			ItemInventoryComponent->RemoveItemByInstanceId(CurrentDraggedItem.InstanceId);
			ItemInventoryComponent->NotifyInventoryUpdated();
		}
	}

	bIsDraggingItem = false;
	CurrentDraggedItem = FStruct_TFTItemInstance{};
	CurrentItemDropTargetUnit = nullptr;
	
	if (ItemDragLayerWidget)
	{
		ItemDragLayerWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}
