// Fill out your copyright notice in the Description page of Project Settings.


#include "Dong/TopDownController.h"
#include "Engine/HitResult.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../SHIN/Character/TFT_UnitCharacter.h"
#include "GirdManager.h"

ATopDownController::ATopDownController()
{
	// 마우스 커서 보이게 설정
	bShowMouseCursor = true;
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
	}
}
void ATopDownController::OnGrabPressed(const FInputActionValue& Value)
{
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
			LiftedLocation.Z += 270.0f;
			GrabbedUnit->SetActorLocation(LiftedLocation);

			// 6. 중요: 들고 있는 기물이 마우스 레이저를 방해하지 않게 충돌을 잠시 끕니다.
			GrabbedUnit->SetActorEnableCollision(false);

			// 7. "지금 기물을 들고 있다"고 상태를 변경합니다.
			bIsHoldingUnit = true;

			// 8. 그리드를 켭니다. (그리드 매니저를 찾아서 Hidden을 False로)
			// TObjectIterator를 쓰면 맵에 있는 특정 클래스를 빠르게 찾을 수 있습니다.
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				// 그리드 액터 이름이 BP_GridManager_C 같은 식일 테니 태그나 클래스로 찾습니다.
				if (It->ActorHasTag(FName("Grid"))) 
				{
					It->SetActorHiddenInGame(false);
					break;
				}
			}
		}
	}
}

// 1. OnDropReleased 함수 본체 추가
void ATopDownController::OnDropReleased(const FInputActionValue& Value)
{
	
	
}





