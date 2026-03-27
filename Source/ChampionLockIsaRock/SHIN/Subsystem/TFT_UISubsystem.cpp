#include "TFT_UISubsystem.h"
#include "SHIN/Struct/FStruct_TFT_Champion.h"

void UTFT_UISubsystem::BroadcastStatUIOpen(bool bIsOpen, FStruct_TFT_Champion championdata, ATFT_UnitCharacter* Unit)
{
	OnStatUIOpen.Broadcast(bIsOpen, championdata, Unit);
}

void UTFT_UISubsystem::BroadcastHPUpdate(ATFT_UnitCharacter* Unit, float MaxHP, float CurrentHP)
{
	OnHPUpdated.Broadcast(Unit, MaxHP, CurrentHP);
}

void UTFT_UISubsystem::BroadcastMPUpdate(ATFT_UnitCharacter* Unit, float MaxMP, float CurrentMP)
{
	OnMPUpdated.Broadcast(Unit, MaxMP, CurrentMP);
}
