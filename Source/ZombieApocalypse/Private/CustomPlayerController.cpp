// Copyright University of Inland Norway

#include "ZombieApocalypse/Public/CustomPlayerController.h"
#include "GameFramework/HUD.h"
#include "ZombieApocalypse/SimulationHUD.h"
#include "ZombieApocalypse/Public/SimGameController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

void ACustomPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// To enable the mouse cursor and click detection
	SetShowMouseCursor(true);
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	// Add Input Mapping Context to Enhanced Input Subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer() ? GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr)
	{
		if (IMC_Player)
		{
			// Priority 0 is standard; adjust if needed for layering
			Subsystem->AddMappingContext(IMC_Player, 0);
		}
	}
	
	// Find SimGameController in the scene
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASimGameController::StaticClass(), Found);

	if (Found.Num() > 0)
	{
		GameController = Cast<ASimGameController>(Found[0]);
	}
}

void ACustomPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Cast InputComponent to EnhancedInputComponent
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_Leftclick)
		{
			// Bind the input action to your handler, trigger on press start
			EnhancedInput->BindAction(IA_Leftclick, ETriggerEvent::Completed, this, &ACustomPlayerController::HandleLeftClick);
		}

		if (IA_Escape)
		{
			// Bind the input action to your handler, trigger on press start
			EnhancedInput->BindAction(IA_Escape, ETriggerEvent::Started, this, &ACustomPlayerController::HandleEscape);
		}
	}
}

void ACustomPlayerController::HandleLeftClick(const FInputActionValue& Value)
{
	// If SimGameController can't be found, return.
	if (!GameController) return;
	
	ASimulationHUD* SimHUD = Cast<ASimulationHUD>(GetHUD());
	// Reset click handling at start or at end of input tick
	if (SimHUD)
	{
		SimHUD->ResetClickHandling();
	}
	
	// After input is processed, check if HUD consumed the click
	if (SimHUD && SimHUD->bIsClickHandled)
	{
		// Click was on HUD button, eliminate further world click processing
		return;
	}
	
	// Step 1: Check if the click is on a HUD button; if so, don't spawn zombies
	FVector2D MousePos;
	if (!GetMousePosition(MousePos.X, MousePos.Y))
	{
		// Mouse position not valid; skip processing
		return;
	}

	FHitResult Hit;
	const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	 
	if (bHit && Hit.bBlockingHit)
	{
		const FVector ClickLocation = Hit.ImpactPoint;
		if (!GameController->bPatientZeroSpawned)
		{
			GameController->SpawnEntityAt(ClickLocation, false);
		}
	}
}

void ACustomPlayerController::HandleEscape()
{
	const bool bPaused = UGameplayStatics::IsGamePaused(this);
	
	UGameplayStatics::SetGamePaused(this, !bPaused);

	// Keep IMC active while paused
	if (!bPaused)
	{
		// Game was unpaused -> now paused
		SetInputMode(FInputModeUIOnly());
	}
	else
	{
		// Game was paused -> now unpaused
		SetInputMode(FInputModeGameAndUI());
	}

	bShowMouseCursor = true;
}
