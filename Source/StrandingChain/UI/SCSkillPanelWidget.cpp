// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillPanelWidget.cpp

#include "UI/SCSkillPanelWidget.h"
#include "UI/SCSkillCardWidget.h"
#include "Battle/SCCharacterBase.h"
#include "Battle/SCSkillBase.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "StrandingChain.h"

void USCSkillPanelWidget::OpenPanel(ASCCharacterBase* Character)
{
	if (!IsValid(Character))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCSkillPanel] OpenPanel: 유효하지 않은 캐릭터."));
		return;
	}
	if (Character->DrawnSkills.IsEmpty())
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCSkillPanel] OpenPanel: DrawnSkills가 비어있음. BeginTurn 호출 필요."));
		return;
	}

	SelectedCharacter = Character;
	BuildCards(Character);
	SetVisibility(ESlateVisibility::Visible);
	PlayOpenAnimation();

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillPanel] 패널 열림. 캐릭터=%s, 드로우스킬=%d개"),
		*Character->GetName(), Character->DrawnSkills.Num());
}

void USCSkillPanelWidget::ClosePanel()
{
	PlayCloseAnimation();
	// 애니메이션 종료 후 Blueprint에서 SetVisibility(Hidden) 호출 권장
	SelectedCharacter.Reset();

	UE_LOG(LogStrandingChain, Log, TEXT("[SCSkillPanel] 패널 닫힘."));
}

void USCSkillPanelWidget::BuildCards(ASCCharacterBase* Character)
{
	ClearCards();

	if (!IsValid(SkillCardClass))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCSkillPanel] SkillCardClass가 설정되지 않음. "
				 "WBP_SkillPanel의 SkillCardClass에 WBP_SkillCard를 지정하세요."));
		return;
	}
	if (!IsValid(CardContainer))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCSkillPanel] CardContainer(HorizontalBox)가 바인딩되지 않음. "
				 "WBP_SkillPanel에 이름이 'CardContainer'인 HorizontalBox가 필요합니다."));
		return;
	}

	TArray<USCSkillCardWidget*> CreatedCards;

	for (int32 i = 0; i < Character->DrawnSkills.Num(); ++i)
	{
		USCSkillBase* Skill = Character->DrawnSkills[i].Get();
		if (!IsValid(Skill)) { continue; }

		USCSkillCardWidget* Card = CreateWidget<USCSkillCardWidget>(
			this, SkillCardClass);
		if (!IsValid(Card)) { continue; }

		Card->InitCard(Skill, i);

		// HorizontalBox에 균등 분배로 추가
		UHorizontalBoxSlot* CardSlot = CardContainer->AddChildToHorizontalBox(Card);
		if (IsValid(CardSlot))
		{
			CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			CardSlot->SetPadding(FMargin(8.f, 0.f));
			CardSlot->SetHorizontalAlignment(HAlign_Fill);
			CardSlot->SetVerticalAlignment(VAlign_Fill);
		}

		CreatedCards.Add(Card);
	}

	OnCardsReady(CreatedCards);

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillPanel] 카드 %d장 생성 완료."), CreatedCards.Num());
}

void USCSkillPanelWidget::ClearCards()
{
	if (IsValid(CardContainer))
	{
		CardContainer->ClearChildren();
	}
}
