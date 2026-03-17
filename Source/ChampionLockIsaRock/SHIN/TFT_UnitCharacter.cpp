#include "TFT_UnitCharacter.h"

ATFT_UnitCharacter::ATFT_UnitCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATFT_UnitCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATFT_UnitCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATFT_UnitCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

