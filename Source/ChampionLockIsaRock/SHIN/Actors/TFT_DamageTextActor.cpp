#include "TFT_DamageTextActor.h"
#include "Components/WidgetComponent.h"
#include "SHIN/Character/UI/TFT_DamageTextWidget.h"

ATFT_DamageTextActor::ATFT_DamageTextActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(Root);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawAtDesiredSize(true);
	
	static ConstructorHelpers::FClassFinder<UUserWidget> DamageWidgetClass(
		TEXT("/Game/SHIN/UI/Blueprints/WBP_DamageText.WBP_DamageText_C")
	);

	if (DamageWidgetClass.Succeeded())
	{
		WidgetComponent->SetWidgetClass(DamageWidgetClass.Class);
	}
}

void ATFT_DamageTextActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATFT_DamageTextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;
	AddActorWorldOffset(FVector(0.f, 0.f, FloatSpeed * DeltaTime));

	if (ElapsedTime >= LifeTime)
	{
		Destroy();
	}
}

void ATFT_DamageTextActor::InitializeDamageText(int32 Damage, bool bCritical)
{
	if (UTFT_DamageTextWidget* DamageWidget = Cast<UTFT_DamageTextWidget>(WidgetComponent->GetUserWidgetObject()))
	{
		DamageWidget->SetDamageText(Damage, bCritical);
	}
}