// Copyright University of Inland Norway


#include "SimulationHUD.h"
#include "Kismet/GameplayStatics.h"
#include "SimulationController.h"

void ASimulationHUD::BeginPlay()
{
	Super::BeginPlay();
    SimulationController = Cast<ASimulationController>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulationController::StaticClass()));

    if (!SimulationController.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: SimulationController not found!"));
    }
}

void ASimulationHUD::DrawHUD()
{
	Super::DrawHUD();
	
	if (!SimulationController.IsValid())
	{	
		UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: SimulationController not found!"));
		return;
	}

    const FVector2D ScreenPosition(50.0f, 50.0f); // X, Y position on screen
    const FLinearColor TextColor = FLinearColor::White;
	constexpr float TextScale = 2.f;

    //FString message = FString::Printf(TEXT("Day: %d, Humans: %f, Bitten: %f, Zombies: %f"),
		//SimulationController->timeStepsFinished,
        //SimulationController->Susceptible,
        //SimulationController->Bitten,
        //SimulationController->Zombies);

    //DrawText(message, textColor, screenPosition.X, screenPosition.Y, nullptr, textScale, true);

    // Multiple lines for better organization
    const FString StepMessage = FString::Printf(TEXT("Day: %d"), SimulationController->TimeStepsFinished);
    const FString HumansMessage = FString::Printf(TEXT("Humans: %d"), static_cast<int>(SimulationController->Susceptible));
    const FString BittenMessage = FString::Printf(TEXT("Bitten: %d"), static_cast<int>(SimulationController->Bitten));
    const FString ZombiesMessage = FString::Printf(TEXT("Zombies: %d"), static_cast<int>(SimulationController->Zombies));

    DrawText(StepMessage, TextColor, ScreenPosition.X, ScreenPosition.Y, nullptr, TextScale, true);
    DrawText(HumansMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 15.0f, nullptr, TextScale, true);
    DrawText(BittenMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 30.0f, nullptr, TextScale, true);
    DrawText(ZombiesMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 45.0f, nullptr, TextScale, true);
}
