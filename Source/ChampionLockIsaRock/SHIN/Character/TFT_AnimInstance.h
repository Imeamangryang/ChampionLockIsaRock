#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TFT_AnimInstance.generated.h"

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:

	// ===== Movement =====
	UPROPERTY(BlueprintReadOnly, Category="State")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="State")
	bool bIsMoving = false;

	// ===== Combat =====
	UPROPERTY(BlueprintReadOnly, Category="State")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category="State")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category="State")
	bool bIsCastingSkill = false;

protected:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	class ATFT_UnitCharacter* OwnerCharacter;
	
};
