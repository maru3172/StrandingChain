// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillQueueSlotWidget.cpp

#include "UI/SCSkillQueueSlotWidget.h"
#include "UI/SCSkillCardWidget.h"
#include "UI/SCSkillQueueWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "StrandingChain.h"

void USCSkillQueueSlotWidget::PlaceCard(USCSkillCardWidget* Card)
{
	if (!IsValid(Card)) { return; }
	SlottedCard = Card;
	OnSlotFilled(Card);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillQueueSlot] 슬롯[%d] 카드 등록: %s"), SlotIndex, *Card->GetName());
}

USCSkillCardWidget* USCSkillQueueSlotWidget::TakeCard()
{
	USCSkillCardWidget* Card = SlottedCard.Get();
	SlottedCard = nullptr;
	OnSlotCleared();
	return Card;
}

bool USCSkillQueueSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (!IsEmpty())
	{
		UE_LOG(LogStrandingChain, Log, TEXT("[SCSkillQueueSlot] 슬롯[%d] 이미 차있음 — 거부."), SlotIndex);
		return false;
	}

	USCSkillCardWidget* DraggedCard = Cast<USCSkillCardWidget>(InOperation->Payload);
	if (!IsValid(DraggedCard)) { return false; }

	USCSkillQueueWidget* Queue = OwnerQueue.Get();
	if (!IsValid(Queue)) { return false; }

	Queue->RegisterCardToSlot(DraggedCard, this);
	return true;
}

FReply USCSkillQueueSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 우클릭 → 카드 원위치 반환
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && !IsEmpty())
	{
		USCSkillQueueWidget* Queue = OwnerQueue.Get();
		if (IsValid(Queue))
		{
			Queue->ReturnCardFromSlot(this);
		}
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

bool USCSkillQueueSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	return IsEmpty();
}
