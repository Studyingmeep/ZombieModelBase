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
    	// Auto find SimGameController
    	TArray<AActor*> Found;
    	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASimGameController::StaticClass(), Found);
    	if (Found.Num() > 0)
    	{
    		SimGameController = Cast<ASimGameController>(Found[0]);
    	}
	    else
	    {
	    	UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: Neither ASimulationController nor ASimGameController found in ASimulationHUD!"));
	    }
    }
	
	// BUTTONS (label, position, size)
	Buttons = {
	    {"Pause",  FVector2D(50, 300), FVector2D(120, 35)},
		{"Resume", FVector2D(50, 340), FVector2D(120, 35)},
		{"1x",     FVector2D(50, 380), FVector2D(120, 35)},
		{"2x",     FVector2D(50, 420), FVector2D(120, 35)},
		{"4x",     FVector2D(50, 460), FVector2D(120, 35)},
	};
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
	const FString SpeedMessage = FString::Printf(TEXT("Current Game Speed: %d"), SimulationController.IsValid() ? 0 : static_cast<int>(SimGameController->CurrentGameSpeed));
	
    DrawText(StepMessage, TextColor, ScreenPosition.X, ScreenPosition.Y, nullptr, TextScale, true);
    DrawText(HumansMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 15.0f, nullptr, TextScale, true);
    DrawText(BittenMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 30.0f, nullptr, TextScale, true);
    DrawText(ZombiesMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 45.0f, nullptr, TextScale, true);
	DrawText(SpeedMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 60.0f, nullptr, TextScale, true);
	
	for (int i = 0; i < Buttons.Num(); i++)
	{
		const FHUDButton& Btn = Buttons[i];

		// Draw a clickable hitbox
		AddHitBox(Btn.Position, Btn.Size, *Btn.Label, true);

		// Draw button background
		DrawRect(FLinearColor(0, 0, 0, 0.5f), Btn.Position.X, Btn.Position.Y, Btn.Size.X, Btn.Size.Y);

		// Draw label
		DrawText(
			Btn.Label,
			FLinearColor::White,
			Btn.Position.X + 10.f,
			Btn.Position.Y + 5.f,
			nullptr,
			1.2f,
			false
		);
	}
}

void ASimulationHUD::NotifyHitBoxClick(FName BoxName)
{
	bIsClickHandled = true;
	
	Super::NotifyHitBoxClick(BoxName);
	
	if (!SimGameController.IsValid()) return;

	FString Button = BoxName.ToString();

	if (Button == "Pause")
	{
		SimGameController->PauseGame();
	}
	else if (Button == "Resume")
	{
		SimGameController->ResumeGame();
	}
	else if (Button == "1x")
	{
		SimGameController->SetGameSpeed(1.0f);
	}
	else if (Button == "2x")
	{
		SimGameController->SetGameSpeed(2.0f);
	}
	else if (Button == "4x")
	{
		SimGameController->SetGameSpeed(4.0f);
	}
}

void ASimulationHUD::NotifyHitBoxRelease(FName BoxName)
{
	Super::NotifyHitBoxRelease(BoxName);
	
	NotifyHitBoxClick(BoxName);
}

void ASimulationHUD::ResetClickHandling()
{
	bIsClickHandled = false;
}
