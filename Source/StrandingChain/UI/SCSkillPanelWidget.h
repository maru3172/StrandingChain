// Copyright StrandingChain. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SCSkillPanelWidget.generated.h"

class USCSkillCardWidget;
class USCSkillQueueWidget;
class ASCCharacterBase;
class USCSkillBase;
class UHorizontalBox;

UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCSkillPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 카드 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Panel")
	TSubclassOf<USCSkillCardWidget> SkillCardClass;

	/** 큐 위젯 클래스 — 화면 중앙에 별도로 띄울 위젯 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Panel")
	TSubclassOf<USCSkillQueueWidget> QueueWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "SC|Panel")
	void OpenPanel(ASCCharacterBase* Character);

	UFUNCTION(BlueprintCallable, Category = "SC|Panel")
	void ClosePanel();

	UFUNCTION(BlueprintPure, Category = "SC|Panel")
	ASCCharacterBase* GetSelectedCharacter() const { return SelectedCharacter.Get(); }

	bool IsMouseOverPanelContent() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Panel")
	void PlayOpenAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Panel")
	void PlayCloseAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Panel")
	void OnCardsReady(const TArray<USCSkillCardWidget*>& Cards);

	/** 큐 위젯 인스턴스 (PlayerController에서 접근용) */
	UPROPERTY(BlueprintReadOnly, Category = "SC|Panel")
	TObjectPtr<USCSkillQueueWidget> QueueWidgetInstance;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> CardContainer;

private:
	TWeakObjectPtr<ASCCharacterBase> SelectedCharacter;
	TArray<TObjectPtr<USCSkillCardWidget>> DrawnCardWidgets;

	void BuildCards(ASCCharacterBase* Character);
	void ClearCards();
};
