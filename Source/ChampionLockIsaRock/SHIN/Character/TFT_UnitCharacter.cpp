#include "TFT_UnitCharacter.h"
#include "Components/TFT_StatComponent.h"
#include "Components/TFT_SkillComponent.h"
#include "Components/TFT_CombatComponent.h"
#include "../Struct/FTFT_ChampionData.h"
#include "../GameFramework/TFT_GameInstance.h"
#include "UI/TFT_HPBarWidget.h"
#include "Components/CapsuleComponent.h"
#include "SHIN/Character/Components/TFT_ItemInventoryComponent.h"
#include "SHIN/Struct/BaseModifier.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ATFT_UnitCharacter::ATFT_UnitCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	StatComponent = CreateDefaultSubobject<UTFT_StatComponent>(TEXT("StatComponent"));
	SkillComponent = CreateDefaultSubobject<UTFT_SkillComponent>(TEXT("SkillComponent"));
	CombatComponent = CreateDefaultSubobject<UTFT_CombatComponent>(TEXT("CombatComponent"));

	static ConstructorHelpers::FClassFinder<UTFT_HPBarWidget> HPBarWidgetBPClass(TEXT("/Game/SHIN/UI/Blueprints/WBP_HealthBar.WBP_HealthBar_C"));
	if (HPBarWidgetBPClass.Succeeded())
	{
		HPBarWidgetClass = HPBarWidgetBPClass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load HP Bar Widget class."));
	}
	
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionObjectType(ECC_Pawn);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
	
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
	
	static ConstructorHelpers::FObjectFinder<USoundBase> DeathSoundAsset(TEXT("/Game/SHIN/Sound/Killstreak_SFX_Assist_1.Killstreak_SFX_Assist_1"));
	if (DeathSoundAsset.Succeeded())
	{
		DeathSound = DeathSoundAsset.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundBase> AttackSoundAsset(TEXT("/Game/SHIN/Sound/BasicAttackSound_1.BasicAttackSound_1"));
	if (AttackSoundAsset.Succeeded())
	{
		AttackSound = AttackSoundAsset.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundBase> SkillSoundAsset(TEXT("/Game/SHIN/Sound/SkillSound.SkillSound"));
	if (SkillSoundAsset.Succeeded())
	{
		SkillSound = SkillSoundAsset.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundBase> ItemEquipSoundAsset(TEXT("/Game/SHIN/Sound/IteminUnit_out.IteminUnit_out"));
	if (ItemEquipSoundAsset.Succeeded())
	{
		ItemEquipSound = ItemEquipSoundAsset.Object;
	}
}

void ATFT_UnitCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (bIsEnemy)
	{
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	}

	InitializeItemSlots();
	InitWithChampionKey(ChampionKey, starLevel);
	CreateHPBarWidget();
}

void ATFT_UnitCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	FString Name = GetChampionNameString();
	if (Name.IsEmpty()) return;
	
	FString Path = BuildMeshPath(Name);
	USkeletalMesh* CharacterMeshs = LoadObject<USkeletalMesh>(nullptr, *Path);

	if (CharacterMeshs)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshs);
	}
	else
	{
		GetMesh()->SetSkeletalMesh(nullptr);
		UE_LOG(LogTemp, Warning, TEXT("Failed to load mesh: %s"), *Path);
	}
	
	FString AnimPath = FString::Printf(TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/%s_Skeleton_AnimBlueprint.%s_Skeleton_AnimBlueprint_C"), *Name, *Name, *Name);

	if (UClass* AnimClass = LoadObject<UClass>(nullptr, *AnimPath))
	{
		GetMesh()->SetAnimInstanceClass(AnimClass);
	}
	else
	{
		GetMesh()->SetAnimInstanceClass(nullptr);
		UE_LOG(LogTemp, Warning, TEXT("Failed to load animation blueprint: %s"), *AnimPath);
	}
#endif
}

void ATFT_UnitCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (HPBarScreenWidget)
	{
		HPBarScreenWidget->RemoveFromParent();
		HPBarScreenWidget = nullptr;
	}
}

void ATFT_UnitCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHPBarScreenPosition();
}

void ATFT_UnitCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ATFT_UnitCharacter::CreateHPBarWidget()
{
	if (HPBarScreenWidget || !HPBarWidgetClass)
	{
		return;
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	HPBarScreenWidget = CreateWidget<UTFT_HPBarWidget>(PC, HPBarWidgetClass);
	if (!HPBarScreenWidget)
	{
		return;
	}

	HPBarScreenWidget->AddToViewport();
	HPBarScreenWidget->SetOwnerCharacter(this);
	HPBarScreenWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

	HPBarScreenWidget->SetAlignmentInViewport(FVector2D(0.5f, 1.0f));
	UpdateHPBarScreenPosition();
}

FVector ATFT_UnitCharacter::GetHPBarWorldAnchorLocation() const
{
	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		return GetActorLocation() + FVector(0.f, 0.f, CapsuleHalfHeight + 30.f);
	}

	return GetActorLocation() + HPBarWorldOffset;
}

void ATFT_UnitCharacter::UpdateHPBarScreenPosition()
{
	if (!HPBarScreenWidget)
	{
		return;
	}
	
	if (bHideHPBarPermanently)
	{
		HPBarScreenWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	FVector2D ScreenPosition;
	const bool bProjected = PC->ProjectWorldLocationToScreen(GetHPBarWorldAnchorLocation(), ScreenPosition, true);

	if (!bProjected)
	{
		HPBarScreenWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);

	if (ViewportX <= 0 || ViewportY <= 0)
	{
		return;
	}

	// 화면 아래쪽에 있는 유닛일수록 HP바를 조금 더 위로 올려서 몸을 덜 가리게 함
	const float NormalizedY = ScreenPosition.Y / static_cast<float>(ViewportY);
	const float ExtraLift = FMath::Lerp(0.f, MaxScreenLiftAtBottom, NormalizedY);

	ScreenPosition += HPBarScreenOffset;
	ScreenPosition.Y -= ExtraLift;

	HPBarScreenWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	HPBarScreenWidget->SetPositionInViewport(ScreenPosition, true);
}

void ATFT_UnitCharacter::Initialize(const FTFT_ChampionData& Data, int32 StarLevel)
{
	ChampionData = ConvertToChampionData(Data);
	
	if (StatComponent)
	{
		StatComponent->Initialize(ChampionData, StarLevel);
	}

	if (SkillComponent)
	{
		SkillComponent->Initialize(ChampionData);
	}
}

void ATFT_UnitCharacter::InitializeMesh()
{
	FString Name = GetChampionNameString();
	if (Name.IsEmpty()) return;
	
	FString Path = BuildMeshPath(Name);
	USkeletalMesh* CharacterMeshs = LoadObject<USkeletalMesh>(nullptr, *Path);

	if (CharacterMeshs)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshs);
	}
	else
	{
		GetMesh()->SetSkeletalMesh(nullptr);
		UE_LOG(LogTemp, Warning, TEXT("Failed to load mesh: %s"), *Path);
	}
	
	FString AnimPath = FString::Printf(TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/%s_Skeleton_AnimBlueprint.%s_Skeleton_AnimBlueprint_C"), *Name, *Name, *Name);

	if (UClass* AnimClass = LoadObject<UClass>(nullptr, *AnimPath))
	{
		GetMesh()->SetAnimInstanceClass(AnimClass);
	}
	else
	{
		GetMesh()->SetAnimInstanceClass(nullptr);
		UE_LOG(LogTemp, Warning, TEXT("Failed to load animation blueprint: %s"), *AnimPath);
	}
	
	FString MontagePath = FString::Printf(
	TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/%sAttack_Montage.%sAttack_Montage"), *Name, *Name, *Name);

	UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, *MontagePath);

	if (Montage)
	{
		AttackMontage = Montage;
	}
	
	FString DeathMontagePath = FString::Printf(
	TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/Death_Montage.Death_Montage"), *Name);
	
	UAnimMontage* TempMontage = LoadObject<UAnimMontage>(nullptr, *DeathMontagePath);
	if (TempMontage)
	{	
		DeathMontage = TempMontage;
	}
	
	FString DanceMontagePath = FString::Printf(
	TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/Dance_Montage.Dance_Montage"), *Name);
	
	TempMontage = LoadObject<UAnimMontage>(nullptr, *DanceMontagePath);
	if (TempMontage)
	{
		DanceMontage = TempMontage;
	}
	
	FString SkillMontagePath = FString::Printf(
	TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/Skill_Montage.Skill_Montage"), *Name);
	
	TempMontage = LoadObject<UAnimMontage>(nullptr, *SkillMontagePath);
	if (TempMontage)
	{
		SkillMontage = TempMontage;
	}
}

void ATFT_UnitCharacter::InitWithChampionKey(ETFT_ChampionKey InChampionKey, int32 InStarLevel)
{
	ChampionKey = InChampionKey;
	starLevel = InStarLevel;
	
	UTFT_GameInstance* GI = GetWorld()->GetGameInstance<UTFT_GameInstance>();
	UDataTable* Table = GI->ChampionDataTable.LoadSynchronous();
	FName RowName = ConvertEnumToRowName(ChampionKey);

	const FTFT_ChampionData* Data = Table->FindRow<FTFT_ChampionData>(RowName, TEXT(""));

	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("Row not found"));
		return;
	}

	Initialize(*Data, starLevel);
	InitializeMesh();
	CombatComponent->OwnerCharacter = this;
	
	if (HPBarScreenWidget)
	{
		HPBarScreenWidget->SetOwnerCharacter(this);
		HPBarScreenWidget->BP_UpdateStarFrame(starLevel);
	}
}

FStruct_TFT_Champion ATFT_UnitCharacter::ConvertToChampionData(const FTFT_ChampionData& Data)
{
	FStruct_TFT_Champion Result;

	Result.Key = Data.Key;
	Result.Name = Data.Name;
	Result.Cost = Data.Cost;
	Result.Image = Data.Image;
	Result.Origins = FName(*Data.Origins);
	Result.Classes = FName(*Data.Classes);

	Result.Stats.AttackDamage = Data.AttackDamage;
	Result.Stats.AbilityPower = Data.AbilityPower;
	Result.Stats.AttackRange = Data.AttackRange;
	Result.Stats.AttackSpeed = Data.AttackSpeed;
	Result.Stats.CriticalChance = Data.CriticalChance;
	Result.Stats.DPS = Data.DPS;
	Result.Stats.Health = Data.Health;
	Result.Stats.Armor = Data.Armor;
	Result.Stats.MagicResist = Data.MagicResist;
	Result.Stats.StartingMana = Data.StartingMana;
	Result.Stats.MaxMana = Data.MaxMana;

	Result.Skill.Name = Data.SkillName;
	Result.Skill.Type = Data.SkillType;
	Result.Skill.Description = Data.SkillDescription;
	Result.Skill.Image = Data.SkillImage;
	Result.Skill.Effect = Data.SkillEffect;

	TArray<FString> Split;
	Data.SkillStats.ParseIntoArray(Split, TEXT("/"));
	for (const FString& Str : Split)
	{
		Result.Skill.Stats.Add(Str);
	}

	return Result;
}

FName ATFT_UnitCharacter::ConvertEnumToRowName(ETFT_ChampionKey Key)
{
	const UEnum* EnumPtr = StaticEnum<ETFT_ChampionKey>();
	
	if (!EnumPtr) return NAME_None;

	return FName(EnumPtr->GetNameStringByValue((int64)Key));
}

void ATFT_UnitCharacter::StopMontage(float BlendOutTime)
{
	if (!AttackMontage)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return;
	}

	if (AnimInstance->Montage_IsPlaying(AttackMontage))
	{
		AnimInstance->Montage_Stop(BlendOutTime, AttackMontage);
	}
	
	if (AnimInstance->Montage_IsPlaying(DanceMontage))
	{
		AnimInstance->Montage_Stop(BlendOutTime, DanceMontage);
	}
}

void ATFT_UnitCharacter::UpdateHPBarWidget()
{
	if (bHideHPBarPermanently)
	{
		return;
	}

	if (HPBarScreenWidget)
	{
		HPBarScreenWidget->UpdateHPBar();
	}
}

void ATFT_UnitCharacter::UpdateMPBarWidget()
{
	if (bHideHPBarPermanently)
	{
		return;
	}

	if (HPBarScreenWidget)
	{
		HPBarScreenWidget->UpdateMPBar();
	}
}

void ATFT_UnitCharacter::HPBarWidgetVisible(bool bIsVisible)
{
	if (HPBarScreenWidget)
	{
		HPBarScreenWidget->SetVisibility(bIsVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

FString ATFT_UnitCharacter::GetChampionNameString()
{
	const UEnum* EnumPtr = StaticEnum<ETFT_ChampionKey>();
	if (!EnumPtr) return TEXT("");

	return EnumPtr->GetNameStringByValue((int64)ChampionKey);
}

FString ATFT_UnitCharacter::BuildMeshPath(const FString& Name)
{
	return FString::Printf(
		TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/%s.%s"),
		*Name, *Name, *Name
	);
}

void ATFT_UnitCharacter::PlayAttackMontageByInterval(float AttackRate)
{
	if (!AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no AttackMontage assigned."), *GetChampionNameString());
		return;
	}

	if (AttackRate <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has invalid AttackRate: %f"), *GetChampionNameString(), AttackRate);
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no AnimInstance."), *GetChampionNameString());
		return;
	}

	const float MontageLength = AttackMontage->GetPlayLength();
	if (MontageLength <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s AttackMontage length is invalid."), *GetChampionNameString());
		return;
	}
	
	const float PlayRate = MontageLength * AttackRate;
	AnimInstance->Montage_Play(AttackMontage, PlayRate);
}

void ATFT_UnitCharacter::PlayDeathMontage()
{
	bHideHPBarPermanently = true;

	if (HPBarScreenWidget)
	{
		HPBarScreenWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no AnimInstance."), *GetChampionNameString());
		return;
	}
	
	UGameplayStatics::PlaySound2D(this, DeathSound);
	
	AnimInstance->Montage_Play(DeathMontage);
}

void ATFT_UnitCharacter::PlayDanceMontage()
{
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no AnimInstance."), *GetChampionNameString());
		return;
	}
	
	AnimInstance->Montage_Play(DanceMontage, 1.f, EMontagePlayReturnType::MontageLength, 0.f, true);
}

void ATFT_UnitCharacter::PlaySkillMontage()
{
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no AnimInstance."), *GetChampionNameString());
		return;
	}
	
	UGameplayStatics::PlaySound2D(this, SkillSound);
	
	AnimInstance->Montage_Play(SkillMontage);
}

void ATFT_UnitCharacter::InitializeItemSlots()
{
	EquippedItemSlots.SetNum(3);

	for (FStruct_TFTEquippedItemSlot& Slot : EquippedItemSlots)
	{
		Slot.bOccupied = false;
	}
}

int32 ATFT_UnitCharacter::FindFirstEmptyItemSlot() const
{
	for (int32 i = 0; i < EquippedItemSlots.Num(); ++i)
	{
		if (!EquippedItemSlots[i].bOccupied)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool ATFT_UnitCharacter::HasEmptyItemSlot() const
{
	return FindFirstEmptyItemSlot() != INDEX_NONE;
}

bool ATFT_UnitCharacter::TryEquipItem(const FStruct_TFTItemInstance& ItemInstance)
{
	const int32 EmptyIndex = FindFirstEmptyItemSlot();
	if (EmptyIndex == INDEX_NONE)
	{
		return false;
	}
	
	
	if (!bIsEnemy) // 적군이 아닐때만 사운드 재생
	{
		UGameplayStatics::PlaySound2D(this, ItemEquipSound);
	}


	UTFT_GameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UTFT_GameInstance>() : nullptr;
	if (!GI)
	{
		return false;
	}

	UDataTable* ItemTable = GI->ItemDataTable.LoadSynchronous();
	if (!ItemTable)
	{
		return false;
	}

	const FStruct_TFTItemDefinition* ItemDef = ItemTable->FindRow<FStruct_TFTItemDefinition>(ItemInstance.ItemId, TEXT("TryEquipItem"));
	if (!ItemDef)
	{
		return false;
	}

	EquippedItemSlots[EmptyIndex].bOccupied = true;
	EquippedItemSlots[EmptyIndex].ItemInstance = ItemInstance;

	if (StatComponent)
	{
		const TArray<FBaseModifier> ItemModifiers = BuildItemModifiers(*ItemDef, this);
		StatComponent->AddModifiers(ItemModifiers);
	}

	RefreshItemSlotWidget();
	UpdateHPBarWidget();
	UpdateMPBarWidget();
	return true;
}

void ATFT_UnitCharacter::RefreshItemSlotWidget()
{
	if (HPBarScreenWidget)
	{
		HPBarScreenWidget->RefreshItemSlots();
	}
}

UTexture2D* ATFT_UnitCharacter::GetItemIconByItemId(FName ItemId) const
{
	if (ItemId.IsNone())
	{
		return nullptr;
	}

	UTFT_GameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UTFT_GameInstance>() : nullptr;
	if (!GI)
	{
		return nullptr;
	}

	UDataTable* ItemTable = GI->ItemDataTable.LoadSynchronous();
	if (!ItemTable)
	{
		return nullptr;
	}

	const FStruct_TFTItemDefinition* ItemDef = ItemTable->FindRow<FStruct_TFTItemDefinition>(ItemId, TEXT("GetItemIconByItemId"));
	if (!ItemDef)
	{
		return nullptr;
	}

	return ItemDef->Icon;
}

void ATFT_UnitCharacter::ItemTest()
{
	FStruct_TFTItemInstance TestItem;
	TestItem.ItemId = TEXT("LordsEdge");
	TryEquipItem(TestItem);
}

TArray<FBaseModifier> ATFT_UnitCharacter::BuildItemModifiers(const FStruct_TFTItemDefinition& ItemDef, UObject* Source)
{
	TArray<FBaseModifier> Result;

	if (ItemDef.StatBonus.AttackDamage != 0)
	{
		Result.Add(FBaseModifier(ETFTModifiedStat::AttackDamage, EModifierOperation::Add, (float)ItemDef.StatBonus.AttackDamage, Source));
	}

	if (ItemDef.StatBonus.AbilityPower != 0)
	{
		Result.Add(FBaseModifier(ETFTModifiedStat::AbilityPower, EModifierOperation::Add, (float)ItemDef.StatBonus.AbilityPower, Source));
	}

	if (ItemDef.StatBonus.AttackSpeed != 0.0f)
	{
		Result.Add(FBaseModifier(ETFTModifiedStat::AttackSpeed, EModifierOperation::Add, ItemDef.StatBonus.AttackSpeed, Source));
	}

	if (ItemDef.StatBonus.CriticalChance != 0.0f)
	{
		Result.Add(FBaseModifier(ETFTModifiedStat::CriticalChance, EModifierOperation::Add, ItemDef.StatBonus.CriticalChance, Source));
	}

	if (ItemDef.StatBonus.Health != 0)
	{
		Result.Add(FBaseModifier(ETFTModifiedStat::Health, EModifierOperation::Add, (float)ItemDef.StatBonus.Health, Source));
	}

	if (ItemDef.StatBonus.Armor != 0)
	{
		Result.Add(FBaseModifier(ETFTModifiedStat::Armor, EModifierOperation::Add, (float)ItemDef.StatBonus.Armor, Source));
	}

	if (ItemDef.StatBonus.MagicResist != 0)
	{
		Result.Add(FBaseModifier(ETFTModifiedStat::MagicResist, EModifierOperation::Add, (float)ItemDef.StatBonus.MagicResist, Source));
	}

	return Result;
}