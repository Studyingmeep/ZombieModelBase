// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "CustomPlayerController.generated.h"

class ASimGameController;
/**
 * 
 */
UCLASS()
class ZOMBIEAPOCALYPSE_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputMappingContext* IMC_Player;
	 
	// Input Action asset reference for left click
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_Leftclick;
	
	void HandleLeftClick(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game")
	ASimGameController* GameController;
};
