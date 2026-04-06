#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "TFT_SynergyData.generated.h"

// 1. 시너지 종류
UENUM(BlueprintType)
enum class ETFT_TraitKey : uint8
{
	None UMETA(DisplayName = "없음"),
	
	// Origins
	Void UMETA(DisplayName = "공허"),
	Noble UMETA(DisplayName = "귀족"),
	Ninja UMETA(DisplayName = "닌자"),
	Robot UMETA(DisplayName = "로봇"),
	Hextech UMETA(DisplayName = "마법공학"),
	Glacial UMETA(DisplayName = "빙하"),
	Demon UMETA(DisplayName = "악마"),
	Wild UMETA(DisplayName = "야생"),
	Yordle UMETA(DisplayName = "요들"),
	Dragon UMETA(DisplayName = "용"),
	Phantom UMETA(DisplayName = "유령"),
	Imperial UMETA(DisplayName = "제국"),
	Exile UMETA(DisplayName = "추방자"),
	Pirate UMETA(DisplayName = "해적"),
	
	// Classes
	Blademaster UMETA(DisplayName = "검사"),
	Knight UMETA(DisplayName = "기사"),
	Sorcerer UMETA(DisplayName = "마법사"),
	Guardian UMETA(DisplayName = "수호자"),
	Brawler UMETA(DisplayName = "싸움꾼"),
	Assassin UMETA(DisplayName = "암살자"),
	Elementalist UMETA(DisplayName = "원소술사"),
	Ranger UMETA(DisplayName = "정찰대"),
	Gunslinger UMETA(DisplayName = "총잡이"),
	Shapeshifter UMETA(DisplayName = "형상변환자")
};

// 2. 시너지의 "단계별 효과" 구조체 (예: 2명일 때, 4명일 때)
USTRUCT(BlueprintType)
struct FTFT_SynergyTier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	int32 RequiredCount; // 시너지 발동에 필요한 인원수 (예: 2, 4, 6)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	float BuffValue;     // 부여할 스탯 수치 (예: 방어력 15, 공격력 30)
};

// 3. 데이터 테이블의 될 최종 구조체
// 데이터 테이블용 구조체는 FTableRowBase를 상속
USTRUCT(BlueprintType)
struct FTFT_SynergyData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	ETFT_TraitKey TraitKey; // 이 시너지의 진짜 이름 (Enum)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	TArray<FTFT_SynergyTier> SynergyTiers; // 단계별 효과가 담길 배열
	
	// UI에 표시될 시너지 아이콘 (Texture2D 형식)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	class UTexture2D* SynergyIcon; 

	// UI에 표시될 달성 조건 텍스트 (예: "2 > 4 > 6")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FString ThresholdText;
};

