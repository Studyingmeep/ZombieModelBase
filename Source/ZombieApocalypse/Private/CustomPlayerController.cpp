// Copyright University of Inland Norway

#include "ZombieApocalypse/Public/CustomPlayerController.h"
#include "ZombieApocalypse/Public/SimGameController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

void ACustomPlayerController::BeginPlay()
{
	Super::BeginPlay();

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
			EnhancedInput->BindAction(IA_Leftclick, ETriggerEvent::Started, this, &ACustomPlayerController::HandleLeftClick);
		}
	}
}

void ACustomPlayerController::HandleLeftClick(const FInputActionValue& Value)
{
	if (!GameController) return;

	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		FVector ClickLocation = Hit.ImpactPoint;

		// Spawn the first zombie!
		GameController->SpawnEntityAt(ClickLocation, false);
	}
}
