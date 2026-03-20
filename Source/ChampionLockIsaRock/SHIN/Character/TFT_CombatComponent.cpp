#include "TFT_CombatComponent.h"
#include "TFT_UnitCharacter.h"
#include "EngineUtils.h"
#include "TFT_StatComponent.h"
#include "GameFramework/Controller.h"
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
	
	AttackRange = OwnerCharacter->StatComponent->AttackRange * 180.f;
	AttackRate = OwnerCharacter->StatComponent->AttackSpeed;
	
	// Idle -> Searching으로 상태 변경 (FSM 가동 시작)
	ChangeState(GetSearchingState(), ECombatState::Searching);
	
}

void UTFT_CombatComponent::EndCombat()
{
	// 전투 종료 시 Idle 상태로 복귀 및 타겟 초기화
	CurrentTarget = nullptr;
	ChangeState(GetIdleState(), ECombatState::Idle);
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

	UE_LOG(LogTemp, Log, TEXT("CombatComponent initialized for %s | Range: %.1f | Interval: %.2f"),
	*OwnerCharacter->GetChampionNameString(),
		AttackRange,
		AttackRate);
}

void UTFT_CombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
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

		// TODO:
		// 나중에 아래 조건들 추가 추천
		// - 같은 팀 제외
		// - 죽은 유닛 제외
		// - 타겟 불가 상태 제외

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
	return IsValid(CurrentTarget) && CurrentTarget != OwnerCharacter;
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
	// 2. 대상 HP 감소
	// 3. 마나 획득

	// 예: StatComponent가 나중에 이런 함수를 가진다면
	// if (CurrentTarget->StatComponent)
	// {
	//     const float Damage = 10.f;
	//     CurrentTarget->StatComponent->ApplyDamage(Damage);
	// }

	// 예: 마나 증가
	// if (OwnerCharacter->StatComponent)
	// {
	//     OwnerCharacter->StatComponent->AddMana(10.f);
	// }
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
	if (OwnerCharacter && OwnerCharacter->GetController())
	{
		OwnerCharacter->GetController()->StopMovement();
	}
}


