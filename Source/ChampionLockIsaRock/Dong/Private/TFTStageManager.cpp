// Fill out your copyright notice in the Description page of Project Settings.


#include "Dong/Public/TFTStageManager.h"
#include "Dong/Public/TopDownController.h"
#include "SHIN/Character/TFT_UnitCharacter.h"
#include "SHIN/Character/Components/TFT_CombatComponent.h"
#include "SHIN/Character/Components/TFT_StatComponent.h"
#include "Dong/Public/GirdManager.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ATFTStageManager::ATFTStageManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATFTStageManager::BeginPlay()
{
	Super::BeginPlay();
	
	// 스테이지 개수(StageDataList.Num())만큼 배열을 '시작 안 함(NotStarted)' 상태로 채워넣습니다.
	StageHistory.Init(EStageStatus::NotStarted, StageDataList.Num());
	
	// 게임 시작하자마자 0번 스테이지(1렙) 적들을 소환해라!
	SetupStage(0);
}

void ATFTStageManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsCombatActive) return;

	// 1. 아군 중에서 방금 죽은 유닛이 있는지 감시
	for (int32 i = ActivePlayerUnits.Num() - 1; i >= 0; --i)
	{
		if (ActivePlayerUnits[i] && ActivePlayerUnits[i]->StatComponent->Health <= 0)
		{
			// 찾았다! 네가 만든 OnUnitDied 함수에게 이 유닛을 넘겨줘.
			OnUnitDied(ActivePlayerUnits[i]);
		}
	}

	// 2. 적군 중에서 방금 죽은 유닛이 있는지 감시
	for (int32 i = ActiveEnemyUnits.Num() - 1; i >= 0; --i)
	{
		if (ActiveEnemyUnits[i] && ActiveEnemyUnits[i]->StatComponent->Health <= 0)
		{
			// 찾았다! OnUnitDied 실행해!
			OnUnitDied(ActiveEnemyUnits[i]);
		}
	}
}

void ATFTStageManager::StartRound()
{
	
	// 0. 전투 시작 직전, 현재 필드(Grid)에 올라와 있는 아군 명단을 가져옵니다.
	auto PC = Cast<ATopDownController>(GetWorld()->GetFirstPlayerController());
	if (PC && PC->CachedGridManager)
	{
		ActivePlayerUnits = PC->CachedGridManager->GetDeployedUnits();
	}
	
	bIsCombatActive = true;
	GetWorldTimerManager().SetTimer(OvertimeTimerHandle, this, &ATFTStageManager::StartOvertime, 30.0f, false);
    
	UE_LOG(LogTemp, Warning, TEXT("전투 시작! 30초 후 연장전 돌입."));
	
	// 1. 아군 유닛들에게 전투 시작 명령
	for (auto Unit : ActivePlayerUnits)
	{
		if (IsValid(Unit))
		{
			if (auto CombatComp = Unit->FindComponentByClass<UTFT_CombatComponent>())
			{
				CombatComp->StartCombat();
			}
		}
	}

	// 2. 적군 유닛들에게 전투 시작 명령
	for (auto Enemy : ActiveEnemyUnits)
	{
		if (IsValid(Enemy))
		{
			if (auto CombatComp = Enemy->FindComponentByClass<UTFT_CombatComponent>())
			{
				CombatComp->StartCombat();
			}
		}
	}
}

void ATFTStageManager::EndRound(bool bIsVictory)
{
	if (!bIsCombatActive) return;
	bIsCombatActive = false;

	// 1. 진행 중인 연장전 타이머가 있다면 취소합니다.
	GetWorldTimerManager().ClearTimer(OvertimeTimerHandle);

	// 2. 게임 속도를 다시 정상(1배속)으로 되돌립니다.
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	
	// 3. 현재 승패 상태를 저장해둠
	bLastRoundVictory = bIsVictory;

	// 4. 모든 유닛의 전투 중단 
	for (auto Unit : ActivePlayerUnits)
	{
		if (IsValid(Unit))
		{
			if (auto CombatComp = Unit->FindComponentByClass<UTFT_CombatComponent>())
				CombatComp->EndCombat();
		}
	}
	for (auto Enemy : ActiveEnemyUnits)
	{
		if (IsValid(Enemy))
		{
			if (auto CombatComp = Enemy->FindComponentByClass<UTFT_CombatComponent>())
				CombatComp->EndCombat();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("전투 종료! 3초 후 정산을 시작합니다."));

	// 5. 3초 타이머 설정
	// 이 함수가 실행되고 3초 뒤에 ProceedToNextRound()호출
	GetWorldTimerManager().SetTimer(StageTransitionTimerHandle, this, &ATFTStageManager::ProceedToNextRound, 3.0f, false);
}

void ATFTStageManager::SetupStage(int32 Index)
{
	UE_LOG(LogTemp, Warning, TEXT("SetupStage 실행됨! 스테이지 번호: %d"), Index);
	
	if (!StageDataList.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Error, TEXT("스테이지 데이터가 비어있습니다!"));
		return;
	}

	CleanupBoard(); 
	ActiveEnemyUnits.Empty();

	// 새로운 적 스폰
	for (const auto& Info : StageDataList[Index].Enemies)
	{
		if (!Info.EnemyClass) continue; 
		
		FVector WorldSpawnLoc = GetActorLocation() + Info.EnemySpawnLocations;

		if (auto NewEnemy = GetWorld()->SpawnActor<ATFT_UnitCharacter>(Info.EnemyClass, WorldSpawnLoc, FRotator(0, 0, 0)))
		{
			// 강제로 적군(Enemy) 판정 먹이기
			NewEnemy->bIsEnemy = true;
			NewEnemy->InitWithChampionKey(Info.ChampionKey, Info.StarLevel);
			
			// 소환된 적을 전투 명단(Active)뿐만 아니라 '전체 명단(Spawned)'에도 넣습니다.
			ActiveEnemyUnits.Add(NewEnemy);
			SpawnedEnemies.Add(NewEnemy);
		}
	}
	//방금 세팅한 스테이지를 '현재 진행 중(Current)' 상태로 바꿔줍니다.
	if (StageHistory.IsValidIndex(Index))
	{
		StageHistory[Index] = EStageStatus::Current;
	}
}

// 1. 유닛이 죽었을 때 실행될 로직
void ATFTStageManager::OnUnitDied(ATFT_UnitCharacter* DeadUnit)
{
	if (!DeadUnit) return;

	// 명단에서 제거 (여기서 제거되기 때문에 다음 Tick에서는 이 유닛을 다시 찾지 않음)
	if (ActivePlayerUnits.Contains(DeadUnit)) ActivePlayerUnits.Remove(DeadUnit);
	if (ActiveEnemyUnits.Contains(DeadUnit)) ActiveEnemyUnits.Remove(DeadUnit);

	UE_LOG(LogTemp, Warning, TEXT("유닛 사망 감지: %s | 남은 적: %d"), *DeadUnit->GetName(), ActiveEnemyUnits.Num());

	// 승패 판단 (이 로직도 그대로 유지!)
	if (ActiveEnemyUnits.Num() == 0)
	{
		EndRound(true); 
	}
	else if (ActivePlayerUnits.Num() == 0)
	{
		EndRound(false);
	}
}

void ATFTStageManager::ResetUnits()
{
	auto PC = Cast<ATopDownController>(GetWorld()->GetFirstPlayerController());
	if (!PC) return;

	// UnitHomeRegistry는 필드에 한 번이라도 올렸던 모든 아군 기물
	for (auto& Elem : PC->UnitHomeRegistry)
	{
		ATFT_UnitCharacter* Unit = Elem.Key;
       
		if (IsValid(Unit))
		{
			// 1. 팀원이 숨긴 기물을 다시 보이게 하고, 충돌을 킴
			Unit->SetActorHiddenInGame(false);
			Unit->SetActorEnableCollision(true);
			
			//죽었을 때 마우스 클릭(Visibility) 판정이 꺼진 것을 강제로 다시 켭니다
			TArray<UPrimitiveComponent*> PrimitiveComps;
			Unit->GetComponents<UPrimitiveComponent>(PrimitiveComps);
			for (UPrimitiveComponent* Comp : PrimitiveComps)
			{
				// 마우스 레이저(Visibility)에 무조건 걸리도록(Block) 설정
				Comp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			}

			// 2. 위치 복구 (원래 배치했던 그 자리로)
			Unit->SetActorTransform(Elem.Value);
          
			// 3. 스탯 및 UI 복구
			if (Unit->StatComponent)
			{
				Unit->StatComponent->Health = Unit->StatComponent->MaxHealth;
				Unit->StatComponent->StartingMana = 0;
				Unit->UpdateHPBarWidget();
				Unit->UpdateMPBarWidget();
			}

			// 4. [상태 리셋] CombatComponent를 Idle 상태로 돌려놓습니다.
			// 이걸 안 하면 기물들이 살아나도 계속 'DeadState'
			if (auto CombatComp = Unit->FindComponentByClass<UTFT_CombatComponent>())
			{
				CombatComp->EndCombat(); // 팀원 코드: 상태를 Idle로 바꾸고 타겟 초기화함
			}
          
			// 5. 애니메이션 리셋 (혹시 죽는 애니메이션에서 멈춰있을까봐)
			Unit->StopMontage(0.1f); 
		}
	}
    
	// 다음 라운드를 위해 내 아군 명단(ActivePlayerUnits)도 일단 비움
	// StartRound가 실행될 때 다시 GridManager에서 생존자 명단을 새로 받아옴
	ActivePlayerUnits.Empty();
}

// 3초 뒤에 실행될 진짜 정산 로직
void ATFTStageManager::ProceedToNextRound()
{
	// 다음 스테이지로 넘어가기전에, 방금 끝난 스테이지에 승패 여부를 찍음
	if (StageHistory.IsValidIndex(CurrentStageIndex))
	{
		StageHistory[CurrentStageIndex] = bLastRoundVictory ? EStageStatus::Won : EStageStatus::Lost;
	}
	
	if (bLastRoundVictory)
	{
		UE_LOG(LogTemp, Warning, TEXT("정산: 승리! 다음 스테이지 준비."));
		CurrentStageIndex++;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("정산: 패배... 현재 스테이지 다시 도전."));
	}

	// 아군 부활 및 적군 스폰
	ResetUnits();
	SetupStage(CurrentStageIndex);
	
	OnPreparationStarted.Broadcast();
}

void ATFTStageManager::StartOvertime()
{
	if (bIsCombatActive)
	{
		// 전역 게임 속도를 2배로 설정합니다.
		// 유닛의 애니메이션, 이동, 투사체 속도 등 모든 것이 2배 빨라집니다.
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 2.0f);

		UE_LOG(LogTemp, Error, TEXT("!!! 연장전 돌입: 2배속 시작 !!!"));
	}
}

float ATFTStageManager::GetRoundTimePct() const
{
	// 1. 전투 중일 때 (30초 타이머 사용)
	if (bIsCombatActive)
	{
		float Remaining = GetWorldTimerManager().GetTimerRemaining(OvertimeTimerHandle);
		return FMath::Clamp(Remaining / 30.0f, 0.0f, 1.0f);
	}

	// 2. 전투 종료 후 3초 정산 중일 때 (3초 타이머 사용)
	if (GetWorldTimerManager().IsTimerActive(StageTransitionTimerHandle))
	{
		float Remaining = GetWorldTimerManager().GetTimerRemaining(StageTransitionTimerHandle);
		return FMath::Clamp(Remaining / 3.0f, 0.0f, 1.0f);
	}

	// 둘 다 아니면 게이지를 비워둠
	return 0.0f;
}

int32 ATFTStageManager::GetRemainingRoundTimeInt() const
{
	float Remaining = -1.0f;

	// 1. 전투 중일 때
	if (bIsCombatActive)
	{
		Remaining = GetWorldTimerManager().GetTimerRemaining(OvertimeTimerHandle);
	}
	// 2. 정산 중일 때
	else if (GetWorldTimerManager().IsTimerActive(StageTransitionTimerHandle))
	{
		Remaining = GetWorldTimerManager().GetTimerRemaining(StageTransitionTimerHandle);
	}
	// Remaining이 0보다 작으면(즉, -1이면) 0을 반환
	return (Remaining < 0.f) ? 0 : FMath::CeilToInt(Remaining);
}

void ATFTStageManager::CleanupBoard()
{
	// 1. 소환된 적 배열을 돌며 하나씩 파괴
	for (AActor* Enemy : SpawnedEnemies)
	{
		if (Enemy && IsValid(Enemy))
		{
			Enemy->Destroy();
		}
	}

	// 2. 배열 비우기 (중요: 이걸 안 하면 다음 라운드 때 죽은 주소를 참조하게 됩니다)
	SpawnedEnemies.Empty();
}

bool ATFTStageManager::IsBoardLocked() const
{
	// 전투 중이거나(bIsCombatActive), 3초 정산 타이머가 아직 째깍거리고 있다면 true
	return bIsCombatActive || GetWorldTimerManager().IsTimerActive(StageTransitionTimerHandle);
}