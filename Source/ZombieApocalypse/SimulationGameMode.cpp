// Copyright University of Inland Norway

#include "SimulationGameMode.h"
#include "CustomPlayerController.h"
#include "SimulationHUD.h"

ASimulationGameMode::ASimulationGameMode()
{
	HUDClass = ASimulationHUD::StaticClass();
	PlayerControllerClass = ACustomPlayerController::StaticClass();
}
