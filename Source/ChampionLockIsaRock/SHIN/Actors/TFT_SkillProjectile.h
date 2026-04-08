#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFT_SkillProjectile.generated.h"

class ATFT_UnitCharacter;
class USceneComponent;

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
		int32 InDamage
	);

protected:
	virtual void BeginPlay() override;

	void HandleHit();

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY()
	ATFT_UnitCharacter* Caster;

	UPROPERTY()
	ATFT_UnitCharacter* Target;

	UPROPERTY(EditAnywhere, Category="Projectile")
	float MoveSpeed = 900.f;

	UPROPERTY(EditAnywhere, Category="Projectile")
	float HitRadius = 50.f;

	UPROPERTY(EditAnywhere, Category="Projectile")
	float MaxLifetime = 5.f;

	UPROPERTY()
	int32 Damage = 0;

	UPROPERTY()
	bool bHitProcessed = false;
};