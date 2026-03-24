// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleAutoSetup.h
// 역할: 레벨에 하나만 배치하면 BeginPlay에서
//       아레나 + 8캐릭터(TeamA 4 / TeamB 4)를 전부 자동 스폰하고
//       SCTurnManager에 배틀 시작 요청까지 처리.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SCBattleTypes.h"
#include "SCBattleAutoSetup.generated.h"

class ASCBattleArena;
class ASCPlaceholderCharacter;
class USCTurnManager;

/** 팀 한 명의 캐릭터 설정 */
USTRUCT(BlueprintType)
struct FSCCharacterSlotConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	ESCPosition Position = ESCPosition::Dealer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FLinearColor DebugColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxHP = 100;
};

UCLASS(Blueprintable)
class STRANDINGCHAIN_API ASCBattleAutoSetup : public AActor
{
	GENERATED_BODY()

public:
	ASCBattleAutoSetup();

	// ── 아레나 설정 ─────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Setup")
	TSubclassOf<ASCBattleArena> ArenaClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Setup")
	TSubclassOf<ASCPlaceholderCharacter> CharacterClass;

	// ── 팀A 설정 (4명) ──────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Setup",
		meta = (ClampMin = 1, ClampMax = 4))
	TArray<FSCCharacterSlotConfig> TeamAConfig;

	// ── 팀B 설정 (4명) ──────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Setup",
		meta = (ClampMin = 1, ClampMax = 4))
	TArray<FSCCharacterSlotConfig> TeamBConfig;

	// ── 런타임 참조 ─────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Runtime")
	TObjectPtr<ASCBattleArena> SpawnedArena;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Runtime")
	TArray<TObjectPtr<ASCPlaceholderCharacter>> TeamACharacters;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Runtime")
	TArray<TObjectPtr<ASCPlaceholderCharacter>> TeamBCharacters;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnArena();
	void SpawnCharacters(
		const TArray<FSCCharacterSlotConfig>& Configs,
		ESCTeam Team,
		TArray<TObjectPtr<ASCPlaceholderCharacter>>& OutChars);
	void StartBattle();

	void SetDefaultConfigs();
};
