#include "TFT_CombatComponent.h"
#include "../TFT_UnitCharacter.h"
#include "EngineUtils.h"
#include "TFT_StatComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "../../Dong/Public/TopDownController.h"
#include "Kismet/GameplayStatics.h"

UTFT_CombatComponent::UTFT_CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	OwnerCharacter = Cast<ATFT_UnitCharacter>(GetOwner());

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("UTFT_CombatComponent: Owner is not ATFT_UnitCharacter."));
		return;
	}

	// TODO: 나중에는 StatComponent 또는 ChampionData에서 가져오도록 변경
	AttackRange = 150.f;
	AttackRate = 1.0f;
	CurrentAttackTimer = AttackRate;

	CurrentTarget = nullptr;
	CurrentState = ECombatState::Idle;
	CurrentStatePtr = &IdleState;

	UE_LOG(LogTemp, Log, TEXT("CombatComponent initialized for %s | Range: %.1f | Interval: %.2f"),
		*OwnerCharacter->GetName(),
		AttackRange,
		AttackRate);
}

void UTFT_CombatComponent::StartCombat()
{
	if (CurrentState != ECombatState::Idle) return;

	// 전투 시작 시점을 알리고, 타겟 탐색 상태로 전이
	UE_LOG(LogTemp, Warning, TEXT("Combat Started for: %s"), *OwnerCharacter->GetChampionNameString());
	
	// 전투 시작 시에는 서로 막혀서 안겹치도록 
	OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	// 전투 시작 시에는 마우스 피킹이 되지 않도록
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	
	if (OwnerCharacter->bIsEnemy == false)
	{
		// 전투 시작 시 캐릭터가 바라보는 방향을 뒤집어서 시작 (180도 회전)
		TargetCombatStartRotation = OwnerCharacter->GetActorRotation();
		TargetCombatStartRotation.Yaw += 180.f;
		bRotateToCombatStart = true;
	}
	
	AttackRange = OwnerCharacter->StatComponent->AttackRange * 180.f + 100;
	AttackRate = OwnerCharacter->StatComponent->AttackSpeed;

	// Idle -> Searching으로 상태 변경 (FSM 가동 시작)
	if (GetWorld())
	{
		FTimerHandle CombatStartDelayTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			CombatStartDelayTimerHandle,
			FTimerDelegate::CreateLambda([this]()
			{
				if (!this || !OwnerCharacter)
				{
					return;
				}

				if (CurrentState != ECombatState::Idle)
				{
					return;
				}

				ChangeState(GetSearchingState(), ECombatState::Searching);
			}),
			2.0f,
			false
		);
	}
	
}

void UTFT_CombatComponent::EndCombat()
{
	// 춤 애니메이션 몽타주 재생 
	OwnerCharacter->PlayDanceMontage();
}

void UTFT_CombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ATFT_UnitCharacter>(GetOwner());

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("UTFT_CombatComponent: Owner is not ATFT_UnitCharacter."));
		return;
	}

	// TODO: 나중에는 StatComponent 또는 ChampionData에서 가져오도록 변경;
	CurrentAttackTimer = AttackRate;

	CurrentTarget = nullptr;
	CurrentState = ECombatState::Idle;
	CurrentStatePtr = &IdleState;
	
	// 델리게이트 바인딩
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		ATopDownController* TopDownController = Cast<ATopDownController>(PC);
		if (TopDownController)
		{
			TopDownController->OnReturnAllUnitsHome.AddUObject(this, &UTFT_CombatComponent::HandleReturnAllUnitsHome);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("CombatComponent initialized for %s | Range: %.1f | Interval: %.2f"),
	*OwnerCharacter->GetChampionNameString(),
		AttackRange,
		AttackRate);
}

void UTFT_CombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// 전투 시작 시 캐릭터가 바라보는 방향을 뒤집어서 시작(180도 회전)
	if (bRotateToCombatStart && OwnerCharacter)
	{
		const FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
		const FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			TargetCombatStartRotation,
			DeltaTime,
			CombatStartRotateSpeed
		);

		OwnerCharacter->SetActorRotation(NewRotation);

		if (NewRotation.Equals(TargetCombatStartRotation, 1.0f))
		{
			OwnerCharacter->SetActorRotation(TargetCombatStartRotation);
			bRotateToCombatStart = false;
		}
	}
	
	// State Tick
	if (CurrentStatePtr)
	{
		CurrentStatePtr->Tick(this, DeltaTime);
	}
}

void UTFT_CombatComponent::ChangeState(ICombatState* NewState, ECombatState NewEnumState)
{
	if (!NewState || CurrentStatePtr == NewState) return;

	CurrentStatePtr->Exit(this);
	CurrentStatePtr = NewState;
	CurrentState = NewEnumState;
	CurrentStatePtr->Enter(this);
}

void UTFT_CombatComponent::UpdateCombat(float DeltaTime)
{
	if (!HasTarget())
	{
		ChangeState(GetSearchingState(), ECombatState::Searching);
		return;
	}
	
	CurrentAttackTimer -= DeltaTime;
	if (CurrentAttackTimer <= 0.f)
	{
		AttackTarget();
		ResetAttackTimer();
	}
}

void UTFT_CombatComponent::FindTarget()
{
	if (!OwnerCharacter || !GetWorld())
	{
		CurrentTarget = nullptr;
		return;
	}

	ATFT_UnitCharacter* BestTarget = nullptr;
	float ClosestDistSq = TNumericLimits<float>::Max();

	const FVector MyLocation = OwnerCharacter->GetActorLocation();

	for (TActorIterator<ATFT_UnitCharacter> It(GetWorld()); It; ++It)
	{
		ATFT_UnitCharacter* Candidate = *It;

		if (!IsValid(Candidate))
		{
			continue;
		}

		// 자기 자신 제외
		if (Candidate == OwnerCharacter)
		{
			continue;
		}
		
		// 같은 팀 제외
		if (Candidate->bIsEnemy == OwnerCharacter->bIsEnemy)
		{
			continue;
		}
		
		// 죽은 유닛 제외
		if (!Candidate->StatComponent || Candidate->StatComponent->Health <= 0)
		{
			continue;
		}

		// TODO:
		// 나중에 아래 조건들 추가 추천
		// - 같은 팀 제외
		// - 죽은 유닛 제외

		const float DistSq = FVector::DistSquared(MyLocation, Candidate->GetActorLocation());

		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	CurrentTarget = BestTarget;

	if (CurrentTarget)
	{
		const float Distance = FMath::Sqrt(ClosestDistSq);
		UE_LOG(LogTemp, Log, TEXT("%s found target: %s (Distance: %.1f)"),
			*OwnerCharacter->GetChampionNameString(),
			*CurrentTarget->GetChampionNameString(),
			Distance);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s could not find a target."), *OwnerCharacter->GetName());
	}
}

bool UTFT_CombatComponent::HasTarget() const
{
	if (!OwnerCharacter || !IsValid(CurrentTarget) || CurrentTarget == OwnerCharacter)
	{
		return false;
	}

	if (!CurrentTarget->StatComponent)
	{
		return false;
	}

	return CurrentTarget->StatComponent->Health > 0;
}

bool UTFT_CombatComponent::IsTargetInRange() const
{
	if (!HasTarget()) return false;
	float Distance = FVector::Dist(OwnerCharacter->GetActorLocation(), CurrentTarget->GetActorLocation());
	return Distance <= AttackRange;
}

void UTFT_CombatComponent::MoveToTarget()
{
	if (!OwnerCharacter || !HasTarget())
	{
		return;
	}
	// 이동 시 재생하고 있는 모든 몽타주 제거

	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	const FVector TargetLocation = CurrentTarget->GetActorLocation();

	FVector Direction = TargetLocation - OwnerLocation;
	Direction.Z = 0.f;

	const float Distance = Direction.Size();

	// 너무 가까우면 이동 안 함
	if (Distance <= AttackRange)
	{
		return;
	}

	if (Direction.IsNearlyZero())
	{
		return;
	}

	Direction.Normalize();

	// 회전
	OwnerCharacter->SetActorRotation(Direction.Rotation());
	// 전진
	OwnerCharacter->AddMovementInput(Direction, 1.0f);
}

void UTFT_CombatComponent::AttackTarget()
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (!HasTarget())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s tried to attack, but has no valid target."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	// 혹시 사거리 밖이면 공격하지 않음
	if (!IsTargetInRange())
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s tried to attack %s, but target is out of range."),
			*OwnerCharacter->GetChampionNameString(),
			*CurrentTarget->GetChampionNameString());
		return;
	}

	// 타겟 방향으로 회전
	FVector Direction = CurrentTarget->GetActorLocation() - OwnerCharacter->GetActorLocation();
	Direction.Z = 0.f;

	if (!Direction.IsNearlyZero())
	{
		FRotator LookAtRotation = Direction.Rotation();
		OwnerCharacter->SetActorRotation(LookAtRotation);
	}

	// 현재는 최소 구현: 로그 출력
	UE_LOG(LogTemp, Log, TEXT("%s attacks %s"),
		*OwnerCharacter->GetChampionNameString(),
		*CurrentTarget->GetChampionNameString());

	// TODO: 여기에 실제 공격 처리 추가
	// 1. 공격 애니메이션 재생 
	OwnerCharacter->PlayAttackMontageByInterval(AttackRate);
}

void UTFT_CombatComponent::OnAttackHitNotify()
{
	if (OwnerCharacter == nullptr || CurrentTarget == nullptr)
	{
		return;
	}
	
	const float Distance = FVector::Dist(
		OwnerCharacter->GetActorLocation(),
		CurrentTarget->GetActorLocation()
	);

	if (Distance > AttackRange)
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s hit notify fired, but target is out of range."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	const float Damage = OwnerCharacter->StatComponent
		? OwnerCharacter->StatComponent->AttackDamage
		: 0.f;

	if (Damage <= 0.f)
	{
		return;
	}

	CurrentTarget->StatComponent->ApplyDamage(Damage);

	if (OwnerCharacter->StatComponent)
	{
		OwnerCharacter->StatComponent->AddMana(10.f);
	}

	UE_LOG(LogTemp, Log, TEXT("%s dealt %.1f damage to %s (Target HP: %d)"),
		*OwnerCharacter->GetChampionNameString(),
		Damage,
		*CurrentTarget->GetChampionNameString(),
		CurrentTarget->StatComponent->Health);
}

void UTFT_CombatComponent::CastSkill()
{
}

bool UTFT_CombatComponent::IsManaFull() const
{
	// Mana 시스템이 구현되어 있다면, 현재 마나가 최대 마나와 같은지 체크
	return false; // 임시 반환값
}

bool UTFT_CombatComponent::IsCasting() const
{
	// 스킬 캐스팅 중인지 여부 체크 (캐스팅 애니메이션 재생 여부 등)
	return false; // 임시 반환값
}

void UTFT_CombatComponent::ResetAttackTimer()
{
	if (AttackRate <= 0.f)
	{
		CurrentAttackTimer = 0.f;
		return;
	}

	// AttackInterval은 "초당 공격 횟수"
	CurrentAttackTimer = 1.0f / AttackRate;
}

void UTFT_CombatComponent::StopAllActions()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->StopMontage(0.15);
	}
}

void UTFT_CombatComponent::HandleReturnAllUnitsHome()
{
	StopAllActions();
	
	// 전투 종료 시 Idle 상태로 복귀 및 타겟 초기화
	CurrentTarget = nullptr;
	ChangeState(GetIdleState(), ECombatState::Idle);
	
	OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
}

void UTFT_CombatComponent::Dead()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->PlayDeathMontage();
		
		// 사망 모션이 끝나면
		// 사망 애니메이션이 끝난 뒤 액터 숨기기
		// 오버랩도 끄기
		// OwnerCharacter->SetActorHiddenInGame(true);
		// OwnerCharacter->SetActorEnableCollision(false);
	}
}


