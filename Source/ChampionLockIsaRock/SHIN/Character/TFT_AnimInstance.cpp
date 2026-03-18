#include "SHIN/Character/TFT_AnimInstance.h"
#include "GameFramework/Pawn.h"
#include "TFT_UnitCharacter.h"

void UTFT_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	OwnerPawn = TryGetPawnOwner();

	if (OwnerPawn)
	{
		OwnerCharacter = Cast<ATFT_UnitCharacter>(OwnerPawn);
	}
}

void UTFT_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!OwnerPawn)
	{
		OwnerPawn = TryGetPawnOwner();
	}

	if (!OwnerPawn) return;

	// ===== Movement =====
	FVector Velocity = OwnerPawn->GetVelocity();
	Velocity.Z = 0.f;

	Speed = Velocity.Size();
	bIsMoving = Speed > 10.f;

	// ===== Character 상태 가져오기 =====
	if (OwnerCharacter)
	{
		// bIsDead = OwnerCharacter->IsDead();
		// bIsAttacking = OwnerCharacter->IsAttacking();
		// bIsCastingSkill = OwnerCharacter->IsCastingSkill();
	}
}
