#include "TFT_CombatComponent.h"

UTFT_CombatComponent::UTFT_CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTFT_CombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTFT_CombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateCombat(DeltaTime);
}

void UTFT_CombatComponent::UpdateCombat(float DeltaTime)
{
	if (CurrentState == ECombatState::Dead) return;

	// // 1️⃣ 타겟 체크
	// if (!HasTarget())
	// {
	// 	FindTarget();
	// 	CurrentState = ECombatState::Searching;
	// 	return;
	// }
	//
	// // 2️⃣ 스킬 우선
	// if (IsManaFull())
	// {
	// 	CastSkill();
	// 	CurrentState = ECombatState::CastingSkill;
	// 	return;
	// }
	//
	// // 3️⃣ 거리 체크
	// if (IsTargetInRange())
	// {
	// 	AttackTarget();
	// 	CurrentState = ECombatState::Attacking;
	// }
	// else
	// {
	// 	MoveToTarget();
	// 	CurrentState = ECombatState::Moving;
	// }
}

void UTFT_CombatComponent::StartCombat()
{
}

void UTFT_CombatComponent::EndCombat()
{
}


