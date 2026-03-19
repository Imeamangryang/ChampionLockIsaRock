#include "TFT_CombatComponent.h"
#include "TFT_UnitCharacter.h"

UTFT_CombatComponent::UTFT_CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentStatePtr = &IdleState;
}

void UTFT_CombatComponent::StartCombat()
{
	if (CurrentState != ECombatState::Idle) return;

	// 전투 시작 시점을 알리고, 타겟 탐색 상태로 전이
	UE_LOG(LogTemp, Log, TEXT("Combat Started for: %s"), *GetOwner()->GetName());
	
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
	// 주변 적군 중 가장 가까운 타겟 검색 로직 (TActorIterator 등 사용)
	// CurrentTarget = ...
}

bool UTFT_CombatComponent::HasTarget() const
{
	return IsValid(CurrentTarget);
}

bool UTFT_CombatComponent::IsTargetInRange() const
{
	if (!HasTarget()) return false;
	float Distance = FVector::Dist(OwnerCharacter->GetActorLocation(), CurrentTarget->GetActorLocation());
	return Distance <= AttackRange;
}

void UTFT_CombatComponent::MoveToTarget()
{
	if (!HasTarget()) return;
	// AI Controller를 통한 이동 또는 단순 방향 이동
	// OwnerCharacter->GetController()->MoveToActor(CurrentTarget);
}

void UTFT_CombatComponent::AttackTarget()
{
	if (!HasTarget()) return;
	// 데미지 전달 및 공격 애니메이션 몽타주 재생
	// CurrentTarget->TakeDamage(...);
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
	CurrentAttackTimer = AttackInterval;
}

void UTFT_CombatComponent::StopAllActions()
{
	if (OwnerCharacter && OwnerCharacter->GetController())
	{
		OwnerCharacter->GetController()->StopMovement();
	}
}


