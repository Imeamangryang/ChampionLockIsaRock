#pragma once

#include "CoreMinimal.h"
#include "../FSM/CombatState.h"
#include "Components/ActorComponent.h"
#include "TFT_CombatComponent.generated.h"

UENUM()
enum class ECombatState : uint8
{
	Idle,
	Searching,
	Moving,
	Attacking,
	CastingSkill,
	Dead
};

class ATFT_UnitCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHAMPIONLOCKISAROCK_API UTFT_CombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTFT_CombatComponent();
	
	UFUNCTION(BlueprintCallable)
	void StartCombat();
	
	UFUNCTION(BlueprintCallable)
	void EndCombat();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// ===== Owner =====
	UPROPERTY()
	class ATFT_UnitCharacter* OwnerCharacter;

	// ===== Target =====
	UPROPERTY()
	ATFT_UnitCharacter* CurrentTarget;

	// ===== State =====
	UPROPERTY()
	ECombatState CurrentState = ECombatState::Idle;

	// ===== Combat Data =====
	float AttackRange;
	float AttackRate;
	float CurrentAttackTimer = 0.f;

	// ===== Core Logic =====
	void UpdateCombat(float DeltaTime);

	void FindTarget();
	bool HasTarget() const;
	bool IsTargetInRange() const;
	void MoveToTarget();
	void AttackTarget();
	void CastSkill();
	bool IsManaFull() const;
    
	// 추가로 필요한 헬퍼 함수들
	bool IsCasting() const;
	void ResetAttackTimer();
	void StopAllActions();
	
	// 상태 전환 함수 (Enum 값도 같이 업데이트 하도록 파라미터 추가)
	void ChangeState(ICombatState* NewState, ECombatState NewEnumState);

	// 각 상태의 인스턴스 포인터를 반환하는 Getter
	ICombatState* GetIdleState()        { return &IdleState; }
	ICombatState* GetSearchingState()   { return &SearchingState; }
	ICombatState* GetMovingState()      { return &MovingState; }
	ICombatState* GetAttackingState()   { return &AttackingState; }
	ICombatState* GetCastingSkillState(){ return &CastingSkillState; }
	ICombatState* GetDeadState()        { return &DeadState; }
	
	// 실제 로직을 구동할 순수 C++ 상태 포인터
	ICombatState* CurrentStatePtr;

	// 동적 할당(new)을 막기 위해 컴포넌트가 미리 소유하는 상태 인스턴스들
	FIdleState IdleState;
	FSearchingState SearchingState;
	FMovingState MovingState;
	FAttackingState AttackingState;
	FCastingSkillState CastingSkillState;
	FDeadState DeadState;
};
