#pragma once

#include "CoreMinimal.h"
#include "FStruct_TFT_Skills.h"
#include "FStruct_TFT_Stats.h"
#include "FStruct_TFT_Champion.generated.h"

USTRUCT(BlueprintType)
struct FStruct_TFT_Champion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Cost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> Origins;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> Classes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FStruct_TFT_Stats Stats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FStruct_TFT_Skills Skill;
};