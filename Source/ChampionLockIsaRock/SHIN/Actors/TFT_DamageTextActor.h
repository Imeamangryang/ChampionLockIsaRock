#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFT_DamageTextActor.generated.h"

class UWidgetComponent;
class UTFT_DamageTextWidget;

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_DamageTextActor : public AActor
{
	GENERATED_BODY()
	
public:
	ATFT_DamageTextActor();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="TFT|DamageText")
	void InitializeDamageText(int32 Damage, bool bCritical);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* WidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TFT|DamageText")
	float LifeTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TFT|DamageText")
	float FloatSpeed = 50.0f;

	float ElapsedTime = 0.0f;
};