#pragma once

#include "CoreMinimal.h"
#include "ETFT_ChampionList.generated.h"

UENUM(BlueprintType)
enum class ETFT_ChampionKey : uint8
{
	Garen	UMETA(DisplayName = "Garen"),
	Graves	UMETA(DisplayName = "Graves"),
	Nidalee UMETA(DisplayName = "Nidalee"),
	Darius 	UMETA(DisplayName = "Darius"),
	Mordekaiser UMETA(DisplayName = "Mordekaiser"),
	Vayne 	UMETA(DisplayName = "Vayne"),
	Elise 	UMETA(DisplayName = "Elise"),
	Warwick UMETA(DisplayName = "Warwick"),
	Camille	UMETA(DisplayName = "Camille"),
	Kassadin UMETA(DisplayName = "Kassadin"),
	KhaZix UMETA(DisplayName = "KhaZix"),
	Tristana UMETA(DisplayName = "Tristana"),
	Fiora 	UMETA(DisplayName = "Fiora"),
};
