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
};
