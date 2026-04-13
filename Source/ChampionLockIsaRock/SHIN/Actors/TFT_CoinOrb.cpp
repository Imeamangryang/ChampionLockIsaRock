#include "TFT_CoinOrb.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "TFT_Coin.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"

ATFT_CoinOrb::ATFT_CoinOrb()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(Root);
	PickupSphere->InitSphereRadius(50.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	OrbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrbMesh"));
	OrbMesh->SetupAttachment(Root);
	OrbMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OrbMesh->SetRelativeScale3D(FVector(0.8f));
	OrbMesh->SetRelativeRotation(FRotator(0.f, -90.f, -50.f));
	
	OrbEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("OrbEffect"));
	OrbEffect->SetupAttachment(Root);
	OrbEffect->bAutoActivate = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> OrbMeshAsset(TEXT("/Game/SHIN/Blueprints/Items/Sphere.Sphere"));
	if (OrbMeshAsset.Succeeded())
	{
		OrbMesh->SetStaticMesh(OrbMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OrbMaterialAsset(TEXT("/Game/SHIN/Blueprints/Items/MT_CoinOrb_Inst.MT_CoinOrb_Inst"));
	if (OrbMaterialAsset.Succeeded())
	{
		OrbMesh->SetMaterial(0, OrbMaterialAsset.Object);
	}

	CoinClass = ATFT_Coin::StaticClass();
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> OrbEffectAsset(TEXT("/Game/VFX/Imortal_Loot_Drop_VFX/Niagara/NS_Loot_Drop_3.NS_Loot_Drop_3"));
	if (OrbEffectAsset.Succeeded())
	{	
		OrbEffect->SetAsset(OrbEffectAsset.Object);
	}
	
	ConstructorHelpers::FObjectFinder<USoundBase> OrbSoundAsset(TEXT("/Game/SHIN/Sound/Addbooty_out.Addbooty_out"));
	if (OrbSoundAsset.Succeeded())
	{
		OrbSound = OrbSoundAsset.Object;
	}
}

void ATFT_CoinOrb::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();

	const FVector2D Random2D = FMath::RandPointInCircle(RandomOffsetRadius);
	TargetLocation = StartLocation + FVector(Random2D.X, Random2D.Y, 0.f);

	ElapsedMoveTime = 0.f;
	bPlaySpawnArc = true;

	if (PickupSphere)
	{
		PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ATFT_CoinOrb::OnPickupSphereBeginOverlap);
	}
}

void ATFT_CoinOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPlaySpawnArc)
	{
		ElapsedMoveTime += DeltaTime;

		const float Alpha = FMath::Clamp(ElapsedMoveTime / MoveDuration, 0.f, 1.f);
		const FVector BaseLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);
		const float ArcOffsetZ = 4.f * ArcHeight * Alpha * (1.f - Alpha);

		const FVector NewLocation = BaseLocation + FVector(0.f, 0.f, ArcOffsetZ);
		SetActorLocation(NewLocation);

		if (Alpha >= 1.f)
		{
			SetActorLocation(TargetLocation);
			bPlaySpawnArc = false;
		}
	}
}

void ATFT_CoinOrb::OnPickupSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bCollected || bPlaySpawnArc)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PlayerController))
	{
		return;
	}

	APawn* ControlledPawn = PlayerController->GetPawn();
	if (!IsValid(ControlledPawn) || OtherActor != ControlledPawn)
	{
		return;
	}

	if (!CoinClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CoinOrb: CoinClass is not set."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 CoinCount = FMath::RandRange(MinCoinCount, MaxCoinCount);
	const FVector SpawnLocation = GetActorLocation();
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	for (int32 i = 0; i < CoinCount; ++i)
	{
		World->SpawnActor<ATFT_Coin>(CoinClass, SpawnLocation, SpawnRotation);
	}

	if (OrbSound)
	{
		UGameplayStatics::PlaySound2D(this, OrbSound);
	}

	UE_LOG(LogTemp, Log, TEXT("CoinOrb spawned %d coins"), CoinCount);

	bCollected = true;
	Destroy();
}