// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillCardWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Battle/SCBattleTypes.h"
#include "SCSkillCardWidget.generated.h"

class USCSkillBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillCardClicked, USCSkillBase*, Skill, int32, DrawnIndex);

UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCSkillCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "SC|Card")
	TObjectPtr<USCSkillBase> SkillRef;

	UPROPERTY(BlueprintReadOnly, Category = "SC|Card")
	int32 DrawnIndex = 0;

	/** 큐에 등록됐는지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "SC|Card")
	bool bEnqueued = false;

	UPROPERTY(BlueprintAssignable, Category = "SC|Card")
	FOnSkillCardClicked OnSkillCardClicked;

	UFUNCTION(BlueprintCallable, Category = "SC|Card")
	void InitCard(USCSkillBase* InSkill, int32 InDrawnIndex);

	UFUNCTION(BlueprintCallable, Category = "SC|Card")
	void SetEnqueued(bool bInEnqueued);

	/**
	 * Blueprint의 Button OnClicked에서 호출.
	 * WBP_SkillCard 안에 Button 위젯을 쓰는 경우
	 * Button → OnClicked → NotifyClicked 연결.
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Card")
	void NotifyClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Card")
	void OnCardInitialized(const FSCSkillData& SkillData);

	/** True = 등록됨(어둡게), False = 미등록(원래대로) */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Card")
	void OnEnqueueStateChanged(bool bIsEnqueued);

protected:
	/** Button 없이 위젯 자체를 클릭 가능하게 쓸 때 사용 */
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
