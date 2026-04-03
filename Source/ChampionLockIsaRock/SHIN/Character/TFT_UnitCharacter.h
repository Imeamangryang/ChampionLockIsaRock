#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SHIN/Struct/FStruct_TFT_Champion.h"
#include "SHIN/Struct/ETFT_ChampionList.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "TFT_UnitCharacter.generated.h"

struct FTFT_ChampionData;

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_UnitCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATFT_UnitCharacter();
	
	// Champion Name을 FString으로 반환
	FString GetChampionNameString();
	static FString BuildMeshPath(const FString& Name);
	
	void PlayAttackMontageByInterval(float AttackRate);
	
	void PlayDeathMontage();
	
	void PlayDanceMontage();
	
	void StopMontage(float BlendOutTime = 0.15f);
	
	void UpdateHPBarWidget();
	void UpdateMPBarWidget();
	void HPBarWidgetVisible(bool bIsVisible);
	
	// data 초기화
	void Initialize(const FTFT_ChampionData& Data, int32 StarLevel);
	
	void InitializeMesh();
	
	void InitWithChampionKey(ETFT_ChampionKey InChampionKey, int32 InStarLevel);
	
	UFUNCTION(blueprintCallable, Category="TFT|Item")
	void ItemTest();
	

protected:
	virtual void BeginPlay() override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// FTFT_ChampionData -> FStruct_TFT_Champion 변환
	static FStruct_TFT_Champion ConvertToChampionData(const FTFT_ChampionData& Data);
	static FName ConvertEnumToRowName(ETFT_ChampionKey Key);


public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TFT|Item")
	TArray<FStruct_TFTEquippedItemSlot> EquippedItemSlots;

	UFUNCTION(BlueprintCallable, Category="TFT|Item")
	void InitializeItemSlots();

	UFUNCTION(BlueprintCallable, Category="TFT|Item")
	int32 FindFirstEmptyItemSlot() const;

	UFUNCTION(BlueprintCallable, Category="TFT|Item")
	bool HasEmptyItemSlot() const;

	UFUNCTION(BlueprintCallable, Category="TFT|Item")
	bool TryEquipItem(const FStruct_TFTItemInstance& ItemInstance);

	UFUNCTION(BlueprintCallable, Category="TFT|Item")
	void RefreshItemSlotWidget();
	
	UFUNCTION(BlueprintCallable, Category="TFT|Item")
	UTexture2D* GetItemIconByItemId(FName ItemId) const;
	
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
	
	// Combat Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTFT_CombatComponent* CombatComponent;
	
	// HPBar Widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UWidgetComponent* HPBarWidgetComponent;
	
	// Attack Montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* AttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* DeathMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* DanceMontage;
	
	
	// Enemy 여부 (적이면 true, 아군이면 false)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEnemy = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 starLevel = 1; 
};
