// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillPanelWidget.h
// 역할: 캐릭터 클릭 시 화면 하단에서 슬라이드업
//       드로우된 스킬 3장 카드를 가로 나열

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SCSkillPanelWidget.generated.h"

class USCSkillCardWidget;
class ASCCharacterBase;
class USCSkillBase;
class UHorizontalBox;
class UNamedSlot;

UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCSkillPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ── 설정 ────────────────────────────────
	/** 카드 위젯 클래스 (Blueprint에서 WBP_SkillCard 지정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Panel")
	TSubclassOf<USCSkillCardWidget> SkillCardClass;

	// ── C++ API ──────────────────────────────
	/**
	 * 패널 열기 — 캐릭터 클릭 시 PlayerController에서 호출
	 * @param Character 선택된 아군 캐릭터
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Panel")
	void OpenPanel(ASCCharacterBase* Character);

	/** 패널 닫기 */
	UFUNCTION(BlueprintCallable, Category = "SC|Panel")
	void ClosePanel();

	/** 현재 선택된 캐릭터 반환 */
	UFUNCTION(BlueprintPure, Category = "SC|Panel")
	ASCCharacterBase* GetSelectedCharacter() const { return SelectedCharacter.Get(); }

	// ── Blueprint 이벤트 ─────────────────────
	/** 슬라이드업 애니메이션 재생 — Blueprint에서 UMG 애니메이션 연결 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Panel")
	void PlayOpenAnimation();

	/** 슬라이드다운 애니메이션 재생 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Panel")
	void PlayCloseAnimation();

	/** 카드 3장이 만들어진 뒤 호출 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Panel")
	void OnCardsReady(const TArray<USCSkillCardWidget*>& Cards);

protected:
	/** Blueprint에서 카드를 넣을 HorizontalBox 바인딩
	 *  WBP_SkillPanel에서 HorizontalBox 이름을 "CardContainer"로 지정 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> CardContainer;

private:
	TWeakObjectPtr<ASCCharacterBase> SelectedCharacter;

	void BuildCards(ASCCharacterBase* Character);
	void ClearCards();
};
