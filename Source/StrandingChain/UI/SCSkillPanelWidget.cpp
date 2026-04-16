// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillPanelWidget.cpp

#include "UI/SCSkillPanelWidget.h"
#include "UI/SCSkillCardWidget.h"
#include "UI/SCSkillQueueWidget.h"
#include "Battle/SCCharacterBase.h"
#include "Battle/SCSkillBase.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "StrandingChain.h"

void USCSkillPanelWidget::OpenPanel(ASCCharacterBase* Character)
{
	if (!IsValid(Character)) { return; }
	if (Character->DrawnSkills.IsEmpty()) { return; }

	SelectedCharacter = Character;
	BuildCards(Character);
	SetVisibility(ESlateVisibility::Visible);
	PlayOpenAnimation();

	// 큐 위젯 열기
	if (IsValid(QueueWidgetInstance))
	{
		QueueWidgetInstance->ClearSlots();
		QueueWidgetInstance->InitSlots();
		QueueWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	}

	UE_LOG(LogStrandingChain, Log, TEXT("[SCSkillPanel] 열림: %s"), *Character->GetName());
}

void USCSkillPanelWidget::ClosePanel()
{
	SetVisibility(ESlateVisibility::Hidden);
	PlayCloseAnimation();

	if (IsValid(QueueWidgetInstance))
	{
		QueueWidgetInstance->ClearSlots();
		QueueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	ClearCards();
	SelectedCharacter.Reset();
	UE_LOG(LogStrandingChain, Log, TEXT("[SCSkillPanel] 닫힘."));
}

void USCSkillPanelWidget::BuildCards(ASCCharacterBase* Character)
{
	ClearCards();
	if (!IsValid(SkillCardClass) || !IsValid(CardContainer)) { return; }

	TArray<USCSkillCardWidget*> CreatedCards;
	for (int32 i = 0; i < Character->DrawnSkills.Num(); ++i)
	{
		USCSkillBase* Skill = Character->DrawnSkills[i].Get();
		if (!IsValid(Skill)) { continue; }

		USCSkillCardWidget* Card = CreateWidget<USCSkillCardWidget>(this, SkillCardClass);
		if (!IsValid(Card)) { continue; }

		Card->InitCard(Skill, i);
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
	UE_LOG(LogStrandingChain, Log, TEXT("[SCSkillPanel] 카드 %d장 생성."), CreatedCards.Num());
}

void USCSkillPanelWidget::ClearCards()
{
	if (IsValid(CardContainer)) { CardContainer->ClearChildren(); }
	DrawnCardWidgets.Empty();
}

bool USCSkillPanelWidget::IsMouseOverPanelContent() const
{
	if (!IsValid(CardContainer)) { return false; }
	const FVector2D CursorAbs = FSlateApplication::Get().GetCursorPos();
	const FGeometry& CardGeo  = CardContainer->GetCachedGeometry();
	const FVector2D  CardMin  = CardGeo.GetAbsolutePosition();
	const FVector2D  CardSize = CardGeo.GetAbsoluteSize();
	const float Left   = CardMin.X - 20.f;
	const float Right  = CardMin.X + CardSize.X + 20.f;
	const float Top    = CardMin.Y - 20.f;
	const float Bottom = CardMin.Y + CardSize.Y + 20.f;
	return CursorAbs.X >= Left && CursorAbs.X <= Right
		&& CursorAbs.Y >= Top  && CursorAbs.Y <= Bottom;
}
