// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillPanelWidget.cpp

#include "UI/SCSkillPanelWidget.h"
#include "UI/SCSkillCardWidget.h"
#include "UI/SCSkillQueueWidget.h"
#include "UI/SCSkillQueueSlotWidget.h"
#include "Battle/SCCharacterBase.h"
#include "Battle/SCSkillBase.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "StrandingChain.h"

void USCSkillPanelWidget::OpenPanel(ASCCharacterBase* InCharacter)
{
	if (!IsValid(InCharacter) || InCharacter->DrawnSkills.IsEmpty()) { return; }

	SelectedCharacter = InCharacter;
	BuildCards(InCharacter);
	SetVisibility(ESlateVisibility::Visible);
	PlayOpenAnimation();

	if (IsValid(QueueWidgetInstance))
	{
		QueueWidgetInstance->ClearSlots();
		QueueWidgetInstance->InitSlots();
		QueueWidgetInstance->SetVisibility(ESlateVisibility::Visible);

		// 슬롯 취소 델리게이트 바인딩 — 색상 복구를 위해
		QueueWidgetInstance->OnCardReturnedFromSlot.AddDynamic(
			this, &USCSkillPanelWidget::OnQueueCardReturnedHandler);
	}

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillPanel] 열림: %s"), *InCharacter->GetName());
}

void USCSkillPanelWidget::ClosePanel()
{
	SetVisibility(ESlateVisibility::Hidden);
	PlayCloseAnimation();

	if (IsValid(QueueWidgetInstance))
	{
		// 델리게이트 언바인딩
		QueueWidgetInstance->OnCardReturnedFromSlot.RemoveDynamic(
			this, &USCSkillPanelWidget::OnQueueCardReturnedHandler);

		QueueWidgetInstance->ClearSlots();
		QueueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	ClearCards();
	SelectedCharacter.Reset();
	UE_LOG(LogStrandingChain, Log, TEXT("[SCSkillPanel] 닫힘."));
}

void USCSkillPanelWidget::BuildCards(ASCCharacterBase* InCharacter)
{
	ClearCards();
	if (!IsValid(SkillCardClass) || !IsValid(CardContainer)) { return; }

	TArray<USCSkillCardWidget*> CreatedCards;
	for (int32 i = 0; i < InCharacter->DrawnSkills.Num(); ++i)
	{
		USCSkillBase* SkillInst = InCharacter->DrawnSkills[i].Get();
		if (!IsValid(SkillInst)) { continue; }

		USCSkillCardWidget* Card = CreateWidget<USCSkillCardWidget>(this, SkillCardClass);
		if (!IsValid(Card)) { continue; }

		Card->InitCard(SkillInst, i);
		Card->OnSkillCardClicked.AddDynamic(this, &USCSkillPanelWidget::OnCardClicked);

		DrawnCardWidgets.Add(Card);

		UHorizontalBoxSlot* CardSlot = CardContainer->AddChildToHorizontalBox(Card);
		if (IsValid(CardSlot))
		{
			CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			CardSlot->SetPadding(FMargin(6.f, 4.f));
			CardSlot->SetHorizontalAlignment(HAlign_Fill);
			CardSlot->SetVerticalAlignment(VAlign_Fill);
		}
		CreatedCards.Add(Card);
	}

	OnCardsReady(CreatedCards);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillPanel] 카드 %d장 생성."), CreatedCards.Num());
}

void USCSkillPanelWidget::ClearCards()
{
	for (TObjectPtr<USCSkillCardWidget>& Card : DrawnCardWidgets)
	{
		if (IsValid(Card))
		{
			Card->OnSkillCardClicked.RemoveDynamic(this, &USCSkillPanelWidget::OnCardClicked);
		}
	}
	if (IsValid(CardContainer)) { CardContainer->ClearChildren(); }
	DrawnCardWidgets.Empty();
	EnqueuedDrawnIndices.Empty();
	EnqueuedCount = 0;
}

TArray<int32> USCSkillPanelWidget::GetEnqueuedDrawnIndices() const
{
	if (IsValid(QueueWidgetInstance))
	{
		return QueueWidgetInstance->GetQueuedDrawnIndices();
	}
	return EnqueuedDrawnIndices;
}

void USCSkillPanelWidget::OnCardClicked(USCSkillBase* InSkill, int32 InDrawnIndex)
{
	if (!IsValid(InSkill)) { return; }

	USCSkillCardWidget* ClickedCard = FindCardWidgetByDrawnIndex(InDrawnIndex);
	if (!IsValid(ClickedCard)) { return; }

	// 이미 등록된 카드 → 재클릭으로 해제
	if (ClickedCard->bEnqueued)
	{
		DequeueCardByDrawnIndex(InDrawnIndex);
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCSkillPanel] 카드 해제(재클릭): DrawnIndex=%d"), InDrawnIndex);
		return;
	}

	// 최대 3개 제한
	if (EnqueuedCount >= 3)
	{
		UE_LOG(LogStrandingChain, Log, TEXT("[SCSkillPanel] 큐 가득 참."));
		return;
	}

	// QueueWidget 슬롯 표시
	if (IsValid(QueueWidgetInstance))
	{
		USCSkillQueueSlotWidget* EmptySlot = QueueWidgetInstance->FindFirstEmptySlot();
		if (IsValid(EmptySlot))
		{
			QueueWidgetInstance->RegisterCardToSlot(ClickedCard, EmptySlot);
		}
	}

	// 카드 상태 변경 (QueueWidget 유무와 무관하게 반드시 실행)
	ClickedCard->SetEnqueued(true);
	EnqueuedDrawnIndices.Add(InDrawnIndex);
	++EnqueuedCount;

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillPanel] 카드 등록: DrawnIndex=%d (총 %d개)"),
		InDrawnIndex, EnqueuedCount);
}

void USCSkillPanelWidget::OnQueueCardReturnedHandler(int32 ReturnedDrawnIndex)
{
	// 큐 슬롯 클릭으로 취소됐을 때 호출
	// 카드 색상 복구 + 내부 상태 동기화
	USCSkillCardWidget* Card = FindCardWidgetByDrawnIndex(ReturnedDrawnIndex);
	if (IsValid(Card) && Card->bEnqueued)
	{
		Card->SetEnqueued(false);  // → OnEnqueueStateChanged(false) → Blueprint 색상 복구
		EnqueuedDrawnIndices.Remove(ReturnedDrawnIndex);
		if (EnqueuedCount > 0) { --EnqueuedCount; }

		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCSkillPanel] 슬롯 취소 → 카드 색상 복구: DrawnIndex=%d (남은 %d개)"),
			ReturnedDrawnIndex, EnqueuedCount);
	}
}

void USCSkillPanelWidget::DequeueCardByDrawnIndex(int32 InDrawnIndex)
{
	if (IsValid(QueueWidgetInstance))
	{
		TArray<int32> CurrentIndices = QueueWidgetInstance->GetQueuedDrawnIndices();
		if (CurrentIndices.Contains(InDrawnIndex))
		{
			TArray<int32> Remaining;
			for (int32 Idx : CurrentIndices)
			{
				if (Idx != InDrawnIndex) { Remaining.Add(Idx); }
			}

			for (TObjectPtr<USCSkillCardWidget>& C : DrawnCardWidgets)
			{
				if (IsValid(C)) { C->SetEnqueued(false); }
			}

			QueueWidgetInstance->ClearSlots();
			QueueWidgetInstance->InitSlots();
			EnqueuedDrawnIndices.Empty();
			EnqueuedCount = 0;

			for (int32 RemainIdx : Remaining)
			{
				USCSkillCardWidget* C = FindCardWidgetByDrawnIndex(RemainIdx);
				USCSkillQueueSlotWidget* NextSlot = QueueWidgetInstance->FindFirstEmptySlot();
				if (IsValid(C) && IsValid(NextSlot))
				{
					QueueWidgetInstance->RegisterCardToSlot(C, NextSlot);
					C->SetEnqueued(true);
					EnqueuedDrawnIndices.Add(RemainIdx);
					++EnqueuedCount;
				}
			}
			return;
		}
	}

	// QueueWidget 없는 경우
	USCSkillCardWidget* Card = FindCardWidgetByDrawnIndex(InDrawnIndex);
	if (IsValid(Card) && Card->bEnqueued)
	{
		Card->SetEnqueued(false);
		EnqueuedDrawnIndices.Remove(InDrawnIndex);
		if (EnqueuedCount > 0) { --EnqueuedCount; }
	}
}

USCSkillCardWidget* USCSkillPanelWidget::FindCardWidgetByDrawnIndex(int32 InDrawnIndex) const
{
	for (const TObjectPtr<USCSkillCardWidget>& Card : DrawnCardWidgets)
	{
		if (IsValid(Card) && Card->DrawnIndex == InDrawnIndex)
		{
			return Card.Get();
		}
	}
	return nullptr;
}

bool USCSkillPanelWidget::IsMouseOverPanelContent() const
{
	if (!IsValid(CardContainer)) { return false; }
	const FVector2D CursorAbs = FSlateApplication::Get().GetCursorPos();
	const FGeometry& CardGeo  = CardContainer->GetCachedGeometry();
	const FVector2D  CardMin  = CardGeo.GetAbsolutePosition();
	const FVector2D  CardSize = CardGeo.GetAbsoluteSize();
	return CursorAbs.X >= CardMin.X - 20.f
		&& CursorAbs.X <= CardMin.X + CardSize.X + 20.f
		&& CursorAbs.Y >= CardMin.Y - 20.f
		&& CursorAbs.Y <= CardMin.Y + CardSize.Y + 20.f;
}
