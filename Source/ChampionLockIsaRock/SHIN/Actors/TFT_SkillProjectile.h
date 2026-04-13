#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFT_SkillProjectile.generated.h"

class ATFT_UnitCharacter;
class USceneComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_SkillProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	ATFT_SkillProjectile();

	virtual void Tick(float DeltaTime) override;

	void InitializeProjectile(
		ATFT_UnitCharacter* InCaster,
		ATFT_UnitCharacter* InTarget,
		int32 InDamage,
		UNiagaraSystem* InEffect
	);
	
	void InitializeBasicProjectile(
		ATFT_UnitCharacter* InCaster,
		ATFT_UnitCharacter* InTarget,
		int32 InDamage
	);

protected:
	virtual void BeginPlay() override;

	void HandleHit();

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	TObjectPtr<UNiagaraComponent> SkillEffectComponent;

	UPROPERTY()
	ATFT_UnitCharacter* Caster;

	UPROPERTY()
	ATFT_UnitCharacter* Target;

	UPROPERTY(EditAnywhere, Category="Projectile")
	float MoveSpeed = 900.f;

	UPROPERTY(EditAnywhere, Category="Projectile")
	float HitRadius = 10.f;

	UPROPERTY(EditAnywhere, Category="Projectile")
	float MaxLifetime = 5.f;

	UPROPERTY()
	int32 Damage = 0;

	UPROPERTY()
	bool bHitProcessed = false;
};