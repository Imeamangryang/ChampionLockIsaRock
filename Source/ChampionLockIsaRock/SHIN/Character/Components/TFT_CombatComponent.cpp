#include "TFT_CombatComponent.h"
#include "../TFT_UnitCharacter.h"
#include "EngineUtils.h"
#include "NiagaraFunctionLibrary.h"
#include "TFT_SkillComponent.h"
#include "TFT_StatComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "../../Dong/Public/TopDownController.h"
#include "Kismet/GameplayStatics.h"
#include "SHIN/Actors/TFT_DamageTextActor.h"
#include "SHIN/Actors/TFT_SkillProjectile.h"
#include "NiagaraSystem.h"

UTFT_CombatComponent::UTFT_CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	OwnerCharacter = nullptr;

	AttackRange = 150.f;
	AttackRate = 1.0f;
	CurrentAttackTimer = AttackRate;

	CurrentTarget = nullptr;
	CurrentState = ECombatState::Idle;
	CurrentStatePtr = &IdleState;
}

void UTFT_CombatComponent::StartCombat()
{
	if (CurrentState != ECombatState::Idle) return;

	// 전투 시작 시점을 알리고, 타겟 탐색 상태로 전이
	UE_LOG(LogTemp, Warning, TEXT("Combat Started for: %s"), *OwnerCharacter->GetChampionNameString());
	
	// 전투 시작 시에는 서로 막혀서 안겹치도록 
	if (UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionObjectType(ECC_Pawn);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
	// 전투 시작 시에는 마우스 피킹이 되지 않도록
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	
	if (OwnerCharacter->bIsEnemy == false)
	{
		// 전투 시작 시 캐릭터가 바라보는 방향을 뒤집어서 시작 (180도 회전)
		TargetCombatStartRotation = OwnerCharacter->GetActorRotation();
		TargetCombatStartRotation.Yaw += 180.f;
		bRotateToCombatStart = true;
	}
	
	AttackRange = OwnerCharacter->StatComponent->AttackRange * 180.f + 100;
	AttackRate = OwnerCharacter->StatComponent->AttackSpeed;

	// Idle -> Searching으로 상태 변경 (FSM 가동 시작)
	if (GetWorld())
	{
		FTimerHandle CombatStartDelayTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			CombatStartDelayTimerHandle,
			FTimerDelegate::CreateLambda([this]()
			{
				if (!this || !OwnerCharacter)
				{
					return;
				}

				if (CurrentState != ECombatState::Idle)
				{
					return;
				}

				ChangeState(GetSearchingState(), ECombatState::Searching);
			}),
			2.0f,
			false
		);
	}
	
}

void UTFT_CombatComponent::EndCombat()
{
	// 춤 애니메이션 몽타주 재생 
	OwnerCharacter->PlayDanceMontage();
	OwnerCharacter->HPBarWidgetVisible(true);
}

void UTFT_CombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ATFT_UnitCharacter>(GetOwner());

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("UTFT_CombatComponent: Owner is not ATFT_UnitCharacter."));
		return;
	}

	// TODO: 나중에는 StatComponent 또는 ChampionData에서 가져오도록 변경;
	CurrentAttackTimer = AttackRate;

	CurrentTarget = nullptr;
	CurrentState = ECombatState::Idle;
	CurrentStatePtr = &IdleState;
	
	// 델리게이트 바인딩
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		ATopDownController* TopDownController = Cast<ATopDownController>(PC);
		if (TopDownController)
		{
			TopDownController->OnReturnAllUnitsHome.AddUObject(this, &UTFT_CombatComponent::HandleReturnAllUnitsHome);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("CombatComponent initialized for %s | Range: %.1f | Interval: %.2f"),
	*OwnerCharacter->GetChampionNameString(),
		AttackRange,
		AttackRate);
}

void UTFT_CombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (OwnerCharacter && OwnerCharacter->StatComponent)
	{
		if (OwnerCharacter->StatComponent->Health <= 0 && CurrentState != ECombatState::Dead)
		{
			ChangeState(GetDeadState(), ECombatState::Dead);
			return;
		}
	}
	
	// 전투 시작 시 캐릭터가 바라보는 방향을 뒤집어서 시작(180도 회전)
	if (bRotateToCombatStart && OwnerCharacter)
	{
		const FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
		const FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			TargetCombatStartRotation,
			DeltaTime,
			CombatStartRotateSpeed
		);

		OwnerCharacter->SetActorRotation(NewRotation);

		if (NewRotation.Equals(TargetCombatStartRotation, 1.0f))
		{
			OwnerCharacter->SetActorRotation(TargetCombatStartRotation);
			bRotateToCombatStart = false;
		}
	}
	
	if (CurrentStatePtr)
	{
		CurrentStatePtr->Tick(this, DeltaTime);
	}
}

void UTFT_CombatComponent::ChangeState(ICombatState* NewState, ECombatState NewEnumState)
{
	if (!NewState || CurrentStatePtr == NewState) return;

	CurrentStatePtr->Exit(this);
	CurrentStatePtr = NewState;
	CurrentState = NewEnumState;
	CurrentStatePtr->Enter(this);
}

void UTFT_CombatComponent::UpdateCombat(float DeltaTime)
{
	if (!HasTarget())
	{
		ChangeState(GetSearchingState(), ECombatState::Searching);
		return;
	}
	
	CurrentAttackTimer -= DeltaTime;
	if (CurrentAttackTimer <= 0.f)
	{
		AttackTarget();
		ResetAttackTimer();
	}
}

void UTFT_CombatComponent::FindTarget()
{
	if (!OwnerCharacter || !GetWorld())
	{
		CurrentTarget = nullptr;
		return;
	}

	ATFT_UnitCharacter* BestTarget = nullptr;
	float ClosestDistSq = TNumericLimits<float>::Max();

	const FVector MyLocation = OwnerCharacter->GetActorLocation();

	for (TActorIterator<ATFT_UnitCharacter> It(GetWorld()); It; ++It)
	{
		ATFT_UnitCharacter* Candidate = *It;

		if (!IsValid(Candidate))
		{
			continue;
		}

		// 자기 자신 제외
		if (Candidate == OwnerCharacter)
		{
			continue;
		}
		
		// 같은 팀 제외
		if (Candidate->bIsEnemy == OwnerCharacter->bIsEnemy)
		{
			continue;
		}
		
		// 죽은 유닛 제외
		if (!Candidate->StatComponent || Candidate->StatComponent->Health <= 0)
		{
			continue;
		}
		
		// 대기석 제외
		if (Candidate->bIsBenched)
		{
			continue;
		}

		// TODO:
		// 나중에 아래 조건들 추가 추천
		// - 같은 팀 제외
		// - 죽은 유닛 제외

		const float DistSq = FVector::DistSquared(MyLocation, Candidate->GetActorLocation());

		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	CurrentTarget = BestTarget;

	if (CurrentTarget)
	{
		const float Distance = FMath::Sqrt(ClosestDistSq);
		UE_LOG(LogTemp, Log, TEXT("%s found target: %s (Distance: %.1f)"),
			*OwnerCharacter->GetChampionNameString(),
			*CurrentTarget->GetChampionNameString(),
			Distance);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s could not find a target."), *OwnerCharacter->GetName());
	}
}

bool UTFT_CombatComponent::HasTarget() const
{
	if (!OwnerCharacter || !IsValid(CurrentTarget) || CurrentTarget == OwnerCharacter)
	{
		return false;
	}

	if (!CurrentTarget->StatComponent)
	{
		return false;
	}

	return CurrentTarget->StatComponent->Health > 0;
}

bool UTFT_CombatComponent::IsTargetInRange() const
{
	if (!HasTarget()) return false;
	float Distance = FVector::Dist(OwnerCharacter->GetActorLocation(), CurrentTarget->GetActorLocation());
	return Distance <= AttackRange;
}

void UTFT_CombatComponent::MoveToTarget() const
{
	if (!OwnerCharacter || !HasTarget())
	{
		return;
	}

	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	const FVector TargetLocation = CurrentTarget->GetActorLocation();

	FVector Direction = TargetLocation - OwnerLocation;
	Direction.Z = 0.f;

	const float Distance = Direction.Size();

	if (Distance <= AttackRange)
	{
		return;
	}

	if (Direction.IsNearlyZero())
	{
		return;
	}

	Direction.Normalize();

	FVector MoveDirection = Direction;

	UWorld* World = GetWorld();
	if (World)
	{
		const float ForwardTraceDistance = 160.f;
		const float ForwardSphereRadius = 65.f;
		const float SeparationRadius = 150.f;
		const float SeparationWeight = 1.35f;
		const float SideAvoidWeight = 0.9f;
		const FVector TraceStart = OwnerLocation + FVector(0.f, 0.f, 40.f);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(UnitAvoidanceSweep), false, OwnerCharacter);

		auto IsBlockingUnit = [this](const FHitResult& HitResult) -> ATFT_UnitCharacter*
		{
			AActor* HitActor = HitResult.GetActor();
			if (!IsValid(HitActor))
			{
				return nullptr;
			}

			ATFT_UnitCharacter* HitUnit = Cast<ATFT_UnitCharacter>(HitActor);
			if (!HitUnit)
			{
				return nullptr;
			}

			if (HitUnit == CurrentTarget)
			{
				return nullptr;
			}

			if (!HitUnit->StatComponent || HitUnit->StatComponent->Health <= 0)
			{
				return nullptr;
			}

			if (HitUnit->bIsBenched)
			{
				return nullptr;
			}

			return HitUnit;
		};

		// 1) 주변 유닛과 간격 벌리기용 Separation 벡터 계산
		FVector SeparationVector = FVector::ZeroVector;

		TArray<FHitResult> NearbyHits;
		const FCollisionShape SeparationShape = FCollisionShape::MakeSphere(SeparationRadius);

		const bool bFoundNearbyUnits = World->SweepMultiByChannel(
			NearbyHits,
			TraceStart,
			TraceStart,
			FQuat::Identity,
			ECC_Pawn,
			SeparationShape,
			Params
		);

		if (bFoundNearbyUnits)
		{
			for (const FHitResult& Hit : NearbyHits)
			{
				ATFT_UnitCharacter* NearbyUnit = Cast<ATFT_UnitCharacter>(Hit.GetActor());
				if (!NearbyUnit || NearbyUnit == OwnerCharacter || NearbyUnit == CurrentTarget)
				{
					continue;
				}

				if (!NearbyUnit->StatComponent || NearbyUnit->StatComponent->Health <= 0)
				{
					continue;
				}

				FVector AwayDir = OwnerLocation - NearbyUnit->GetActorLocation();
				AwayDir.Z = 0.f;

				const float Dist = AwayDir.Size();
				if (Dist <= KINDA_SMALL_NUMBER)
				{
					continue;
				}

				AwayDir.Normalize();

				// 가까울수록 더 강하게 밀어냄
				const float Strength = 1.f - FMath::Clamp(Dist / SeparationRadius, 0.f, 1.f);
				SeparationVector += AwayDir * Strength;
			}
		}

		if (!SeparationVector.IsNearlyZero())
		{
			SeparationVector.Normalize();
		}

		// 2) 전방 장애물 회피
		FVector AvoidanceVector = FVector::ZeroVector;

		const FCollisionShape ForwardShape = FCollisionShape::MakeSphere(ForwardSphereRadius);
		FHitResult ForwardHit;
		const FVector ForwardEnd = TraceStart + (Direction * ForwardTraceDistance);

		const bool bForwardSweepHit = World->SweepSingleByChannel(
			ForwardHit,
			TraceStart,
			ForwardEnd,
			FQuat::Identity,
			ECC_Pawn,
			ForwardShape,
			Params
		);

		ATFT_UnitCharacter* ForwardBlocker = bForwardSweepHit ? IsBlockingUnit(ForwardHit) : nullptr;

		if (ForwardBlocker)
		{
			const FVector RightDir = FVector::CrossProduct(FVector::UpVector, Direction).GetSafeNormal();
			const FVector LeftDir = -RightDir;

			const FVector LeftTestDir = (Direction + LeftDir).GetSafeNormal();
			const FVector RightTestDir = (Direction + RightDir).GetSafeNormal();

			FHitResult LeftHit;
			FHitResult RightHit;

			const FVector LeftEnd = TraceStart + (LeftTestDir * ForwardTraceDistance);
			const FVector RightEnd = TraceStart + (RightTestDir * ForwardTraceDistance);

			const bool bLeftSweepHit = World->SweepSingleByChannel(
				LeftHit,
				TraceStart,
				LeftEnd,
				FQuat::Identity,
				ECC_Pawn,
				ForwardShape,
				Params
			);

			const bool bRightSweepHit = World->SweepSingleByChannel(
				RightHit,
				TraceStart,
				RightEnd,
				FQuat::Identity,
				ECC_Pawn,
				ForwardShape,
				Params
			);

			const bool bLeftBlocked = bLeftSweepHit && IsBlockingUnit(LeftHit) != nullptr;
			const bool bRightBlocked = bRightSweepHit && IsBlockingUnit(RightHit) != nullptr;

			if (!bLeftBlocked && !bRightBlocked)
			{
				// 기본은 왼쪽 우선, 원하면 나중에 랜덤화 가능
				AvoidanceVector = LeftTestDir;
			}
			else if (!bLeftBlocked)
			{
				AvoidanceVector = LeftTestDir;
			}
			else if (!bRightBlocked)
			{
				AvoidanceVector = RightTestDir;
			}
			else
			{
				AvoidanceVector = RightDir;
			}
		}

		// 3) 타겟 방향 + 회피 + 간격벌리기 합성
		MoveDirection =
			(Direction * 1.0f) +
			(AvoidanceVector * SideAvoidWeight) +
			(SeparationVector * SeparationWeight);

		if (MoveDirection.IsNearlyZero())
		{
			MoveDirection = Direction;
		}
		else
		{
			MoveDirection.Normalize();
		}
	}

	OwnerCharacter->SetActorRotation(MoveDirection.Rotation());
	OwnerCharacter->AddMovementInput(MoveDirection, 1.0f);
}

void UTFT_CombatComponent::AttackTarget() const
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (!HasTarget())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s tried to attack, but has no valid target."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	// 혹시 사거리 밖이면 공격하지 않음
	if (!IsTargetInRange())
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s tried to attack %s, but target is out of range."),
			*OwnerCharacter->GetChampionNameString(),
			*CurrentTarget->GetChampionNameString());
		return;
	}

	// 타겟 방향으로 회전
	FVector Direction = CurrentTarget->GetActorLocation() - OwnerCharacter->GetActorLocation();
	Direction.Z = 0.f;

	if (!Direction.IsNearlyZero())
	{
		FRotator LookAtRotation = Direction.Rotation();
		OwnerCharacter->SetActorRotation(LookAtRotation);
	}

	// 현재는 최소 구현: 로그 출력
	UE_LOG(LogTemp, Log, TEXT("%s attacks %s"),
		*OwnerCharacter->GetChampionNameString(),
		*CurrentTarget->GetChampionNameString());

	// TODO: 여기에 실제 공격 처리 추가
	// 1. 공격 애니메이션 재생 
	OwnerCharacter->PlayAttackMontageByInterval(AttackRate);
}

void UTFT_CombatComponent::OnAttackHitNotify() const
{
	if (OwnerCharacter == nullptr || CurrentTarget == nullptr || CurrentTarget->StatComponent == nullptr)
	{
		return;
	}

	const float Distance = FVector::Dist(
		OwnerCharacter->GetActorLocation(),
		CurrentTarget->GetActorLocation()
	);

	if (Distance > AttackRange)
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s hit notify fired, but target is out of range."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	const int32 Damage = OwnerCharacter->StatComponent
		? OwnerCharacter->StatComponent->AttackDamage
		: 0;

	if (Damage <= 0)
	{
		return;
	}

	const bool bUseProjectileAttack =
		OwnerCharacter->StatComponent &&
		OwnerCharacter->StatComponent->AttackRange >= 2;

	if (bUseProjectileAttack)
	{
		SpawnBasicAttackProjectile(Damage);

		if (OwnerCharacter->StatComponent)
		{
			OwnerCharacter->StatComponent->AddMana(10);
		}

		UE_LOG(LogTemp, Log, TEXT("%s spawned basic attack projectile toward %s"),
			*OwnerCharacter->GetChampionNameString(),
			*CurrentTarget->GetChampionNameString());

		return;
	}

	const FTFTDamageResult DamageResult = CurrentTarget->StatComponent->ApplyDamage(Damage, OwnerCharacter);

	if (DamageResult.FinalDamage > 0)
	{
		const FVector SpawnLocation = CurrentTarget->GetActorLocation();

		if (UWorld* World = GetWorld())
		{
			ATFT_DamageTextActor* DamageTextActor = World->SpawnActor<ATFT_DamageTextActor>(
				ATFT_DamageTextActor::StaticClass(),
				SpawnLocation,
				FRotator::ZeroRotator
			);

			if (DamageTextActor)
			{
				DamageTextActor->InitializeDamageText(DamageResult.FinalDamage, DamageResult.bIsCritical);
			}
		}
	}

	if (DamageResult.bIsCritical)
	{
		UGameplayStatics::PlaySound2D(this, OwnerCharacter->AttackSound);
	}

	if (OwnerCharacter->StatComponent)
	{
		OwnerCharacter->StatComponent->AddMana(10);
	}

	UE_LOG(LogTemp, Log, TEXT("%s dealt %d damage to %s%s (Target HP: %d)"),
		*OwnerCharacter->GetChampionNameString(),
		DamageResult.FinalDamage,
		*CurrentTarget->GetChampionNameString(),
		DamageResult.bIsCritical ? TEXT(" [CRITICAL]") : TEXT(""),
		CurrentTarget->StatComponent->Health);
}

void UTFT_CombatComponent::OnSkillCastNotify()
{
	if (!OwnerCharacter || !OwnerCharacter->SkillComponent)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("%s is casting skill: %s"),
		*OwnerCharacter->GetChampionNameString(),
		*OwnerCharacter->SkillComponent->SkillName.ToString());

	const FString SkillType = OwnerCharacter->SkillComponent->Type.ToString();

	if (SkillType == TEXT("SelfArea"))
	{
		UE_LOG(LogTemp, Log, TEXT("%s is handling SelfArea skill."), *OwnerCharacter->GetChampionNameString());
		HandleSelfAreaSkill();
	}
	else if (SkillType == TEXT("Projectile"))
	{
		UE_LOG(LogTemp, Log, TEXT("%s is handling Projectile skill."), *OwnerCharacter->GetChampionNameString());
		HandleProjectileSkill();
	}
	else if (SkillType == TEXT("TargetArea"))
	{
		UE_LOG(LogTemp, Log, TEXT("%s is handling TargetArea skill."), *OwnerCharacter->GetChampionNameString());
		HandleTargetAreaSkill();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has unknown skill type: %s"),
			*OwnerCharacter->GetChampionNameString(),
			*SkillType);
	}
}

void UTFT_CombatComponent::HandleSelfAreaSkill()
{
	if (!OwnerCharacter || !OwnerCharacter->StatComponent || !OwnerCharacter->SkillComponent || !GetWorld())
	{
		return;
	}

	if (!HasTarget())
	{
		return;
	}

	const int32 StarIndex = FMath::Clamp(OwnerCharacter->starLevel - 1, 0, 2);
	const TArray<float>& SkillValues = OwnerCharacter->SkillComponent->SkillValues;

	if (!SkillValues.IsValidIndex(StarIndex))
	{
		return;
	}

	int32 SkillDamage = FMath::RoundToInt(SkillValues[StarIndex]);
	SkillDamage = SkillDamage + OwnerCharacter->StatComponent->AbilityPower;

	const FTFTDamageResult DamageResult = CurrentTarget->StatComponent->ApplyDamage(SkillDamage, OwnerCharacter);

	if (DamageResult.FinalDamage > 0)
	{
		if (ATFT_DamageTextActor* DamageTextActor = GetWorld()->SpawnActor<ATFT_DamageTextActor>(
			ATFT_DamageTextActor::StaticClass(),
			CurrentTarget->GetActorLocation(),
			FRotator::ZeroRotator))
		{
			DamageTextActor->InitializeDamageText(DamageResult.FinalDamage, DamageResult.bIsCritical);
		}
	}
}

void UTFT_CombatComponent::HandleProjectileSkill()
{
	if (!OwnerCharacter || !OwnerCharacter->StatComponent || !GetWorld())
	{
		return;
	}

	if (!HasTarget())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s tried to cast Projectile skill, but has no target."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	if (!OwnerCharacter->SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no SkillComponent."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	// 1) 별 레벨에 맞는 스킬 데미지 가져오기
	const int32 StarIndex = FMath::Clamp(OwnerCharacter->starLevel - 1, 0, 2);
	const TArray<float>& SkillValues = OwnerCharacter->SkillComponent->SkillValues;

	if (!SkillValues.IsValidIndex(StarIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has invalid SkillValues index for star level %d"),
			*OwnerCharacter->GetChampionNameString(),
			OwnerCharacter->starLevel);
		return;
	}

	int32 SkillDamage = FMath::RoundToInt(SkillValues[StarIndex]);
	if (SkillDamage <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Projectile skill damage is invalid: %d"),
			*OwnerCharacter->GetChampionNameString(),
			SkillDamage);
		return;
	}
	SkillDamage = SkillDamage + OwnerCharacter->StatComponent->AbilityPower;

	UNiagaraSystem* SkillEffect= OwnerCharacter->SkillComponent->SkillEffect;

	if (!SkillEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s SkillEffect is null"), *OwnerCharacter->GetChampionNameString());
	}
	
	// 2) Projectile Class 체크
	TSubclassOf<ATFT_SkillProjectile> ProjectileClass = ATFT_SkillProjectile::StaticClass();
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no ProjectileClass for projectile skill."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	// 3) 스폰 위치/방향 계산
	const FVector SpawnLocation =
		OwnerCharacter->GetActorLocation() +
		OwnerCharacter->GetActorForwardVector() * 50 +
		FVector(0.f, 0.f, 100.f);

	const FVector ToTarget = CurrentTarget->GetActorLocation() - SpawnLocation;
	const FRotator SpawnRotation = ToTarget.IsNearlyZero()
		? OwnerCharacter->GetActorRotation()
		: ToTarget.Rotation();

	// 4) 투사체 스폰
	ATFT_SkillProjectile* Projectile = GetWorld()->SpawnActor<ATFT_SkillProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation
	);

	if (!Projectile)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to spawn projectile skill."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	// 5) 초기화
	Projectile->InitializeProjectile(OwnerCharacter, CurrentTarget, SkillDamage, SkillEffect);

	UE_LOG(LogTemp, Log, TEXT("%s cast Projectile skill on %s | Damage: %d"),
		*OwnerCharacter->GetChampionNameString(),
		*CurrentTarget->GetChampionNameString(),
		SkillDamage);
}

void UTFT_CombatComponent::HandleTargetAreaSkill()
{
	if (!OwnerCharacter || !OwnerCharacter->StatComponent || !GetWorld())
	{
		return;
	}

	if (!HasTarget())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s tried to cast TargetArea skill, but has no target."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	if (!OwnerCharacter->SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no SkillComponent."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	// 1) 별 레벨에 맞는 스킬 데미지 가져오기
	const int32 StarIndex = FMath::Clamp(OwnerCharacter->starLevel - 1, 0, 2);
	const TArray<float>& SkillValues = OwnerCharacter->SkillComponent->SkillValues;

	if (!SkillValues.IsValidIndex(StarIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has invalid SkillValues index for star level %d"),
			*OwnerCharacter->GetChampionNameString(),
			OwnerCharacter->starLevel);
		return;
	}

	int32 SkillDamage = FMath::RoundToInt(SkillValues[StarIndex]);
	if (SkillDamage <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s TargetArea skill damage is invalid: %d"),
			*OwnerCharacter->GetChampionNameString(),
			SkillDamage);
		return;
	}
	SkillDamage = SkillDamage + OwnerCharacter->StatComponent->AbilityPower;

	if (!CurrentTarget || !CurrentTarget->StatComponent || CurrentTarget->StatComponent->Health <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s TargetArea skill failed: invalid current target."),
			*OwnerCharacter->GetChampionNameString());
		return;
	}

	// 2) 타겟 위치에 나이아가라 이펙트 재생
	if (OwnerCharacter->SkillComponent->SkillEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			OwnerCharacter->SkillComponent->SkillEffect,
			CurrentTarget->GetActorLocation() + FVector(0, 0, 50),
			FRotator::ZeroRotator
		);
	}

	// 3) 현재 타겟 한 명에게만 데미지 적용
	const FTFTDamageResult DamageResult = CurrentTarget->StatComponent->ApplyDamage(SkillDamage, OwnerCharacter);

	if (DamageResult.FinalDamage > 0)
	{
		if (ATFT_DamageTextActor* DamageTextActor = GetWorld()->SpawnActor<ATFT_DamageTextActor>(
			ATFT_DamageTextActor::StaticClass(),
			CurrentTarget->GetActorLocation(),
			FRotator::ZeroRotator))
		{
			DamageTextActor->InitializeDamageText(DamageResult.FinalDamage, DamageResult.bIsCritical);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s cast TargetArea skill on %s | Damage: %d"),
		*OwnerCharacter->GetChampionNameString(),
		*CurrentTarget->GetChampionNameString(),
		DamageResult.FinalDamage);
}

void UTFT_CombatComponent::SpawnBasicAttackProjectile(int32 Damage) const
{
	if (!OwnerCharacter || !CurrentTarget || !GetWorld())
	{
		return;
	}

	TSubclassOf<ATFT_SkillProjectile> ProjectileClass = ATFT_SkillProjectile::StaticClass();
	if (!ProjectileClass)
	{
		return;
	}

	const FVector SpawnLocation =
		OwnerCharacter->GetActorLocation() +
		OwnerCharacter->GetActorForwardVector() * 80.f +
		FVector(0.f, 0.f, 100.f);

	const FVector ToTarget = CurrentTarget->GetActorLocation() - SpawnLocation;
	const FRotator SpawnRotation = ToTarget.IsNearlyZero()
		? OwnerCharacter->GetActorRotation()
		: ToTarget.Rotation();

	ATFT_SkillProjectile* Projectile = GetWorld()->SpawnActor<ATFT_SkillProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation
	);

	if (!Projectile)
	{
		return;
	}

	Projectile->InitializeBasicProjectile(OwnerCharacter, CurrentTarget, Damage);
}

void UTFT_CombatComponent::CastSkill()
{
	if (!OwnerCharacter || !OwnerCharacter->StatComponent)
	{
		return;
	}

	StopAllActions();

	if (HasTarget())
	{
		FVector Direction = CurrentTarget->GetActorLocation() - OwnerCharacter->GetActorLocation();
		Direction.Z = 0.f;

		if (!Direction.IsNearlyZero())
		{
			OwnerCharacter->SetActorRotation(Direction.Rotation());
		}
	}

	OwnerCharacter->PlaySkillMontage();

	OwnerCharacter->StatComponent->StartingMana = 0;
	OwnerCharacter->UpdateMPBarWidget();

	if (OwnerCharacter->SkillMontage)
	{
		const float Duration = OwnerCharacter->SkillMontage->GetPlayLength();

		if (Duration > 0.f && GetWorld())
		{
			FTimerHandle SkillEndTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
			SkillEndTimerHandle,
			FTimerDelegate::CreateLambda([this]()
			{
				if (!this || !OwnerCharacter || !OwnerCharacter->StatComponent)
				{
					return;
				}

				if (CurrentState == ECombatState::Dead)
				{
					return;
				}

				if (OwnerCharacter->StatComponent->Health <= 0)
				{
					ChangeState(GetDeadState(), ECombatState::Dead);
					return;
				}

				ChangeState(GetSearchingState(), ECombatState::Searching);
			}),
			Duration,
			false
		);
		}
		else
		{
			ChangeState(GetSearchingState(), ECombatState::Searching);
		}
	}
	else
	{
		ChangeState(GetSearchingState(), ECombatState::Searching);
	}
}

bool UTFT_CombatComponent::IsManaFull() const
{
	if (!OwnerCharacter || !OwnerCharacter->StatComponent || OwnerCharacter->StatComponent->MaxMana == 0)
	{
		return false;
	}
	
	return OwnerCharacter->StatComponent->StartingMana >= OwnerCharacter->StatComponent->MaxMana;
}

void UTFT_CombatComponent::ResetAttackTimer()
{
	if (AttackRate <= 0.f)
	{
		CurrentAttackTimer = 0.f;
		return;
	}

	// AttackInterval은 "초당 공격 횟수"
	CurrentAttackTimer = 1.0f / AttackRate;
}

void UTFT_CombatComponent::StopAllActions() const
{
	if (OwnerCharacter)
	{
		OwnerCharacter->StopMontage(0.15);
	}
}

void UTFT_CombatComponent::HandleReturnAllUnitsHome()
{
	StopAllActions();
	
	// 전투 종료 시 Idle 상태로 복귀 및 타겟 초기화
	CurrentTarget = nullptr;
	ChangeState(GetIdleState(), ECombatState::Idle);
	
	OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	OwnerCharacter->bHideHPBarPermanently = false;
	OwnerCharacter->HPBarWidgetVisible(true);
	OwnerCharacter->UpdateHPBarWidget();
	OwnerCharacter->UpdateMPBarWidget();
}

void UTFT_CombatComponent::Dead() const
{
	if (OwnerCharacter)
	{
		OwnerCharacter->HPBarWidgetVisible(false);
		OwnerCharacter->PlayDeathMontage();
		
		// 사망 모션이 끝나면
		// 사망 애니메이션이 끝난 뒤 액터 숨기기
		// 오버랩도 끄기
		// OwnerCharacter->SetActorHiddenInGame(true);
		// OwnerCharacter->SetActorEnableCollision(false);
	}
}


