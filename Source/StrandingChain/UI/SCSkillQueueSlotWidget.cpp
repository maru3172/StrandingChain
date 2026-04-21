// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillQueueSlotWidget.cpp

#include "UI/SCSkillQueueSlotWidget.h"
#include "UI/SCSkillCardWidget.h"
#include "UI/SCSkillQueueWidget.h"
#include "StrandingChain.h"

void USCSkillQueueSlotWidget::PlaceCard(USCSkillCardWidget* Card)
{
	if (!IsValid(Card)) { return; }
	SlottedCard = Card;
	OnSlotFilled(Card);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillQueueSlot] 슬롯[%d] 카드 등록."), SlotIndex);
}

USCSkillCardWidget* USCSkillQueueSlotWidget::TakeCard()
{
	USCSkillCardWidget* Card = SlottedCard.Get();
	SlottedCard = nullptr;
	OnSlotCleared();
	return Card;
}

FReply USCSkillQueueSlotWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭 → 슬롯 해제
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !IsEmpty())
	{
		USCSkillQueueWidget* Queue = OwnerQueue.Get();
		if (IsValid(Queue))
		{
			Queue->ReturnCardFromSlot(this);
			UE_LOG(LogStrandingChain, Log,
				TEXT("[SCSkillQueueSlot] 슬롯[%d] 좌클릭으로 카드 해제."), SlotIndex);
		}
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
