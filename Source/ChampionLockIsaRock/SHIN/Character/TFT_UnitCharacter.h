#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SHIN/Struct/BaseModifier.h"
#include "SHIN/Struct/FStruct_TFT_Champion.h"
#include "SHIN/Struct/ETFT_ChampionList.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "TFT_UnitCharacter.generated.h"

struct FTFT_ChampionData;
class UTFT_HPBarWidget;
class USoundBase;

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
	
	void PlaySkillMontage();
	
	void StopMontage(float BlendOutTime = 0.15f);
	
	void UpdateHPBarWidget();
	void UpdateMPBarWidget();
	void HPBarWidgetVisible(bool bIsVisible);
	
	// data 초기화
	void Initialize(const FTFT_ChampionData& Data, int32 StarLevel);
	
	void InitializeMesh();
	
	void InitWithChampionKey(ETFT_ChampionKey InChampionKey, int32 InStarLevel);
	
	static TArray<FBaseModifier> BuildItemModifiers(const FStruct_TFTItemDefinition& ItemDef, UObject* Source);
	
	UFUNCTION(BlueprintCallable, Category="TFT|Item")
	void ItemTest();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// FTFT_ChampionData -> FStruct_TFT_Champion 변환
	static FStruct_TFT_Champion ConvertToChampionData(const FTFT_ChampionData& Data);
	static FName ConvertEnumToRowName(ETFT_ChampionKey Key);

	// 화면 좌표 기반 HP바 생성/위치 갱신
	void CreateHPBarWidget();
	void UpdateHPBarScreenPosition();
	FVector GetHPBarWorldAnchorLocation() const;

protected:
	// HPBar 화면 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category="TFT|UI")
	TSubclassOf<UTFT_HPBarWidget> HPBarWidgetClass;

	// 실제 화면에 올라간 HPBar 위젯
	UPROPERTY()
	TObjectPtr<UTFT_HPBarWidget> HPBarScreenWidget;

	// 월드 기준 앵커 위치(머리 위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|UI")
	FVector HPBarWorldOffset = FVector(0.f, 0.f, -40.f);

	// 화면 기준 추가 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|UI")
	FVector2D HPBarScreenOffset = FVector2D(0.f, 0.f);

	// 화면 아래쪽일수록 조금 더 위로 올리는 최대 추가 보정치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|UI")
	float MaxScreenLiftAtBottom = 30.f;
	


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
	
	// Attack Montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* AttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* DeathMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* DanceMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* SkillMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* DeathSound = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* AttackSound = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* SkillSound = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* ItemEquipSound = nullptr;
	
	// Enemy 여부 (적이면 true, 아군이면 false)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEnemy = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 starLevel = 1; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBenched = true;
	
	UPROPERTY()
	bool bHideHPBarPermanently = false;
};