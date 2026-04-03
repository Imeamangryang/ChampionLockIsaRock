// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AttackHit.h"
#include "SHIN/Character/Components/TFT_CombatComponent.h"

void UAnimNotify_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;
	UTFT_CombatComponent* CombatComponent = OwnerActor->FindComponentByClass<UTFT_CombatComponent>();
	if (!CombatComponent) return;

	CombatComponent->OnAttackHitNotify();
}
