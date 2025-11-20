// Copyright University of Inland Norway

#include "ZombieApocalypse/Public/CustomPlayerController.h"
#include "GameFramework/HUD.h"
#include "ZombieApocalypse/SimulationHUD.h"
#include "ZombieApocalypse/Public/SimGameController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Zombie.h"
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
			UE_LOG(LogTemp, Warning, TEXT("Binding IA_Leftclick"));
		}
	}
}

void ACustomPlayerController::HandleLeftClick(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Left Click!"));
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
	
	UE_LOG(LogTemp, Warning, TEXT("Left Click not handled by HUD!"));
	
	// Step 1: Check if the click is on a HUD button; if so, don't spawn zombies
	FVector2D MousePos;
	if (!GetMousePosition(MousePos.X, MousePos.Y))
	{
		// Mouse position not valid; skip processing
		return;
	}

	FHitResult Hit;
	const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	 
	UE_LOG(LogTemp, Warning, TEXT("Mouse click at %s"), *MousePos.ToString());
	
	if (bHit && Hit.bBlockingHit)
	{
		if (!GameController->bPatientZeroSpawned)
		{
			UE_LOG(LogTemp, Warning, TEXT("Patient Zero Spawned!"));
			
			FVector SpawnPos = Hit.ImpactPoint;
			DrawDebugSphere(GetWorld(), SpawnPos, 25.f, 12, FColor::Green, false, 2.f);
			
			FRotator SpawnRotation = FRotator::ZeroRotator;
	 
			// Spawn the zombie actor
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			
			AActor* NewZombie = GetWorld()->SpawnActor<AActor>(GameController->ZombieClass, SpawnPos, SpawnRotation, SpawnParams);
			
			if (NewZombie)
			{
				UE_LOG(LogTemp, Log, TEXT("Spawned Zombie at %s"), *SpawnPos.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to spawn Zombie!"));
			}
			
			GameController->SpawnEntityAt(Hit.ImpactPoint, false);
			GameController->bPatientZeroSpawned = true;
		}
	}
}
