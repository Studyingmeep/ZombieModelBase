// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SimulationHUD.generated.h"

USTRUCT()
struct FHUDButton
{
	GENERATED_BODY()

	FString Label;
	FVector2D Position;
	FVector2D Size;
};

class ASimGameController;
class ASimulationController;

/**
 * 
 */
UCLASS()
class ZOMBIEAPOCALYPSE_API ASimulationHUD : public AHUD
{
	GENERATED_BODY()


public:
	
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;
	virtual void NotifyHitBoxClick(FName BoxName) override;
	virtual void NotifyHitBoxRelease(FName BoxName) override;
	void ResetClickHandling();

	TArray<FHUDButton> Buttons;
	
	bool bIsClickHandled = false;

private:
	
	TWeakObjectPtr<ASimulationController> SimulationController;
	TWeakObjectPtr<ASimGameController> SimGameController;;

};
