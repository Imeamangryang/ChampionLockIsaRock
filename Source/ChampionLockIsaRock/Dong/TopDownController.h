// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "../SHIN/Character/TFT_UnitCharacter.h"
#include "TopDownController.generated.h"


class UInputMappingContext;
class UInputAction;

/**
 * 
 */
UCLASS()
class CHAMPIONLOCKISAROCK_API ATopDownController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ATopDownController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* IA_Grabbed;
	
	// 1. 지금 집어든 기물이 있는가?
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Interaction")
	bool bIsHoldingUnit = false;
	
	// 2. 어떤 기물을 들고 있는가?
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Interaction")
	ATFT_UnitCharacter* GrabbedUnit = nullptr;
	
	// 3. 원래 있던 위치 (스왑용)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Interaction")
	FVector OriginalLocation;
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	void OnGrabPressed(const FInputActionValue& Value);
	void OnDropReleased(const FInputActionValue& Value);
	
};
