// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_SynergyComponent.h"
#include "Dong/Public/TopDownController.h"
#include "Dong/Public/GirdManager.h"
#include "SHIN/Character/TFT_UnitCharacter.h"
#include "UObject/Class.h"

UTFT_SynergyComponent::UTFT_SynergyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UTFT_SynergyComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UTFT_SynergyComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTFT_SynergyComponent::RecalculateSynergies()
{
    // 1. 장부 초기화
    ActiveSynergyData.Counts.Empty();

    ATopDownController* PC = Cast<ATopDownController>(GetOwner());
    if (!PC || !PC->CachedGridManager) return;
    
    TArray<ATFT_UnitCharacter*> DeployedUnits = PC->CachedGridManager->GetDeployedUnits();

    TSet<FString> UniqueChampions; 

    for (ATFT_UnitCharacter* Unit : DeployedUnits)
    {
        // 살아있고, 벤치에 없으며, 적군이 아닐 때
        if (IsValid(Unit) && !Unit->bIsBenched && !Unit->bIsEnemy)
        {
            FString UKey = Unit->ChampionData.Key;

            if (!UniqueChampions.Contains(UKey))
            {
                UniqueChampions.Add(UKey);

                // 시너지 정보 가져오기
                FString OriginStr = Unit->ChampionData.Origins.ToString();
                FString ClassStr = Unit->ChampionData.Classes.ToString();
                FString Combined = OriginStr + TEXT(",") + ClassStr;

                TArray<FString> SplitTraits;
                Combined.ParseIntoArray(SplitTraits, TEXT(","), true);

                for (const FString& TraitStr : SplitTraits) 
                {
                    FString CleanName = TraitStr.TrimStartAndEnd();
    
                    if (!CleanName.IsEmpty())
                    {
                        // 장부에 시너지 개수 추가
                        int32& Count = ActiveSynergyData.Counts.FindOrAdd(CleanName);
                        Count++;
                    }
                }
            }
        }
    }

    // 2. UI로 데이터 전송
    OnSynergyUpdated.Broadcast(ActiveSynergyData);
}
