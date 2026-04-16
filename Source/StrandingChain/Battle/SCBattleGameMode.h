// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SCBattleTypes.h"
#include "SCBattleGameMode.generated.h"

class USCTurnManager;
class ASCCharacterBase;
class ASCBattleArena;
class ASCBattlePlayerController;

UCLASS(Blueprintable)
class STRANDINGCHAIN_API ASCBattleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASCBattleGameMode();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void InitiateBattle(
		const TArray<ASCCharacterBase*>& TeamAChars,
		const TArray<ASCCharacterBase*>& TeamBChars);

private:
	TWeakObjectPtr<USCTurnManager> TurnManager;

	UPROPERTY()
	TWeakObjectPtr<ASCCharacterBase> PendingAICharacter;

	FTimerHandle AITimerHandle;

	void SetArenaCamera();

	// InCharacter로 통일 — 델리게이트 시그니처와 일치
	UFUNCTION()
	void OnCharacterTurnBeginHandler(ASCCharacterBase* InCharacter);

	UFUNCTION()
	void ExecuteAITurn();
};
