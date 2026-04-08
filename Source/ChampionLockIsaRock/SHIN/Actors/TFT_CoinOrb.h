#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFT_CoinOrb.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ATFT_Coin;

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_CoinOrb : public AActor
{
	GENERATED_BODY()

public:
	ATFT_CoinOrb();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPickupSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* OrbMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|CoinOrb")
	bool bCollected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|CoinOrb")
	int32 MinCoinCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|CoinOrb")
	int32 MaxCoinCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|CoinOrb")
	TSubclassOf<ATFT_Coin> CoinClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|CoinOrb|SpawnArc")
	float RandomOffsetRadius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|CoinOrb|SpawnArc")
	float ArcHeight = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|CoinOrb|SpawnArc")
	float MoveDuration = 1.5f;

protected:
	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY()
	float ElapsedMoveTime = 0.f;

	UPROPERTY()
	bool bPlaySpawnArc = false;
};