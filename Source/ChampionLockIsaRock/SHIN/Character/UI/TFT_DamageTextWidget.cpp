#include "TFT_DamageTextWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UTFT_DamageTextWidget::SetDamageText(int32 Damage, bool bCritical)
{
	if (DamageText)
	{
		DamageText->SetText(FText::AsNumber(Damage));

		if (bCritical)
		{
			DamageText->SetColorAndOpacity(FSlateColor(FLinearColor(0.443f, 0.15f, 0.162f, 1.0f)));
		}
		else
		{
			// 특정 수치 값 컬러 
			DamageText->SetColorAndOpacity(FSlateColor(FLinearColor(0.443f, 0.15f, 0.162f, 1.0f)));
		}
	}

	if (CriticalIcon)
	{
		CriticalIcon->SetVisibility(bCritical ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}