// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillQueueWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SCSkillQueueWidget.generated.h"

class USCSkillCardWidget;
class USCSkillQueueSlotWidget;
class UHorizontalBox;

/**
 * 슬롯에서 카드가 취소될 때 — SCSkillPanelWidget이 바인딩해서 색상 복구 처리
 * 파라미터: 취소된 카드의 DrawnIndex
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueSlotCardReturned, int32, ReturnedDrawnIndex);

UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCSkillQueueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Queue")
	TSubclassOf<USCSkillQueueSlotWidget> QueueSlotClass;

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void InitSlots();

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void ClearSlots();

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void RegisterCardToSlot(USCSkillCardWidget* Card, USCSkillQueueSlotWidget* InSlot);

	/**
	 * 슬롯에서 카드 제거 + OnCardReturnedFromSlot 브로드캐스트
	 * SCSkillQueueSlotWidget의 좌클릭에서 호출
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void ReturnCardFromSlot(USCSkillQueueSlotWidget* InSlot);

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	TArray<int32> GetQueuedDrawnIndices() const;

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	USCSkillQueueSlotWidget* FindFirstEmptySlot() const;

	/**
	 * 슬롯에서 카드가 취소됐을 때 브로드캐스트
	 * SCSkillPanelWidget이 바인딩 → SetEnqueued(false) 처리
	 */
	UPROPERTY(BlueprintAssignable, Category = "SC|Queue")
	FOnQueueSlotCardReturned OnCardReturnedFromSlot;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SlotContainer;

private:
	UPROPERTY()
	TArray<TObjectPtr<USCSkillQueueSlotWidget>> Slots;
};
