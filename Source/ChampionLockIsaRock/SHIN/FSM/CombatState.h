#pragma once

#include "CoreMinimal.h"

class UTFT_CombatComponent;

class ICombatState
{
public:
	virtual ~ICombatState() {}
	virtual void Enter(UTFT_CombatComponent* Combat) {}
	virtual void Tick(UTFT_CombatComponent* Combat, float DeltaTime) {}
	virtual void Exit(UTFT_CombatComponent* Combat) {}
};


class FIdleState : public ICombatState 
{ 
	public: virtual void Enter(UTFT_CombatComponent* Combat) override; 
};

class FSearchingState : public ICombatState
{
	public: virtual void Enter(UTFT_CombatComponent* Combat) override; 
	virtual void Tick(UTFT_CombatComponent* Combat, float DeltaTime) override;
};

class FMovingState : public ICombatState
{
	public: virtual void Tick(UTFT_CombatComponent* Combat, float DeltaTime) override;
};

class FAttackingState : public ICombatState
{
	public: virtual void Enter(UTFT_CombatComponent* Combat) override; 
	virtual void Tick(UTFT_CombatComponent* Combat, float DeltaTime) override;
};

class FCastingSkillState : public ICombatState
{
	public: virtual void Enter(UTFT_CombatComponent* Combat) override; 
	virtual void Tick(UTFT_CombatComponent* Combat, float DeltaTime) override;
};

class FDeadState : public ICombatState
{
	public: virtual void Enter(UTFT_CombatComponent* Combat) override;
};