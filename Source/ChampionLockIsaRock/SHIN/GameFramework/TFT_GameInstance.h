#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TFT_GameInstance.generated.h"

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UDataTable> ChampionDataTable;
};
