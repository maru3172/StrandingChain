// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattlePlayerController.cpp

#include "SCBattlePlayerController.h"
#include "SCBattleArena.h"
#include "SCCharacterBase.h"
#include "SCTurnManager.h"
#include "SCTeamState.h"
#include "SCBattleTypes.h"
#include "StrandingChain.h"
#include "UI/SCSkillPanelWidget.h"
#include "UI/SCSkillQueueWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"

ASCBattlePlayerController::ASCBattlePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
}

void ASCBattlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor   = true;
	bEnableClickEvents = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), ASCBattleArena::StaticClass());
	if (IsValid(Found)) { ArenaRef = Cast<ASCBattleArena>(Found); }

	UGameInstance* GI = GetGameInstance();
	if (IsValid(GI))
	{
		USCTurnManager* TM = GI->GetSubsystem<USCTurnManager>();
		if (IsValid(TM))
		{
			TurnManagerRef = TM;
			// 플레이어 턴 시작 알림 바인딩
			TM->OnPlayerTurnStarted.AddDynamic(
				this, &ASCBattlePlayerController::OnPlayerTurnStartedHandler);
			UE_LOG(LogStrandingChain, Log, TEXT("[SCBattlePC] TurnManager 바인딩 완료."));
		}
	}

	FVector2D VP = FVector2D(1920.f, 1080.f);
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const float Scale = GEngine->GameViewport->GetDPIScale();
		VP = ViewportSize / Scale;
	}

	if (IsValid(SkillPanelClass))
	{
		SkillPanelWidget = CreateWidget<USCSkillPanelWidget>(this, SkillPanelClass);
		if (IsValid(SkillPanelWidget))
		{
			SkillPanelWidget->AddToViewport(0);
			SkillPanelWidget->SetVisibility(ESlateVisibility::Hidden);

			if (IsValid(SkillPanelWidget->QueueWidgetClass))
			{
				USCSkillQueueWidget* QueueWidget = CreateWidget<USCSkillQueueWidget>(
					this, SkillPanelWidget->QueueWidgetClass);
				if (IsValid(QueueWidget))
				{
					QueueWidget->AddToViewport(1);
					QueueWidget->SetVisibility(ESlateVisibility::Hidden);
					QueueWidget->SetPositionInViewport(
						FVector2D(VP.X * 0.15f, VP.Y * 0.45f), false);
					SkillPanelWidget->QueueWidgetInstance = QueueWidget;
				}
			}
		}
	}

	if (IsValid(EnemyTargetWidgetClass))
	{
		EnemyTargetWidget = CreateWidget<UUserWidget>(this, EnemyTargetWidgetClass);
		if (IsValid(EnemyTargetWidget))
		{
			EnemyTargetWidget->AddToViewport(2);
			EnemyTargetWidget->SetVisibility(ESlateVisibility::Hidden);
			EnemyTargetWidget->SetPositionInViewport(
				FVector2D(VP.X * 0.6f, VP.Y * 0.45f), false);
		}
	}
}

void ASCBattlePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!IsValid(InputComponent)) { return; }

	InputComponent->BindAxis("SC_CameraMove",
		this, &ASCBattlePlayerController::OnCameraMoveAxis);
	InputComponent->BindAction("SC_ZoomIn",  IE_Pressed,
		this, &ASCBattlePlayerController::ZoomIn);
	InputComponent->BindAction("SC_ZoomOut", IE_Pressed,
		this, &ASCBattlePlayerController::ZoomOut);
	InputComponent->BindAction("SC_Select",  IE_Pressed,
		this, &ASCBattlePlayerController::OnMouseLeftClick);

	InputComponent->BindAxis("MoveForward");
	InputComponent->BindAxis("MoveRight");
	InputComponent->BindAxis("MoveUp");
}

void ASCBattlePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (FMath::IsNearlyZero(MoveAxisValue)) { return; }
	ASCBattleArena* Arena = ArenaRef.Get();
	if (!IsValid(Arena)) { return; }
	FVector Loc = Arena->GetActorLocation();
	Loc.X += MoveAxisValue * CameraMoveSpeed * DeltaTime;
	Arena->SetActorLocation(Loc);
}

void ASCBattlePlayerController::OnCameraMoveAxis(float Value) { MoveAxisValue = Value; }

void ASCBattlePlayerController::ZoomIn()
{
	ASCBattleArena* Arena = ArenaRef.Get();
	if (!IsValid(Arena) || !IsValid(Arena->SpringArm)) { return; }
	Arena->SpringArm->TargetArmLength = FMath::Clamp(
		Arena->SpringArm->TargetArmLength - ZoomStep, ZoomMin, ZoomMax);
}

void ASCBattlePlayerController::ZoomOut()
{
	ASCBattleArena* Arena = ArenaRef.Get();
	if (!IsValid(Arena) || !IsValid(Arena->SpringArm)) { return; }
	Arena->SpringArm->TargetArmLength = FMath::Clamp(
		Arena->SpringArm->TargetArmLength + ZoomStep, ZoomMin, ZoomMax);
}

bool ASCBattlePlayerController::IsMouseOverWidget(UUserWidget* InWidget) const
{
	if (!IsValid(InWidget) || InWidget->GetVisibility() != ESlateVisibility::Visible)
		return false;
	const FVector2D Cur = FSlateApplication::Get().GetCursorPos();
	const FGeometry& Geo = InWidget->GetCachedGeometry();
	const FVector2D Min = Geo.GetAbsolutePosition();
	const FVector2D Size = Geo.GetAbsoluteSize();
	if (Size.IsNearlyZero()) { return false; }
	return Cur.X >= Min.X && Cur.X <= Min.X + Size.X
		&& Cur.Y >= Min.Y && Cur.Y <= Min.Y + Size.Y;
}

void ASCBattlePlayerController::OnMouseLeftClick()
{
	if (IsValid(SkillPanelWidget) &&
		SkillPanelWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		if (SkillPanelWidget->IsMouseOverPanelContent()) { return; }
		if (IsMouseOverWidget(EnemyTargetWidget)) { return; }

		FHitResult HitResult;
		GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
		if (HitResult.bBlockingHit)
		{
			ASCCharacterBase* HitChar = Cast<ASCCharacterBase>(HitResult.GetActor());
			if (IsValid(HitChar) && HitChar->Team == ESCTeam::TeamA && !HitChar->IsDead())
			{
				SelectedCharacter = HitChar;
				OpenSkillPanel(HitChar);
				return;
			}
		}
		CloseSkillPanel();
		return;
	}
	TrySelectCharacter();
}

void ASCBattlePlayerController::TrySelectCharacter()
{
	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	if (!HitResult.bBlockingHit) { return; }

	ASCCharacterBase* HitChar = Cast<ASCCharacterBase>(HitResult.GetActor());
	if (!IsValid(HitChar) || HitChar->Team != ESCTeam::TeamA || HitChar->IsDead()) { return; }

	SelectedCharacter = HitChar;
	OpenSkillPanel(HitChar);
}

void ASCBattlePlayerController::OpenSkillPanel(ASCCharacterBase* InCharacter)
{
	if (!IsValid(SkillPanelWidget)) { return; }
	if (InCharacter->DrawnSkills.IsEmpty()) { InCharacter->BeginTurn(); }

	ResetEnemySelection();
	SkillPanelWidget->OpenPanel(InCharacter);
	ShowEnemyTargetPanel();

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetWidgetToFocus(SkillPanelWidget->TakeWidget());
	SetInputMode(Mode);
}

void ASCBattlePlayerController::CloseSkillPanel()
{
	if (!IsValid(SkillPanelWidget)) { return; }
	SkillPanelWidget->ClosePanel();
	HideEnemyTargetPanel();
	SelectedCharacter.Reset();

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetWidgetToFocus(nullptr);
	SetInputMode(Mode);
}

void ASCBattlePlayerController::ShowEnemyTargetPanel()
{
	if (IsValid(EnemyTargetWidget))
		EnemyTargetWidget->SetVisibility(ESlateVisibility::Visible);
}

void ASCBattlePlayerController::HideEnemyTargetPanel()
{
	if (IsValid(EnemyTargetWidget))
		EnemyTargetWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ASCBattlePlayerController::ResetEnemySelection()
{
	SelectedEnemyIndex = -1;
	OnEnemyTargetSelected.Broadcast(-1);
}

// ── TurnManager 델리게이트 콜백 ──────────────────────────────────────────────

void ASCBattlePlayerController::OnPlayerTurnStartedHandler()
{
	// 플레이어 턴 시작 — 어떤 TeamA 캐릭터를 쓸지 선택 대기
	// 특정 캐릭터가 자동 지정되지 않으므로 패널은 열지 않음
	// 플레이어가 직접 캐릭터를 클릭해서 선택
	ResetEnemySelection();

	// 이미 열린 패널이 있으면 닫기
	if (IsValid(SkillPanelWidget) &&
		SkillPanelWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		CloseSkillPanel();
	}

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattlePC] 플레이어 턴 — 캐릭터를 선택하세요."));
}

// ── 배틀 액션 ─────────────────────────────────────────────────────────────────

void ASCBattlePlayerController::SelectEnemyTarget(int32 EnemySlotIndex)
{
	USCTurnManager* TM = TurnManagerRef.Get();
	if (!IsValid(TM)) { return; }

	TArray<ASCCharacterBase*> AliveEnemies = TM->GetAliveCharsOfTeam(ESCTeam::TeamB);
	if (!AliveEnemies.IsValidIndex(EnemySlotIndex)) { return; }

	SelectedEnemyIndex = EnemySlotIndex;
	OnEnemyTargetSelected.Broadcast(EnemySlotIndex);
}

void ASCBattlePlayerController::ConfirmSkillQueue()
{
	if (!IsValid(SkillPanelWidget)) { return; }

	if (!IsEnemySelected())
	{
		UE_LOG(LogStrandingChain, Warning, TEXT("[SCBattlePC] 적을 먼저 선택하세요."));
		return;
	}

	ASCCharacterBase* PlayerChar = SelectedCharacter.Get();
	if (!IsValid(PlayerChar) || PlayerChar->IsDead())
	{
		UE_LOG(LogStrandingChain, Warning, TEXT("[SCBattlePC] 선택된 캐릭터 없음."));
		return;
	}

	USCTurnManager* TM = TurnManagerRef.Get();
	if (!IsValid(TM)) { return; }

	TArray<ASCCharacterBase*> AliveEnemies = TM->GetAliveCharsOfTeam(ESCTeam::TeamB);
	if (!AliveEnemies.IsValidIndex(SelectedEnemyIndex))
	{
		UE_LOG(LogStrandingChain, Warning, TEXT("[SCBattlePC] 선택된 적 사망."));
		ResetEnemySelection();
		return;
	}

	// 스킬 큐 구성
	TArray<int32> Indices = SkillPanelWidget->GetEnqueuedDrawnIndices();
	if (Indices.IsEmpty())
	{
		UE_LOG(LogStrandingChain, Warning, TEXT("[SCBattlePC] 큐에 스킬 없음."));
		return;
	}
	PlayerChar->SkillQueue.Empty();
	for (int32 Idx : Indices) { PlayerChar->EnqueueSkill(Idx); }

	TArray<AActor*> Targets = { AliveEnemies[SelectedEnemyIndex] };

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattlePC] 실행: %s 스킬 %d개 → %s"),
		*PlayerChar->GetName(), PlayerChar->SkillQueue.Num(),
		*AliveEnemies[SelectedEnemyIndex]->GetName());

	ResetEnemySelection();
	CloseSkillPanel();

	// TurnManager에 플레이어 캐릭터를 명시적으로 전달
	TM->ConfirmAndExecuteQueue(PlayerChar, Targets);
}

void ASCBattlePlayerController::SkipCurrentTurn()
{
	USCTurnManager* TM = TurnManagerRef.Get();
	if (!IsValid(TM)) { return; }
	ResetEnemySelection();
	CloseSkillPanel();
	TM->SkipCurrentActorTurn();
}

// ── 조회 ─────────────────────────────────────────────────────────────────────

TArray<ASCCharacterBase*> ASCBattlePlayerController::GetAliveEnemies() const
{
	USCTurnManager* TM = TurnManagerRef.Get();
	if (!IsValid(TM)) { return {}; }
	return TM->GetAliveCharsOfTeam(ESCTeam::TeamB);
}

int32 ASCBattlePlayerController::GetTeamACost() const
{
	USCTurnManager* TM = TurnManagerRef.Get();
	if (!IsValid(TM)) { return 0; }
	USCTeamState* State = TM->GetTeamState(ESCTeam::TeamA);
	return IsValid(State) ? State->CurrentCost : 0;
}

int32 ASCBattlePlayerController::GetTeamBCost() const
{
	USCTurnManager* TM = TurnManagerRef.Get();
	if (!IsValid(TM)) { return 0; }
	USCTeamState* State = TM->GetTeamState(ESCTeam::TeamB);
	return IsValid(State) ? State->CurrentCost : 0;
}

int32 ASCBattlePlayerController::GetCurrentTurnNumber() const
{
	USCTurnManager* TM = TurnManagerRef.Get();
	return IsValid(TM) ? TM->GetTurnNumber() : 0;
}
