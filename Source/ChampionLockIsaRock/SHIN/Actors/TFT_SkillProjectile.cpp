#include "TFT_SkillProjectile.h"
#include "SHIN/Character/TFT_UnitCharacter.h"
#include "SHIN/Character/Components/TFT_StatComponent.h"
#include "SHIN/Actors/TFT_DamageTextActor.h"
#include "Components/SceneComponent.h"

ATFT_SkillProjectile::ATFT_SkillProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	InitialLifeSpan = 0.f;
}

void ATFT_SkillProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(MaxLifetime);
}

void ATFT_SkillProjectile::InitializeProjectile(
	ATFT_UnitCharacter* InCaster,
	ATFT_UnitCharacter* InTarget,
	int32 InDamage)
{
	Caster = InCaster;
	Target = InTarget;
	Damage = InDamage;
}

void ATFT_SkillProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHitProcessed)
	{
		return;
	}

	if (!IsValid(Target) || !Target->StatComponent || Target->StatComponent->Health <= 0)
	{
		Destroy();
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation() + FVector(0.f, 0.f, 80.f);

	FVector MoveDirection = TargetLocation - CurrentLocation;
	const float Distance = MoveDirection.Size();

	if (Distance <= HitRadius)
	{
		HandleHit();
		return;
	}

	if (MoveDirection.IsNearlyZero())
	{
		return;
	}

	MoveDirection.Normalize();

	const FVector NewLocation = CurrentLocation + MoveDirection * MoveSpeed * DeltaTime;
	SetActorLocation(NewLocation);
	SetActorRotation(MoveDirection.Rotation());
}

void ATFT_SkillProjectile::HandleHit()
{
	if (bHitProcessed)
	{
		return;
	}

	bHitProcessed = true;

	if (!IsValid(Caster) || !IsValid(Target) || !Target->StatComponent)
	{
		Destroy();
		return;
	}

	const FTFTDamageResult DamageResult = Target->StatComponent->ApplyDamage(Damage, Caster);

	if (DamageResult.FinalDamage > 0)
	{
		if (UWorld* World = GetWorld())
		{
			ATFT_DamageTextActor* DamageTextActor = World->SpawnActor<ATFT_DamageTextActor>(
				ATFT_DamageTextActor::StaticClass(),
				Target->GetActorLocation(),
				FRotator::ZeroRotator
			);

			if (DamageTextActor)
			{
				DamageTextActor->InitializeDamageText(DamageResult.FinalDamage, DamageResult.bIsCritical);
			}
		}
	}

	Destroy();
}