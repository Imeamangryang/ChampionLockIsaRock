#include "TFT_SkillComponent.h"
#include "../Struct/FStruct_TFT_Champion.h"

UTFT_SkillComponent::UTFT_SkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTFT_SkillComponent::Initialize(const FStruct_TFT_Champion& Data)
{
	SkillName = Data.Skill.Name;
	Description = Data.Skill.Description;
}

void UTFT_SkillComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTFT_SkillComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

