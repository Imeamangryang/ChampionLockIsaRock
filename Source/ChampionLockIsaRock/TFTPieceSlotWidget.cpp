#include "TFTPieceSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UTFTPieceSlotWidget::SetChampionData(const FTFT_ChampionData& Data)
{
    if (TextBlock_ChampionName)
        TextBlock_ChampionName->SetText(Data.Name);

    if (TextBlock_Origins)
        TextBlock_Origins->SetText(FText::FromString(Data.Origins));

    if (TextBlock_Classes)
        TextBlock_Classes->SetText(FText::FromString(Data.Classes));

    if (TextBlock_Cost)
        TextBlock_Cost->SetText(FText::FromString(FString::Printf(TEXT("%d"), Data.Cost)));

    if (Image_0 && !Data.Image.IsNull())
    {
        UTexture2D* Tex = Data.Image.LoadSynchronous();
        if (Tex)
        {
            Image_0->SetBrushFromTexture(Tex);
        }
    }
}

void UTFTPieceSlotWidget::ClearSlot()
{
    if (TextBlock_ChampionName)
        TextBlock_ChampionName->SetText(FText::FromString(TEXT("")));

    if (TextBlock_Origins)
        TextBlock_Origins->SetText(FText::FromString(TEXT("")));

    if (TextBlock_Classes)
        TextBlock_Classes->SetText(FText::FromString(TEXT("")));

    if (TextBlock_Cost)
        TextBlock_Cost->SetText(FText::FromString(TEXT("")));
}
