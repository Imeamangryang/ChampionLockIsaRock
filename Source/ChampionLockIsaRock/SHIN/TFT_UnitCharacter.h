#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TFT_UnitCharacter.generated.h"

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_UnitCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATFT_UnitCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
