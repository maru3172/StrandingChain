// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillQueueWidget.cpp

#include "UI/SCSkillQueueWidget.h"
#include "UI/SCSkillCardWidget.h"
#include "UI/SCSkillQueueSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "StrandingChain.h"

void USCSkillQueueWidget::InitSlots()
{
	ClearSlots();
	if (!IsValid(QueueSlotClass) || !IsValid(SlotContainer)) { return; }

	for (int32 i = 0; i < 3; ++i)
	{
		USCSkillQueueSlotWidget* QueueSlot =
			CreateWidget<USCSkillQueueSlotWidget>(this, QueueSlotClass);
		if (!IsValid(QueueSlot)) { continue; }

		QueueSlot->SlotIndex  = i;
		QueueSlot->OwnerQueue = this;
		Slots.Add(QueueSlot);

		UHorizontalBoxSlot* BoxSlot = SlotContainer->AddChildToHorizontalBox(QueueSlot);
		if (IsValid(BoxSlot))
		{
			BoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			BoxSlot->SetPadding(FMargin(6.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
			BoxSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
	UE_LOG(LogStrandingChain, Log, TEXT("[SCSkillQueueWidget] 슬롯 3개 초기화."));
}

void USCSkillQueueWidget::ClearSlots()
{
	if (IsValid(SlotContainer)) { SlotContainer->ClearChildren(); }
	Slots.Empty();
}

void USCSkillQueueWidget::RegisterCardToSlot(
	USCSkillCardWidget* Card, USCSkillQueueSlotWidget* InSlot)
{
	if (!IsValid(Card) || !IsValid(InSlot)) { return; }
	InSlot->PlaceCard(Card);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillQueueWidget] 카드 → 슬롯[%d]"), InSlot->SlotIndex);
}

void USCSkillQueueWidget::ReturnCardFromSlot(USCSkillQueueSlotWidget* InSlot)
{
	if (!IsValid(InSlot)) { return; }
	InSlot->TakeCard();
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillQueueWidget] 슬롯[%d] 해제."), InSlot->SlotIndex);
}

TArray<int32> USCSkillQueueWidget::GetQueuedDrawnIndices() const
{
	TArray<int32> Result;
	for (const TObjectPtr<USCSkillQueueSlotWidget>& SlotEntry : Slots)
	{
		if (!IsValid(SlotEntry) || SlotEntry->IsEmpty()) { continue; }
		const USCSkillCardWidget* Card = SlotEntry->SlottedCard.Get();
		if (IsValid(Card))
		{
			Result.Add(Card->DrawnIndex);
		}
	}
	return Result;
}

USCSkillQueueSlotWidget* USCSkillQueueWidget::FindFirstEmptySlot() const
{
	for (const TObjectPtr<USCSkillQueueSlotWidget>& SlotEntry : Slots)
	{
		if (IsValid(SlotEntry) && SlotEntry->IsEmpty())
		{
			return SlotEntry.Get();
		}
	}
	return nullptr;
}
