// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCHPBarWidget.cpp

#include "UI/SCHPBarWidget.h"
#include "StrandingChain.h"

void USCHPBarWidget::UpdateHP(int32 CurrentHP, int32 MaxHP)
{
	if (MaxHP <= 0)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCHPBarWidget] UpdateHP: MaxHP=%d가 0 이하. 갱신 무시."), MaxHP);
		return;
	}
	HPPercent = FMath::Clamp(
		static_cast<float>(CurrentHP) / static_cast<float>(MaxHP),
		0.f, 1.f);

	OnHPChanged(HPPercent);
}
