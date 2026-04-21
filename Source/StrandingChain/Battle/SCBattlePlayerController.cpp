// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattlePlayerController.cpp

#include "SCBattlePlayerController.h"
#include "SCBattleArena.h"
#include "SCCharacterBase.h"
#include "SCTurnManager.h"
#include "SCBattleTypes.h"
#include "StrandingChain.h"
#include "UI/SCSkillPanelWidget.h"
#include "UI/SCSkillQueueWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"

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
	if (IsValid(Found))
	{
		ArenaRef = Cast<ASCBattleArena>(Found);
	}

	UGameInstance* GI = GetGameInstance();
	if (IsValid(GI))
	{
		USCTurnManager* TM = GI->GetSubsystem<USCTurnManager>();
		if (IsValid(TM))
		{
			TurnManagerRef = TM;
			TM->OnCharacterTurnBegin.AddDynamic(
				this, &ASCBattlePlayerController::OnCharacterTurnBeginHandler);
			UE_LOG(LogStrandingChain, Log, TEXT("[SCBattlePC] TurnManager 바인딩 완료."));
		}
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

					if (GEngine && GEngine->GameViewport)
					{
						FVector2D ViewportSize;
						GEngine->GameViewport->GetViewportSize(ViewportSize);
						const float Scale  = GEngine->GameViewport->GetDPIScale();
						const FVector2D VP = ViewportSize / Scale;
						QueueWidget->SetPositionInViewport(
							FVector2D((VP.X - 400.f) * 0.5f + 150.f,
							          (VP.Y - 120.f) * 0.5f + 80.f), false);
						QueueWidget->SetDesiredSizeInViewport(FVector2D(400.f, 120.f));
					}
					SkillPanelWidget->QueueWidgetInstance = QueueWidget;
					UE_LOG(LogStrandingChain, Log, TEXT("[SCBattlePC] QueueWidget 생성 완료."));
				}
			}
			else
			{
				UE_LOG(LogStrandingChain, Warning,
					TEXT("[SCBattlePC] QueueWidgetClass 미설정 — 큐 패널 없이 동작."));
			}
			UE_LOG(LogStrandingChain, Log, TEXT("[SCBattlePC] SkillPanelWidget 생성 완료."));
		}
	}
	else
	{
		UE_LOG(LogStrandingChain, Warning, TEXT("[SCBattlePC] SkillPanelClass 미설정."));
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

void ASCBattlePlayerController::OnMouseLeftClick()
{
	if (IsValid(SkillPanelWidget) &&
		SkillPanelWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		if (SkillPanelWidget->IsMouseOverPanelContent()) { return; }

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
	if (!IsValid(SkillPanelWidget))
	{
		UE_LOG(LogStrandingChain, Error, TEXT("[SCBattlePC] SkillPanelWidget 없음."));
		return;
	}
	if (InCharacter->DrawnSkills.IsEmpty())
	{
		InCharacter->BeginTurn();
	}

	SkillPanelWidget->OpenPanel(InCharacter);

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
	SelectedCharacter.Reset();

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetWidgetToFocus(nullptr);
	SetInputMode(Mode);
}

void ASCBattlePlayerController::OnCharacterTurnBeginHandler(ASCCharacterBase* InCharacter)
{
	if (!IsValid(InCharacter)) { return; }

	if (InCharacter->Team == ESCTeam::TeamA && !InCharacter->IsDead())
	{
		SelectedCharacter = InCharacter;
		OpenSkillPanel(InCharacter);
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCBattlePC] TeamA 턴 → 패널 열기: %s"), *InCharacter->GetName());
	}
	else
	{
		CloseSkillPanel();
	}
}

void ASCBattlePlayerController::ConfirmSkillQueue()
{
	USCTurnManager* TM = TurnManagerRef.Get();
	if (!IsValid(TM)) { return; }

	ASCCharacterBase* Actor = TM->GetCurrentActor();
	if (!IsValid(Actor) || Actor->Team != ESCTeam::TeamA)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCBattlePC] ConfirmSkillQueue: 현재 액터가 TeamA가 아님."));
		return;
	}

	if (!IsValid(SkillPanelWidget))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCBattlePC] ConfirmSkillQueue: SkillPanelWidget 없음."));
		return;
	}

	// GetEnqueuedDrawnIndices: QueueWidget 유/무 모두 처리
	TArray<int32> Indices = SkillPanelWidget->GetEnqueuedDrawnIndices();

	Actor->SkillQueue.Empty();
	for (int32 Idx : Indices)
	{
		Actor->EnqueueSkill(Idx);
	}

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattlePC] SkillQueue 구성: %d개"), Actor->SkillQueue.Num());

	TArray<ASCCharacterBase*> Enemies = TM->GetAliveCharsOfTeam(ESCTeam::TeamB);
	TArray<AActor*> Targets;
	for (ASCCharacterBase* E : Enemies) { if (IsValid(E)) Targets.Add(E); }

	CloseSkillPanel();
	TM->ConfirmAndExecuteQueue(Targets);
}

void ASCBattlePlayerController::SkipCurrentTurn()
{
	USCTurnManager* TM = TurnManagerRef.Get();
	if (!IsValid(TM)) { return; }
	CloseSkillPanel();
	TM->SkipCurrentActorTurn();
	UE_LOG(LogStrandingChain, Log, TEXT("[SCBattlePC] 턴 건너뜀."));
}
