#pragma once

#include "CoreMinimal.h"
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
	
	void StartCombat();
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
	float AttackInterval;
	float CurrentAttackTimer = 0.f;

	// ===== Core Logic =====
	void UpdateCombat(float DeltaTime);

	// void FindTarget();
	// bool HasTarget() const;
	//
	// bool IsTargetInRange() const;
	//
	// void MoveToTarget();
	// void AttackTarget();
	// void CastSkill();
	//
	// bool IsManaFull() const;
};
