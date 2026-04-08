#include "CombatState.h"
#include "SHIN/Character/Components/TFT_CombatComponent.h"
#include "SHIN/Character/TFT_UnitCharacter.h"

void FIdleState::Enter(UTFT_CombatComponent* Combat)
{
    Combat->CurrentTarget = nullptr;
	// Combat->StopAllActions();
}

void FSearchingState::Enter(UTFT_CombatComponent* Combat)
{
    Combat->FindTarget();
}

void FSearchingState::Tick(UTFT_CombatComponent* Combat, float DeltaTime)
{
    if (!Combat->HasTarget())
    {
        Combat->FindTarget();

        if (!Combat->HasTarget())
        {
            Combat->ChangeState(Combat->GetIdleState(), ECombatState::Idle);
            return;
        }
    }

    if (Combat->IsManaFull())
    {
        Combat->ChangeState(Combat->GetCastingSkillState(), ECombatState::CastingSkill);
        return;
    }

    if (Combat->IsTargetInRange())
    {
        Combat->ChangeState(Combat->GetAttackingState(), ECombatState::Attacking);
        return;
    }

    Combat->ChangeState(Combat->GetMovingState(), ECombatState::Moving);
}

void FMovingState::Enter(UTFT_CombatComponent* Combat)
{
    if (Combat && Combat->OwnerCharacter)
	{
		Combat->OwnerCharacter->StopMontage(0.15f);
	}
}

void FMovingState::Tick(UTFT_CombatComponent* Combat, float DeltaTime)
{
    if (!Combat->HasTarget())
    {
        Combat->ChangeState(Combat->GetSearchingState(), ECombatState::Searching);
        return;
    }

    if (Combat->IsManaFull())
    {
        Combat->ChangeState(Combat->GetCastingSkillState(), ECombatState::CastingSkill);
        return;
    }

    if (Combat->IsTargetInRange())
    {
        Combat->ChangeState(Combat->GetAttackingState(), ECombatState::Attacking);
        return;
    }

    // 타겟 방향으로 이동 명령
    Combat->MoveToTarget();
}

void FAttackingState::Enter(UTFT_CombatComponent* Combat)
{
    Combat->AttackTarget();
    Combat->ResetAttackTimer();
}

void FAttackingState::Tick(UTFT_CombatComponent* Combat, float DeltaTime)
{
    if (!Combat->HasTarget())
    {
        Combat->ChangeState(Combat->GetSearchingState(), ECombatState::Searching);
        return;
    }

    if (Combat->IsManaFull())
    {
        Combat->ChangeState(Combat->GetCastingSkillState(), ECombatState::CastingSkill);
        return;
    }

    if (!Combat->IsTargetInRange())
    {
        Combat->ChangeState(Combat->GetMovingState(), ECombatState::Moving);
        return;
    }

    // 공격 타이머 업데이트 및 공격 실행
    Combat->UpdateCombat(DeltaTime);
}

void FCastingSkillState::Enter(UTFT_CombatComponent* Combat)
{
    Combat->CastSkill();
}

void FCastingSkillState::Tick(UTFT_CombatComponent* Combat, float DeltaTime)
{
}

void FDeadState::Enter(UTFT_CombatComponent* Combat)
{
    Combat->Dead();
    // 사망 처리 로직 (콜리전 끄기 등)
}
