// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TFT_SynergyData.h"
#include "TFT_SynergyComponent.generated.h"

// TMap을 바로 던지지 말고 구조체에 담아서 던집니다.
USTRUCT(BlueprintType)
struct FSynergyCountMap
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, int32> Counts;
};

// UI에게 TMap(시너지 이름, 개수)을 던져줄 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSynergyUpdated, const FSynergyCountMap&, SynergyData);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHAMPIONLOCKISAROCK_API UTFT_SynergyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTFT_SynergyComponent();
	
	// 필드 인원이 바뀔 때마다 컨트롤러가 찔러줄 함수
	UFUNCTION(BlueprintCallable, Category = "TFT|Synergy")
	void RecalculateSynergies();

	// UI 블루프린트에서 Event Bind를 걸 수 있는 방송국 변수
	UPROPERTY(BlueprintAssignable, Category = "TFT|Synergy")
	FOnSynergyUpdated OnSynergyUpdated;

protected:
	virtual void BeginPlay() override;
	
	// 현재 발동 중인 시너지와 그 인원수를 기록하는 장부
	// FString(문자열)을 키로 써서 팀원의 데이터와 쉽게 매칭
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT|Synergy")
	FSynergyCountMap ActiveSynergyData;

	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
