// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillPanelWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SCSkillPanelWidget.generated.h"

class USCSkillCardWidget;
class USCSkillQueueWidget;
class USCSkillQueueSlotWidget;
class ASCCharacterBase;
class USCSkillBase;
class UHorizontalBox;

UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCSkillPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Panel")
	TSubclassOf<USCSkillCardWidget> SkillCardClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Panel")
	TSubclassOf<USCSkillQueueWidget> QueueWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "SC|Panel")
	void OpenPanel(ASCCharacterBase* InCharacter);

	UFUNCTION(BlueprintCallable, Category = "SC|Panel")
	void ClosePanel();

	UFUNCTION(BlueprintPure, Category = "SC|Panel")
	ASCCharacterBase* GetSelectedCharacter() const { return SelectedCharacter.Get(); }

	bool IsMouseOverPanelContent() const;

	/** 등록 순서가 보장된 DrawnIndex 배열 반환 (PlayerController에서 실행 시 사용) */
	UFUNCTION(BlueprintCallable, Category = "SC|Panel")
	TArray<int32> GetEnqueuedDrawnIndices() const;

	UFUNCTION(BlueprintPure, Category = "SC|Panel")
	int32 GetEnqueuedCount() const { return EnqueuedCount; }

	UPROPERTY(BlueprintReadOnly, Category = "SC|Panel")
	TObjectPtr<USCSkillQueueWidget> QueueWidgetInstance;

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Panel")
	void PlayOpenAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Panel")
	void PlayCloseAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Panel")
	void OnCardsReady(const TArray<USCSkillCardWidget*>& Cards);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> CardContainer;

private:
	TWeakObjectPtr<ASCCharacterBase> SelectedCharacter;
	TArray<TObjectPtr<USCSkillCardWidget>> DrawnCardWidgets;

	/** 등록 순서 기록 배열 (QueueWidget 없을 때도 순서 보장) */
	TArray<int32> EnqueuedDrawnIndices;
	int32 EnqueuedCount = 0;

	void BuildCards(ASCCharacterBase* InCharacter);
	void ClearCards();

	/** 카드 클릭 → 등록/해제 */
	UFUNCTION()
	void OnCardClicked(USCSkillBase* InSkill, int32 InDrawnIndex);

	/**
	 * 큐 슬롯 클릭으로 취소됐을 때 호출
	 * OnCardReturnedFromSlot 델리게이트 수신 → SetEnqueued(false) + 색상 복구
	 */
	UFUNCTION()
	void OnQueueCardReturnedHandler(int32 ReturnedDrawnIndex);

	void DequeueCardByDrawnIndex(int32 InDrawnIndex);
	USCSkillCardWidget* FindCardWidgetByDrawnIndex(int32 InDrawnIndex) const;
};
