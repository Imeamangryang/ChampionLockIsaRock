// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFT_SkillComponent.generated.h"

struct FStruct_TFT_Champion;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHAMPIONLOCKISAROCK_API UTFT_SkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTFT_SkillComponent();
	
	void Initialize(const FStruct_TFT_Champion& Data);

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(BlueprintReadOnly)
	FText SkillName;
	
	UPROPERTY(BlueprintReadOnly)
	FText Type;

	UPROPERTY(BlueprintReadOnly)
	FText Description;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> SkillValues;
	
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SkillImage;
};
