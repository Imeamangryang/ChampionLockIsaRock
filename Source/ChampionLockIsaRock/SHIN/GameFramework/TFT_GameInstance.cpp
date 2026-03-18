#include "SHIN/GameFramework/TFT_GameInstance.h"

void UTFT_GameInstance::Init()
{
	Super::Init();
	
	// DataTable 로드
	ChampionDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/SHIN/Data/DataTable/DT_ChampionData.DT_ChampionData")));
	
	if (ChampionDataTable.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("ChampionDataTable Path is NULL"));
		return;
	}

	UDataTable* LoadedTable = ChampionDataTable.LoadSynchronous();

	if (!LoadedTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Champion DataTable"));
		return;
	}
}
