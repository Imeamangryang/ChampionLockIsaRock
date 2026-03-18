#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Struct/FStruct_TFT_Champion.h"
#include "../Struct/ETFT_ChampionList.h"
#include "TFT_UnitCharacter.generated.h"

struct FTFT_ChampionData;

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_UnitCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATFT_UnitCharacter();

protected:
	virtual void BeginPlay() override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// data 초기화
	void Initialize(const FTFT_ChampionData& Data, int32 StarLevel);
	
	void InitializeMesh();
	
	// FTFT_ChampionData -> FStruct_TFT_Champion 변환
	static FStruct_TFT_Champion ConvertToChampionData(const FTFT_ChampionData& Data);
	static FName ConvertEnumToRowName(ETFT_ChampionKey Key);
	
	// Champion Name을 FString으로 반환
	FString GetChampionNameString();
	static FString BuildMeshPath(const FString& Name);

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FStruct_TFT_Champion ChampionData;
	
	// Character Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETFT_ChampionKey ChampionKey = ETFT_ChampionKey::Garen;
	
	// Stat Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTFT_StatComponent* StatComponent;
	
	// Skill Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTFT_SkillComponent* SkillComponent;
};
