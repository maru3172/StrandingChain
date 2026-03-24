// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCPlaceholderCharacter.h
// 역할: 테스트용 임시 캐릭터. 스켈레탈 메시 없이 캡슐+디버그 색상으로 식별.

#pragma once

#include "CoreMinimal.h"
#include "SCCharacterBase.h"
#include "SCPlaceholderCharacter.generated.h"

UCLASS(Blueprintable)
class STRANDINGCHAIN_API ASCPlaceholderCharacter : public ASCCharacterBase
{
	GENERATED_BODY()

public:
	ASCPlaceholderCharacter();

	/** 포지션별 디버그 색상 (에디터/런타임 모두 표시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Debug")
	FLinearColor DebugColor = FLinearColor::White;

protected:
	virtual void BeginPlay() override;

private:
	// 디버그용 컬러 메시 컴포넌트
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> DebugMesh;
};
