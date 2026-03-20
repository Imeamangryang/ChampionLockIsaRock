// Fill out your copyright notice in the Description page of Project Settings.


#include "Dong/TopDownController.h"
#include "Engine/HitResult.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../SHIN/Character/TFT_UnitCharacter.h"
#include "GirdManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraFunctionLibrary.h"

ATopDownController::ATopDownController()
{
	// 마우스 커서 보이게 설정
	bShowMouseCursor = true;
	// 이게 true여야 Tick이 작동
	PrimaryActorTick.bCanEverTick = true;
}

void ATopDownController::BeginPlay()
{
	Super::BeginPlay();

	// 1. 입력 서브시스템을 가져와서 IMC를 활성화합니다.
	// 블루프린트의 [Add Mapping Context] 노드와 똑같은 역할입니다.
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	
	// 맵에 있는 AGirdManager 클래스의 액터를 딱 하나 찾아옵니다.
	CachedGridManager = Cast<AGirdManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGirdManager::StaticClass()));

	// if (CachedGridManager)
	// {
	// 	 시작할 때는 숨겨둡니다.
	// 	CachedGridManager->SetActorHiddenInGame(true);
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("그리드 매니저를 찾을 수 없습니다! 맵에 배치했는지 확인하세요."));
	// }
}

void ATopDownController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 2. 기본 InputComponent를 EnhancedInputComponent로 형변환합니다.
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		// 3. IA_Grabbed 액션에 함수를 바인딩합니다.
		// Started: 버튼을 누른 순간 (Grab)
		EnhancedInputComponent->BindAction(IA_Grabbed, ETriggerEvent::Started, this, &ATopDownController::OnGrabPressed);
        
		// Completed: 버튼을 뗀 순간 (Drop)
		EnhancedInputComponent->BindAction(IA_Grabbed, ETriggerEvent::Completed, this, &ATopDownController::OnDropReleased);
		
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATopDownController::OnMoveInputReleased);
	}
}
void ATopDownController::OnGrabPressed(const FInputActionValue& Value)
{
	// 이미 기물을 들고 있는 상태에서 다시 클릭했다면? -> 내려놓기!
	if (bIsHoldingUnit)
	{
		PerformDrop();
		return;
	}
	
	// 1. 마우스 아래에 무엇이 있는지 정보를 담을 '바구니'를 만듭니다.
	FHitResult HitResult;
    
	// 2. 마우스 커서 아래로 레이저를 쏩니다. (블루프린트의 Get Hit Result Under Cursor)
	// ECC_Visibility: 눈에 보이는 것들을 검사하겠다는 뜻입니다.
	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		AActor* HitActor = HitResult.GetActor();

		// 3. 레이저에 맞은 액터가 팀원이 만든 'ATFT_UnitCharacter'인지 확인합니다. (Cast 노드)
		ATFT_UnitCharacter* TargetUnit = Cast<ATFT_UnitCharacter>(HitActor);

		if (TargetUnit)
		{
			// 4.기물을 찾았습니다. 변수들에 정보를 채워 넣습니다.
			GrabbedUnit = TargetUnit;
            
			// 기물의 현재 위치를 기억해 둡니다. (스왑용 OriginalLocation)
			OriginalLocation = GrabbedUnit->GetActorLocation();

			// 5. 기물을 번쩍 들어 올립니다.
			FVector LiftedLocation = OriginalLocation;
			LiftedLocation.Z += 300.0f;
			GrabbedUnit->SetActorLocation(LiftedLocation);

			// 6. 중요: 들고 있는 기물이 마우스 레이저를 방해하지 않게 충돌을 잠시 끕니다.
			GrabbedUnit->SetActorEnableCollision(false);

			// 7. "지금 기물을 들고 있다"고 상태를 변경합니다.
			bIsHoldingUnit = true;
			
			if (CachedGridManager)
			{
				UE_LOG(LogTemp, Warning, TEXT("그리드 켜기 명령 전송!"));
				// 액터를 켜는 게 아니라, 머티리얼 투명도를 1.0으로 만드는 함수를 호출합니다.
				CachedGridManager->ToggleGridVisibility(true);
				// 클릭한 순간의 시간을 기록합니다.
				GrabStartTime = GetWorld()->GetTimeSeconds();
			}
		}
	}
}

// 1. OnDropReleased 함수 본체 추가
void ATopDownController::OnDropReleased(const FInputActionValue& Value)
{
	// 1. 안전장치: 현재 기물을 들고 있지 않다면 아무것도 하지 않습니다.
    if (!bIsHoldingUnit || !GrabbedUnit) return;
	
	// [추가] 마우스를 누르고 있었던 시간을 계산합니다.
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float Duration = CurrentTime - GrabStartTime;
	
	// 만약 0.15초보다 짧게 눌렀다 뗐다면? (단순 클릭)
	// 내려놓지 않고 함수를 종료해서 기물을 손에 붙여둡니다.
	if (Duration < 0.15f)
	{
		UE_LOG(LogTemp, Log, TEXT("짧은 클릭 감지: 기물을 손에 유지합니다."));
		return; 
	}
	
	// 0.15초 이상 꾹 누르고 있었다면 (드래그) -> 바로 내려놓습니다.
	PerformDrop();
}

void ATopDownController::PerformDrop()
{
	
	if (!bIsHoldingUnit || !GrabbedUnit) return;
	
	FHitResult HitResult;
    // 2. 마우스 커서 아래의 '바닥(그리드)' 좌표를 먼저 찾습니다.
    if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
    {
        // 클릭한 바닥의 정확한 위치 (나중에 빈칸 이동 시 사용)
        FVector RawGridLocation = HitResult.Location;
    	
    	// 그리드 매니저에게 "이 근처 육각형 중심 좌표"를 물어봅니다.
    	FVector SnappedLocation = RawGridLocation; // 일단 기본값은 마우스 좌표
    	if (CachedGridManager)
    	{
    		SnappedLocation = CachedGridManager->GetSnappedLocation(RawGridLocation);
    	}
        
        // --- [여기서부터 주변 기물 탐색 시작 (Sphere Overlap)] ---
        
        // 발견된 액터들을 담을 배열
        TArray<AActor*> OverlappedActors;
        // 탐색할 구체의 반지름
        float ScanRadius = 60.0f;
        // 탐색할 오브젝트 타입
        TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
        ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

        // 3. 바닥 좌표를 기준으로 주변에 다른 기물이 있는지 훑습니다.
    	bool bFoundOtherUnit = UKismetSystemLibrary::SphereOverlapActors(
			 GetWorld(),
			 SnappedLocation,
			 ScanRadius,
			 ObjectTypes,
			 ATFT_UnitCharacter::StaticClass(),
			 TArray<AActor*>{GrabbedUnit},
			 OverlappedActors);

        if (bFoundOtherUnit && OverlappedActors.Num() > 0)
        {
            // [경우 A: 스왑(Swap) 발생!] 타일 위에 이미 다른 기물이 있는 경우
            ATFT_UnitCharacter* OtherUnit = Cast<ATFT_UnitCharacter>(OverlappedActors[0]);
            if (OtherUnit)
            {
                // 상대방 위치를 먼저 따옵니다. (바닥 높이여야 하므로 Z값 주의)
                FVector OtherUnitLoc = OtherUnit->GetActorLocation();

                // 1. 내 기물 이동: 상대방이 서 있던 바닥 자리로 (Z값은 바닥 높이로 스냅)
                GrabbedUnit->SetActorLocation(FVector(OtherUnitLoc.X, OtherUnitLoc.Y, OriginalLocation.Z), false, nullptr, ETeleportType::TeleportPhysics);
                
                // 2. 상대방 이동: 내가 처음 기물을 집었던 원래 자리(OriginalLocation)로
                OtherUnit->SetActorLocation(OriginalLocation, false, nullptr, ETeleportType::TeleportPhysics);
                
                UE_LOG(LogTemp, Log, TEXT("Swap Completed with: %s"), *OtherUnit->GetName());
            }
        }
        else
        {
        	// [경우 B: 빈 타일인 경우 - 단순 이동]
        	// 마우스 좌표가 아니라, 아까 구한 'SnappedLocation'의 X, Y를 사용합니다
        	FVector DropLoc = FVector(SnappedLocation.X, SnappedLocation.Y, OriginalLocation.Z);
            
        	GrabbedUnit->SetActorLocation(DropLoc, false, nullptr, ETeleportType::TeleportPhysics);
            
        	UE_LOG(LogTemp, Log, TEXT("Moved to Empty Snapped Grid"));
        }
    }
    else
    {
        // [경우 C: 허공] 그리드가 아닌 곳에 놓았을 때 (원래 자리로 복귀)
        GrabbedUnit->SetActorLocation(OriginalLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }
	
	if (CachedGridManager)
	{
		// 다시 투명도를 0.0으로 만들어 숨깁니다.
		CachedGridManager->ToggleGridVisibility(false);
		UE_LOG(LogTemp, Log, TEXT("그리드 숨김"));
	}
	
    // 4. 기물의 충돌을 다시 켭니다. (다시 클릭할 수 있게)
    GrabbedUnit->SetActorEnableCollision(true);
    // 5. 변수 초기화 (손을 비웁니다)
    bIsHoldingUnit = false;
    GrabbedUnit = nullptr;
	
}

void ATopDownController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 1. 기물을 들고 있을 때만 실행
	if (bIsHoldingUnit && GrabbedUnit)
	{
		FHitResult HitResult;
        
		// 2. 마우스 커서 아래의 '바닥' 좌표를 찾습니다.
		// 이때 ECC_Visibility 채널을 쓰면 바닥(그리드)을 감지
		// (기물의 충돌을 Grab에서 꺼두었기 때문에 레이저가 기물을 통과해 바닥을 맞춤)
		if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			// 3. 레이저가 맞은 지점(바닥)의 좌표를 가져옵니다.
			FVector MouseWorldLocation = HitResult.Location;

			// 4. 기물의 새로운 위치를 계산
			// X, Y는 마우스 위치를 그대로 따르되, Z는 우리가 정한 공중(270)으로 고정
			FVector NewLocation = FVector(MouseWorldLocation.X, MouseWorldLocation.Y, 270.0f);

			// 5. 기물을 해당 위치로 즉시 이동시킵니다.
			// Sweep을 false로 두어야 물리 엔진에 걸리지 않고 부드럽게 움직임
			GrabbedUnit->SetActorLocation(NewLocation);
		}
	}
}

void ATopDownController::OnMoveInputReleased()
{
	// 1. 마우스 아래의 위치를 찾습니다.
	FHitResult Hit;
	// 기물에 가려지지 않게 ECC_Visibility 대신 ECC_WorldStatic을 추천합니다.
	if (GetHitResultUnderCursor(ECC_WorldStatic, true, Hit))
	{
		CachedDestination = Hit.Location;

		// 2. [핵심] 내 전설이(Pawn)에게 목적지로 이동하라고 명령합니다.
		// 이 함수가 작동하려면 맵에 'NavMeshBoundsVolume'이 깔려있어
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);

		// 3. 클릭한 자리에 이펙트 생성
		if (FXCursor)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination);
		}
	}
}



