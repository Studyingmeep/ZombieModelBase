// Copyright University of Inland Norway


#include "SimulationHUD.h"
#include "Kismet/GameplayStatics.h"
#include "SimulationController.h"
#include "Public/SimGameController.h"

void ASimulationHUD::BeginPlay()
{
	Super::BeginPlay();
	
	/// Cast to see if we can find SimulationController first, and then try cast and find SimGameController.
    SimulationController = Cast<ASimulationController>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulationController::StaticClass()));

    if (!SimulationController.IsValid())
    {
    	SimGameController = Cast<ASimGameController>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimGameController::StaticClass()));
	    if (!SimGameController.IsValid())
	    {
		    UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: Neither SimulationController nor SimGameController found!"));
	    }
	    else
	    {
	    	UE_LOG(LogTemp, Verbose, TEXT("SimulationHUD: ASimGameController found!"));
	    }
    }
}

void ASimulationHUD::DrawHUD()
{
	Super::DrawHUD();
	
	if (!SimulationController.IsValid() && !SimGameController.IsValid())
	{	
		UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: Neither SimulationController nor SimGameController found!"));
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
    const FString StepMessage = FString::Printf(TEXT("Day: %d"), SimulationController.IsValid() ? SimulationController->TimeStepsFinished : SimGameController->CurrentDay);
    const FString HumansMessage = FString::Printf(TEXT("Humans: %d"), SimulationController.IsValid() ? static_cast<int>(SimulationController->Susceptible) : static_cast<int>(SimGameController->Susceptible));
    const FString BittenMessage = FString::Printf(TEXT("Bitten: %d"), SimulationController.IsValid() ? static_cast<int>(SimulationController->Bitten) : static_cast<int>(SimGameController->Bitten));
    const FString ZombiesMessage = FString::Printf(TEXT("Zombies: %d"), SimulationController.IsValid() ? static_cast<int>(SimulationController->Zombies) : static_cast<int>(SimGameController->Zombies));

    DrawText(StepMessage, TextColor, ScreenPosition.X, ScreenPosition.Y, nullptr, TextScale, true);
    DrawText(HumansMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 15.0f, nullptr, TextScale, true);
    DrawText(BittenMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 30.0f, nullptr, TextScale, true);
    DrawText(ZombiesMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 45.0f, nullptr, TextScale, true);
}
