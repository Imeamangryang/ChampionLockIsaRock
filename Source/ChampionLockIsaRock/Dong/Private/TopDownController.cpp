// Fill out your copyright notice in the Description page of Project Settings.

#include "Dong/Public/TopDownController.h"
#include "Dong/Public/BenchManager.h"
#include "Dong/Public/GirdManager.h"
#include "Dong/Public/UnitManager.h"
#include "Dong/Public/TFTPlayerState.h"
#include "Dong/Public/TFTStageManager.h"
#include "Dong/Synergy/TFT_SynergyComponent.h"
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
#include "Blueprint/SlateBlueprintLibrary.h"

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
    	EnhancedInputComponent->BindAction(IA_Sell, ETriggerEvent::Started, this, &ATopDownController::OnSellKeyPressed);
    	EnhancedInputComponent->BindAction(IA_CheatUnits, ETriggerEvent::Started, this, &ATopDownController::OnCheatUnitsPressed);
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
			float NewLength = SpringArm->TargetArmLength + (ZoomValue * 50.0f);
			SpringArm->TargetArmLength = FMath::Clamp(NewLength, 50.0f, 1200.0f);
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
              bIsComingFromBench = (DistFromBench < DistFromGrid); 
           }

       // =========================================================
       // 1차 검사: 타일에 놓았는가? + 인원수 컷에 걸리지 않는가?
       // =========================================================
       FVector DropLoc = FVector(FinalSnappedLoc.X, FinalSnappedLoc.Y, OriginalLocation.Z);
       TArray<AActor*> OverlappedActors;
       
       if (bIsValidDrop)
       {
           UKismetSystemLibrary::SphereOverlapActors(GetWorld(), DropLoc, 60.0f, {UEngineTypes::ConvertToObjectType(ECC_Pawn)}, ATFT_UnitCharacter::StaticClass(), {GrabbedUnit}, OverlappedActors);
           
           bool bIsSwapping = (OverlappedActors.Num() > 0);

           // 벤치 -> 필드 빈자리로 가는 경우에만 인원수 체크
           if (!bGoingToBench && bIsComingFromBench && !bIsSwapping)
           {
              ATFTPlayerState* PS = GetPlayerState<ATFTPlayerState>();
              if (PS && CachedGridManager)
              {
                 int32 CurrentUnitsOnGrid = CachedGridManager->GetUnitCountOnGrid();
                 if (CurrentUnitsOnGrid >= PS->PlayerLevel)
                 {
                    UE_LOG(LogTemp, Warning, TEXT("레벨이 낮아 기물을 더 배치할 수 없습니다. 현재 필드 기물 수: %d, 레벨: %d"), CurrentUnitsOnGrid, PS->PlayerLevel);
                    bIsValidDrop = false; // 인원수 초과 시 여기서 드롭 무효화!
                 }
              }
           }
       }

       // =========================================================
       // 2차 실행: 검사 통과 여부에 따라 이동 or 튕겨내기
       // =========================================================
       if (bIsValidDrop)
       {
           // 통과! 장부에서 지우고 위치 이동/스왑 처리
           if (CachedBenchManager) CachedBenchManager->ClearUnitFromBench(GrabbedUnit);
           if (CachedGridManager) CachedGridManager->ClearUnitFromGrid(GrabbedUnit);

           if (OverlappedActors.Num() > 0)
           {
              // 스왑 로직
              ATFT_UnitCharacter* OtherUnit = Cast<ATFT_UnitCharacter>(OverlappedActors[0]);
           	
           	  if (bIsComingFromBench) OtherUnit->bIsBenched = true; 
           	  else OtherUnit->bIsBenched = false; // 내가 필드에서 들고 왔다면, 벤치에 있던 녀석은 필드로 나가니까 false입니다.
      
              if (CachedBenchManager) CachedBenchManager->ClearUnitFromBench(OtherUnit);
              if (CachedGridManager) CachedGridManager->ClearUnitFromGrid(OtherUnit);

              FVector OtherUnitLoc = OtherUnit->GetActorLocation();
              GrabbedUnit->SetActorLocation(FVector(OtherUnitLoc.X, OtherUnitLoc.Y, OriginalLocation.Z));
              OtherUnit->SetActorLocation(OriginalLocation);

           	  if (bIsComingFromBench)
           	  {
           		// 첫 번째 빈칸이 아니라, 방금 잡은 기물이 '원래 있던 자리'의 인덱스로 돌려보냄
           		int32 OriginalIndex = CachedBenchManager->GetClosestBenchSlotIndex(OriginalLocation);
           		if (OriginalIndex != -1) CachedBenchManager->RegisterUnitToSlot(OriginalIndex, OtherUnit);
           	  }
              else
              {
                 CachedGridManager->RegisterUnitToGrid(OtherUnit);
              }
      
              UnitHomeRegistry.Add(GrabbedUnit, GrabbedUnit->GetActorTransform());
              UnitHomeRegistry.Add(OtherUnit, OtherUnit->GetActorTransform());
           }
           else
           {
              // 빈칸 이동 로직
              GrabbedUnit->SetActorLocation(DropLoc);
           }
           
           // 도착한 곳에 따라 내 기물 장부에 기입
           if (bGoingToBench && CachedBenchManager)
           {
           	 GrabbedUnit->bIsBenched = true; // 벤치로 갔으니 true로 설정
              
           	int32 TargetIndex = CachedBenchManager->GetClosestBenchSlotIndex(FinalSnappedLoc);
           	if (TargetIndex != -1) CachedBenchManager->RegisterUnitToSlot(TargetIndex, GrabbedUnit);
           }
           else if (!bGoingToBench && CachedGridManager)
           {
           	 GrabbedUnit->bIsBenched = false; // 필드로 나왔으니 false
              
           	 CachedGridManager->RegisterUnitToGrid(GrabbedUnit);
           }
   
       	  UnitHomeRegistry.Add(GrabbedUnit, GrabbedUnit->GetActorTransform());
       	  if (SoundUnitDrop)
       	  {
       	  	  UGameplayStatics::PlaySound2D(this, SoundUnitDrop, 1, 1, 0.3);
       	  } 
       }
       else
       {
       	// 타일을 벗어났거나, 인원수 제한에 걸려 bIsValidDrop이 false가 되면 무조건 여기로 와서 제자리로 돌아갑니다
       	GrabbedUnit->SetActorLocation(OriginalLocation);
       }
    	// 기물 이동이 완료되었으니 시너지를 다시 계산합니다.
    	if (UTFT_SynergyComponent* SynergyComp = FindComponentByClass<UTFT_SynergyComponent>())
    	{
    		SynergyComp->RecalculateSynergies();
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
				UISubsystem->BroadcastStatUIOpen(true, Cast<ATFT_UnitCharacter>(HitActor));
				return;
			}
		}
	}

	// 아니면  UI 닫기
	UISubsystem->BroadcastStatUIOpen(false, nullptr);
	
	FHitResult GroundHit;
	if (GetHitResultUnderCursor(ECC_WorldStatic, true, GroundHit))
	{
		CachedDestination = GroundHit.Location;
		
		if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn())) // 현재 내가 조종 중인 캐릭터(전설이)를 가져옵니다.
		{
			if (UAnimInstance* AnimInstance = MyCharacter->GetMesh()->GetAnimInstance()) // 애니메이션 담당자를 부릅니다.
			{
				// 0.1초의 부드러운 전환(Blend Out)을 주면서 재생 중인 모든 몽타주를 강제로 끕니다.
				AnimInstance->Montage_Stop(0.1f); 
			}
		}
		
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
void ATopDownController::showmethemoney()
{
	if (ATFTPlayerState* PS = GetPlayerState<ATFTPlayerState>())
	{
		PS->AddGold(999);
	}
}
 
void ATopDownController::ExecuteSellUnit(ATFT_UnitCharacter* UnitToSell)
{
    if (!UnitToSell) return;

    // 1. PlayerState에 돈 추가
    if (ATFTPlayerState* PS = GetPlayerState<ATFTPlayerState>())
    {
       // 기물의 코스트와 성급 가져오기
       int32 BaseCost = UnitToSell->ChampionData.Cost;   
       int32 Star = UnitToSell->starLevel;

       // 1. 이 기물을 만드는 데 들어간 총 1성 기물의 개수 (1성=1개, 2성=3개, 3성=9개)
       int32 TotalUnits = 1;
       if (Star == 2) TotalUnits = 3;
       else if (Star == 3) TotalUnits = 9;

       // 2. 원가(기본 가치) 계산
       int32 Price = BaseCost * TotalUnits;

       // 3. 1코스트를 제외한 나머지 기물에 대해 판매 패널티 적용
       if (BaseCost > 1)
       {
           if (Star == 2)
           {
               Price -= 1; // 2성일 때 -1원
           }
           else if (Star == 3)
           {
               Price -= 2; // 3성일 때 -2원
           }
       }

       // 만약 버그나 데이터 오류로 가격이 0 이하로 떨어지는 것을 막는 안전장치
       Price = FMath::Max(Price, 1);
    	
       PS->AddGold(Price);
    	
    	if (SoundUnitSell)
    	{
    		UGameplayStatics::PlaySound2D(this, SoundUnitSell);
    	}
    	
       UE_LOG(LogTemp, Warning, TEXT("기물 판매 완료! %d성 %d코스트 -> +%d 골드 획득. 현재 잔액: %d"), Star, BaseCost, Price, PS->PlayerGold);
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
    ReturnUnitEquippedItemsToInventory(UnitToSell);
    
    // 5. 기물 완전 파괴
    UnitToSell->Destroy();
    BroadcastUnitCount();
    
    // 기물이 팔렸으니 시너지를 다시 계산합니다.
    if (UTFT_SynergyComponent* SynergyComp = FindComponentByClass<UTFT_SynergyComponent>())
    {
       SynergyComp->RecalculateSynergies();
    }
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

void ATopDownController::ProcessPendingUpgrades()
{
	if (PendingUpgradeKeys.IsEmpty()) return;

	AUnitManager* UnitManager = Cast<AUnitManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AUnitManager::StaticClass()));
	if (!UnitManager) return;

	// 원본을 복사해두고 즉시 비움
	TArray<ETFT_ChampionKey> KeysToProcess = PendingUpgradeKeys;
	PendingUpgradeKeys.Empty();

	for (ETFT_ChampionKey Key : KeysToProcess)
	{
		UnitManager->TryUpgradeUnit(Key, 1); 
		UnitManager->TryUpgradeUnit(Key, 2); 
	}
}

void ATopDownController::EndItemDrag(bool bDroppedSuccessfully)
{
	if (!bIsDraggingItem)
	{
		return;
	}

	ATFT_UnitCharacter* DropTargetUnit = nullptr;

	if (bDroppedSuccessfully)
	{
		DropTargetUnit = FindItemDropTargetAtScreenPosition(LastKnownItemDragScreenPosition);
	}

	if (bDroppedSuccessfully && DropTargetUnit && ItemInventoryComponent)
	{
		const bool bEquipSuccess = DropTargetUnit->TryEquipItem(CurrentDraggedItem);
		if (bEquipSuccess)
		{
			ItemInventoryComponent->RemoveItemByInstanceId(CurrentDraggedItem.InstanceId);
			ItemInventoryComponent->NotifyInventoryUpdated();
		}
		
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UTFT_UISubsystem* UISubsystem = LP->GetSubsystem<UTFT_UISubsystem>())
			{
				UISubsystem->BroadcastStatUIOpen(true, DropTargetUnit);
			}
		}
	}

	bIsDraggingItem = false;
	CurrentDraggedItem = FStruct_TFTItemInstance{};
	CurrentItemDropTargetUnit = nullptr;
	LastKnownItemDragScreenPosition = FVector2D::ZeroVector;

	if (ItemDragLayerWidget)
	{
		ItemDragLayerWidget->SetVisibility(ESlateVisibility::Collapsed);
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
	LastKnownItemDragScreenPosition = FVector2D::ZeroVector;
	
	if (DragOperation)
	{
		DragOperation->OnDragScreenPositionUpdated.RemoveDynamic(this, &ATopDownController::HandleItemDragScreenPositionUpdated);
		DragOperation->OnDragScreenPositionUpdated.AddDynamic(this, &ATopDownController::HandleItemDragScreenPositionUpdated);
	}
}


void ATopDownController::HandleItemDragScreenPositionUpdated(FVector2D ScreenPosition)
{
	FVector2D PixelPosition;
	FVector2D ViewportPosition;

	USlateBlueprintLibrary::AbsoluteToViewport(
		this,
		ScreenPosition,
		PixelPosition,
		ViewportPosition
	);

	LastKnownItemDragScreenPosition = PixelPosition;
}

ATFT_UnitCharacter* ATopDownController::FindItemDropTargetAtScreenPosition(const FVector2D& ScreenPosition) const
{
	if (ScreenPosition.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("FindItemDropTargetAtScreenPosition - ScreenPosition is zero"));
		return nullptr;
	}

	FVector WorldLocation;
	FVector WorldDirection;

	if (!DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldLocation, WorldDirection))
	{
		UE_LOG(LogTemp, Warning, TEXT("Deproject failed"));
		return nullptr;
	}

	const FVector TraceStart = WorldLocation;
	const FVector TraceEnd = TraceStart + (WorldDirection * 100000.0f);

	if (!GetWorld())
	{
		return nullptr;
	}

	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 3.0f, 0, 2.0f);
	// DrawDebugSphere(GetWorld(), TraceEnd, 40.0f, 12, FColor::Blue, false, 3.0f);

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ItemDropTrace), false);

	const bool bHit = GetWorld()->LineTraceMultiByChannel(
		HitResults,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (!bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trace hit nothing"));
		return nullptr;
	}

	for (const FHitResult& HitResult : HitResults)
	{

		if (ATFT_UnitCharacter* HitUnit = Cast<ATFT_UnitCharacter>(HitResult.GetActor()))
		{
			if (!HitUnit->bIsEnemy)
			{
				return HitUnit;
			}
		}
	}
	return nullptr;
}

void ATopDownController::ReturnUnitEquippedItemsToInventory(ATFT_UnitCharacter* Unit)
{
	if (!Unit || !ItemInventoryComponent)
	{
		return;
	}

	for (FStruct_TFTEquippedItemSlot& Slot : Unit->EquippedItemSlots)
	{
		if (Slot.bOccupied && !Slot.ItemInstance.ItemId.IsNone())
		{
			ItemInventoryComponent->AddItem(Slot.ItemInstance);
			Slot.bOccupied = false;
			Slot.ItemInstance = FStruct_TFTItemInstance{};
		}
	}

	Unit->RefreshItemSlotWidget();
}

void ATopDownController::OnSellKeyPressed()
{
	// 기물을 들고 있고(bIsHoldingUnit), 실제로 잡힌 기물이 있을 때만 실행
	if (bIsHoldingUnit && GrabbedUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("E 키 입력: 기물 즉시 판매 시작"));

		// 1. 이미 만들어둔 판매 함수 호출
		ExecuteSellUnit(GrabbedUnit);

		// 2. 판매 후 '들고 있는 상태' 초기화 (이거 안 하면 허공을 들고 있게 됩니다!)
		bIsHoldingUnit = false;
		GrabbedUnit = nullptr;

		// 3. 시각적 가이드(하이라이트, 그리드) 모두 끄기
		if (HighlightActor) HighlightActor->SetActorHiddenInGame(true);
		if (CachedGridManager) CachedGridManager->ToggleGridVisibility(false);
		if (CachedBenchManager) CachedBenchManager->ToggleBenchVisibility(false);
        
		// 4. 인원수 UI 갱신 (이미 ExecuteSellUnit에 들어있을 수도 있지만 확실하게 하기 위해)
		BroadcastUnitCount();
	}
}

void ATopDownController::OnCheatUnitsPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("치트키 활성화"));
	
	if (ATFTPlayerState* PS = GetPlayerState<ATFTPlayerState>())
	{
		// 현재 레벨이 10 미만인 동안 계속해서 '다음 레벨까지 남은 경험치'를 딱 맞춰서 먹여줍니다.
		while (PS->PlayerLevel < 10)
		{
			int32 XPNeeded = PS->MaxXP - PS->CurrentXP;
			PS->AddXP(XPNeeded); 
		}
	}

	// 케일 3성 소환 (Enum 이름 확인 필요!)
	SpawnCheatUnit(ETFT_ChampionKey::Sejuani, 3);
	
	SpawnCheatUnit(ETFT_ChampionKey::Lissandra, 3);
	
	SpawnCheatUnit(ETFT_ChampionKey::Pantheon, 3);
	
	SpawnCheatUnit(ETFT_ChampionKey::Braum, 3);
	
	SpawnCheatUnit(ETFT_ChampionKey::Ashe, 3);
	
	SpawnCheatUnit(ETFT_ChampionKey::Anivia, 3);
	
	SpawnCheatUnit(ETFT_ChampionKey::Volibear, 3);
    
	// 소환 후 인원수 및 시너지 갱신
	BroadcastUnitCount();
	if (UTFT_SynergyComponent* SynergyComp = FindComponentByClass<UTFT_SynergyComponent>())
	{
		SynergyComp->RecalculateSynergies();
	}
}

void ATopDownController::SpawnCheatUnit(ETFT_ChampionKey Key, int32 StarLevel)
{
	if (!UnitClass || !CachedBenchManager) return;

	int32 EmptyIndex = CachedBenchManager->GetFirstEmptySlotIndex();
	if (EmptyIndex > -1)
	{
		FVector BenchSlotLoc = CachedBenchManager->GetBenchSlotCenterByIndex(EmptyIndex);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (ATFT_UnitCharacter* NewUnit = GetWorld()->SpawnActor<ATFT_UnitCharacter>(UnitClass, BenchSlotLoc, FRotator::ZeroRotator, SpawnParams))
		{
			// 핵심: 성급을 3으로 바로 넘겨줌
			NewUnit->InitWithChampionKey(Key, StarLevel);
            
			CachedBenchManager->RegisterUnitToSlot(EmptyIndex, NewUnit);
			UnitHomeRegistry.Add(NewUnit, NewUnit->GetActorTransform());
		}
	}
}