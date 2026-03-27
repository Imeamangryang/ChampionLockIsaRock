#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TFT_GameState.generated.h"

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_GameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;	
};
