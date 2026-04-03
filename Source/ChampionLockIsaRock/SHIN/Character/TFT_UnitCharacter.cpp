#include "TFT_UnitCharacter.h"
#include "Components/TFT_StatComponent.h"
#include "Components/TFT_SkillComponent.h"
#include "Components/TFT_CombatComponent.h"
#include "../Struct/FTFT_ChampionData.h"
#include "../GameFramework/TFT_GameInstance.h"
#include "Components/WidgetComponent.h"
#include "UI/TFT_HPBarWidget.h"
#include "Components/CapsuleComponent.h"
#include "SHIN/Character/Components/TFT_ItemInventoryComponent.h"

ATFT_UnitCharacter::ATFT_UnitCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	StatComponent = CreateDefaultSubobject<UTFT_StatComponent>(TEXT("StatComponent"));
	SkillComponent = CreateDefaultSubobject<UTFT_SkillComponent>(TEXT("SkillComponent"));
	CombatComponent = CreateDefaultSubobject<UTFT_CombatComponent>(TEXT("CombatComponent"));
	
	HPBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidgetComponent"));
	HPBarWidgetComponent->SetupAttachment(RootComponent);
	HPBarWidgetComponent->SetRelativeLocation (FVector(0.f, 0.f, 150.f));
	HPBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	
	static ConstructorHelpers::FClassFinder<UUserWidget> HPBarWidgetClass(TEXT("/Game/SHIN/UI/Blueprints/WBP_HealthBar.WBP_HealthBar_C")); 
	if (HPBarWidgetClass.Succeeded())
	{
		HPBarWidgetComponent->SetWidgetClass(HPBarWidgetClass.Class);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load HP Bar Widget class."));
	}
	
	// Pawn끼리는 콜리전 무시
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void ATFT_UnitCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (bIsEnemy)
	{
		// 적일 경우에도 피킹이 되지 않도록 무시
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	}
	InitializeItemSlots();
	
	InitWithChampionKey(ChampionKey, starLevel);
}

void ATFT_UnitCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	// Character Type을 바꾸면 메시 설정
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
	
	// 메시 설정 후 애니메이션 블루프린트도 설정
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

void ATFT_UnitCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
}

void ATFT_UnitCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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
	// Champion Type을 바꾸면 메시 설정
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
	
	// 메시 설정 후 애니메이션 블루프린트도 설정
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
	
	// DeathMontage
	FString DeathMontagePath = FString::Printf(
	TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/Death_Montage.Death_Montage"), *Name);
	
	UAnimMontage* TempMontage = LoadObject<UAnimMontage>(nullptr, *DeathMontagePath);
	if (TempMontage)
	{	
		DeathMontage = TempMontage;
	}
	
	// DanceMontage
	FString DanceMontagePath = FString::Printf(
	TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/Dance_Montage.Dance_Montage"), *Name);
	
	TempMontage = LoadObject<UAnimMontage>(nullptr, *DanceMontagePath);
	if (TempMontage)
	{
		DanceMontage = TempMontage;
	}
}

void ATFT_UnitCharacter::InitWithChampionKey(ETFT_ChampionKey InChampionKey, int32 InStarLevel)
{
	ChampionKey = InChampionKey;
	starLevel = InStarLevel;
	
	// Data Table에서 Champion Data 가져오기
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
	
	// 메시 & 애니메이션 블루프린트 설정
	InitializeMesh();
	
	// combat component에 owner 캐릭터 설정
	CombatComponent->OwnerCharacter = this;
	
	// HP Bar Widget 연결
	if (HPBarWidgetComponent)
	{
		if (UTFT_HPBarWidget* HPWidget = Cast<UTFT_HPBarWidget>(HPBarWidgetComponent->GetUserWidgetObject()))
		{
			HPWidget->SetOwnerCharacter(this);
			HPWidget->BP_UpdateStarFrame(starLevel);
		}
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

	// Stats
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

	// Skill
	Result.Skill.Name = Data.SkillName;
	Result.Skill.Type = Data.SkillType;
	Result.Skill.Description = Data.SkillDescription;
	Result.Skill.Image = Data.SkillImage;

	// SkillStats 파싱
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
	if (UTFT_HPBarWidget* HPWidget = Cast<UTFT_HPBarWidget>(HPBarWidgetComponent->GetUserWidgetObject()))
	{
		HPWidget->UpdateHPBar();
	}
}

void ATFT_UnitCharacter::UpdateMPBarWidget()
{
	if (UTFT_HPBarWidget* HPWidget = Cast<UTFT_HPBarWidget>(HPBarWidgetComponent->GetUserWidgetObject()))
	{
		HPWidget->UpdateMPBar();
	}
}

void ATFT_UnitCharacter::HPBarWidgetVisible(bool bIsVisible)
{
	if (UTFT_HPBarWidget* HPWidget = Cast<UTFT_HPBarWidget>(HPBarWidgetComponent->GetUserWidgetObject()))
	{
		HPWidget->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
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

	// AttackRate = 초당 공격 횟수
	// 공격 1회의 주기 = 1 / AttackRate
	// 그 주기에 맞게 몽타주 재생속도 계산
	const float PlayRate = MontageLength * AttackRate;

	AnimInstance->Montage_Play(AttackMontage, PlayRate);
}

void ATFT_UnitCharacter::PlayDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no AnimInstance."), *GetChampionNameString());
		return;
	}
	
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
	
	// 몽타주 수동으로 멈출때까지 반복 재생하도록 설정
	AnimInstance->Montage_Play(DanceMontage, 1.f, EMontagePlayReturnType::MontageLength, 0.f, true);
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

	EquippedItemSlots[EmptyIndex].bOccupied = true;
	EquippedItemSlots[EmptyIndex].ItemInstance = ItemInstance;

	RefreshItemSlotWidget();
	return true;
}

void ATFT_UnitCharacter::RefreshItemSlotWidget()
{
	if (UTFT_HPBarWidget* HPWidget = Cast<UTFT_HPBarWidget>(HPBarWidgetComponent->GetUserWidgetObject()))
	{
		HPWidget->RefreshItemSlots();
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
	// Item Test
	FStruct_TFTItemInstance TestItem;
	TestItem.ItemId = TEXT("LordsEdge");
	TryEquipItem(TestItem);
}