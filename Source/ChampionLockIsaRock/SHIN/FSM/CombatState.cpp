#include "CombatState.h"
#include "SHIN/Character/TFT_CombatComponent.h"

void FIdleState::Enter(UTFT_CombatComponent* Combat)
{
	Combat->StopAllActions();
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

    Combat->ChangeState(Combat->GetMovingState(), ECombatState::Moving);
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
    // 스킬 캐스팅(애니메이션 등)이 완료되면 다시 검색으로
    if (!Combat->IsCasting())
    {
        Combat->ChangeState(Combat->GetSearchingState(), ECombatState::Searching);
    }
}

void FDeadState::Enter(UTFT_CombatComponent* Combat)
{
    Combat->StopAllActions();
    // 사망 처리 로직 (콜리전 끄기 등)
}