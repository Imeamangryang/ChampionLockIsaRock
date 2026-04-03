// Fill out your copyright notice in the Description page of Project Settings.


#include "Dong/Public/UnitManager.h"
#include "EngineUtils.h"
#include "SHIN/GameFramework/TFT_GameInstance.h"
#include "SHIN/Character/TFT_UnitCharacter.h"
#include "SHIN/Struct/FTFT_ChampionData.h"
#include "Dong/Public/BenchManager.h"
#include "Dong/Public/GirdManager.h"
#include "Dong/Public/TopDownController.h"
#include "Kismet/GameplayStatics.h"

AUnitManager::AUnitManager()
{
    PrimaryActorTick.bCanEverTick = true; 
    PrimaryActorTick.bStartWithTickEnabled = false; 
}

void AUnitManager::BeginPlay()
{
    Super::BeginPlay();
    
    // 매니저 자동 할당
    if (!BenchManager) BenchManager = Cast<ABenchManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABenchManager::StaticClass()));
    if (!GirdManager) GirdManager = Cast<AGirdManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGirdManager::StaticClass()));
}

void AUnitManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsEvolving && MainUnit_Ptr && Fodder1_Ptr && Fodder2_Ptr)
    {
        EvolutionAlpha += DeltaTime * EvolutionSpeed;
        FVector TargetLoc = MainUnit_Ptr->GetActorLocation();

        Fodder1_Ptr->SetActorLocation(FMath::Lerp(F1_StartLoc, TargetLoc, EvolutionAlpha));
        Fodder2_Ptr->SetActorLocation(FMath::Lerp(F2_StartLoc, TargetLoc, EvolutionAlpha));

        if (EvolutionAlpha >= 1.0f)
        {
            bIsEvolving = false;
            SetActorTickEnabled(false);
            FinishUpgrade(MainUnit_Ptr, Fodder1_Ptr, Fodder2_Ptr, PendingKey, PendingLevel);
        }
    }
}
 
void AUnitManager::TryUpgradeUnit(ETFT_ChampionKey UnitKey, int32 StarLevel)
{
    if (StarLevel >= 3 || bIsEvolving) return; // 이미 합체 중이면 중복 방지

    TArray<ATFT_UnitCharacter*> FoundUnits;
    for (TActorIterator<ATFT_UnitCharacter> It(GetWorld()); It; ++It)
    {
        ATFT_UnitCharacter* Unit = *It;
        if (Unit && Unit->ChampionKey == UnitKey && Unit->starLevel == StarLevel && !Unit->bIsEnemy)
        {
            FoundUnits.Add(Unit);
        }
    }

    if (FoundUnits.Num() >= 3)
    {
        ATopDownController* PC = Cast<ATopDownController>(GetWorld()->GetFirstPlayerController());
        if (!PC || !BenchManager) return;

        // 메인 기물 우선순위 결정 (필드 > 대기석 왼쪽)
        ATFT_UnitCharacter* MainUnit = nullptr;
        for (ATFT_UnitCharacter* Unit : FoundUnits) {
            if (!PC->IsUnitOnBench(Unit)) { MainUnit = Unit; break; }
        }
        if (!MainUnit) {
            int32 LowestIdx = 99;
            for (ATFT_UnitCharacter* Unit : FoundUnits) {
                int32 CurrentIdx = BenchManager->BenchUnits.Find(Unit);
                if (CurrentIdx != INDEX_NONE && CurrentIdx < LowestIdx) { LowestIdx = CurrentIdx; MainUnit = Unit; }
            }
        }
        if (!MainUnit) MainUnit = FoundUnits[0];

        // 재료 분류
        TArray<ATFT_UnitCharacter*> Fodders;
        for (ATFT_UnitCharacter* Unit : FoundUnits) {
            if (Unit != MainUnit) Fodders.Add(Unit);
        }

        // 애니메이션 셋팅
        if (Fodders.Num() >= 2) {
            MainUnit_Ptr = MainUnit;
            Fodder1_Ptr = Fodders[0];
            Fodder2_Ptr = Fodders[1];
            F1_StartLoc = Fodder1_Ptr->GetActorLocation();
            F2_StartLoc = Fodder2_Ptr->GetActorLocation();
            PendingKey = UnitKey;
            PendingLevel = StarLevel + 1;

            Fodder1_Ptr->SetActorEnableCollision(false);
            Fodder2_Ptr->SetActorEnableCollision(false);

            EvolutionAlpha = 0.0f;
            bIsEvolving = true;
            SetActorTickEnabled(true);
        }
    }
}

void AUnitManager::FinishUpgrade(ATFT_UnitCharacter* TargetUnit, ATFT_UnitCharacter* F1, ATFT_UnitCharacter* F2, ETFT_ChampionKey UnitKey, int32 NewStarLevel)
{
    if (!TargetUnit || !F1 || !F2) return;
    
    ATopDownController* PC = Cast<ATopDownController>(GetWorld()->GetFirstPlayerController());
    
    if (BenchManager) 
    {
        BenchManager->ClearUnitFromBench(F1);
        BenchManager->ClearUnitFromBench(F2);
    }

    if (PC)
    {
        PC->UnitHomeRegistry.Remove(F1);
        PC->UnitHomeRegistry.Remove(F2);
    }
    
    F1->Destroy();
    F2->Destroy();

    // 데이터 갱신
    if (UTFT_GameInstance* GI = Cast<UTFT_GameInstance>(GetGameInstance())) {
        if (UDataTable* Table = GI->ChampionDataTable.LoadSynchronous()) {
            const UEnum* EnumPtr = StaticEnum<ETFT_ChampionKey>();
            FName RowName = FName(EnumPtr->GetNameStringByValue((int64)UnitKey));
            const FTFT_ChampionData* Data = Table->FindRow<FTFT_ChampionData>(RowName, TEXT(""));
            if (Data) { 
                TargetUnit->InitWithChampionKey(UnitKey, NewStarLevel);
                // 3성이면 1.5배, 2성이면 1.2배
                float NewScale = (NewStarLevel == 3) ? 1.5f : 1.2f; 
                TargetUnit->SetActorScale3D(FVector(NewScale));
            }
        }
    }

    // 블루프린트에 이펙트 터뜨리라고 신호 보내기
    BP_OnFusionComplete(TargetUnit);

    // 연쇄 합체 체크 (2성 3개가 모여 3성이 되는 경우)
    TryUpgradeUnit(UnitKey, NewStarLevel);
}

