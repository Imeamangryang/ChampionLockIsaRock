// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_UnitDead.h"

void UAnimNotify_UnitDead::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;
	
	// 사망 애니메이션이 끝난 뒤 액터 숨기기
	// 오버랩도 끄기
	OwnerActor->SetActorHiddenInGame(true);
	OwnerActor->SetActorEnableCollision(false);
}
