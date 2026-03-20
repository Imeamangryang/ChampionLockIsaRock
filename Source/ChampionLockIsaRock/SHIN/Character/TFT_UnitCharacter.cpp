#include "TFT_UnitCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TFT_StatComponent.h"
#include "TFT_SkillComponent.h"
#include "TFT_CombatComponent.h"
#include "../Struct/FTFT_ChampionData.h"
#include "../GameFramework/TFT_GameInstance.h"
#include "Components/CapsuleComponent.h"

ATFT_UnitCharacter::ATFT_UnitCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	// 
	
	StatComponent = CreateDefaultSubobject<UTFT_StatComponent>(TEXT("StatComponent"));
	SkillComponent = CreateDefaultSubobject<UTFT_SkillComponent>(TEXT("SkillComponent"));
	CombatComponent = CreateDefaultSubobject<UTFT_CombatComponent>(TEXT("CombatComponent"));
}

void ATFT_UnitCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

	Initialize(*Data, 1);
	
	// 메시 & 애니메이션 블루프린트 설정
	InitializeMesh();
	
	// combat component에 owner 캐릭터 설정
	CombatComponent->OwnerCharacter = this;
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
	FString AnimPath = FString::Printf(TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/ABP_%s.ABP_%s_C"), *Name, *Name, *Name);

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
	FString AnimPath = FString::Printf(TEXT("/Game/SHIN/Data/Models/%s/SkeletalMeshes/ABP_%s.ABP_%s_C"), *Name, *Name, *Name);

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
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load Attack Montage: %s"), *MontagePath);
	}
}

FStruct_TFT_Champion ATFT_UnitCharacter::ConvertToChampionData(const FTFT_ChampionData& Data)
{
	FStruct_TFT_Champion Result;

	Result.Key = Data.Key;
	Result.Name = Data.Name;
	Result.Cost = Data.Cost;
	Result.Image = Data.Image;

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
