// Copyright University of Inland Norway

#include "ZombieApocalypse/Public/SimGameController.h"

// Sets default values
ASimGameController::ASimGameController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASimGameController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASimGameController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

