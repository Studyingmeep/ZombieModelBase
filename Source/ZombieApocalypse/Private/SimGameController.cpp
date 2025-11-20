// Copyright University of Inland Norway

#include "ZombieApocalypse/Public/SimGameController.h"

#include "Kismet/GameplayStatics.h"
#include "ZombieApocalypse/Public/Human.h"
#include "ZombieApocalypse/Public/SpawnVolume.h"
#include "ZombieApocalypse/Public/Zombie.h"

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

	if (!SpawnVolumeActor)
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), Found);

		if (Found.Num() > 0)
		{
			SpawnVolumeActor = Cast<ASpawnVolume>(Found[0]);
			if (!SpawnVolumeActor)
			{
				UE_LOG(LogTemp, Error, TEXT("Found actor of class ASpawnVolume but cast failed."));
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SpawnVolumeActor is not assigned in the editor!"));
			return;
		}
	}

	// Spawn humans
	for (int i = 0; i < Susceptible; i++)
	{
		FVector Pos = SpawnVolumeActor->GetRandomPoint();
		SpawnEntityAt(Pos, true);
	}
	
	GetWorldTimerManager().SetTimer(DayTimer, this, &ASimGameController::AdvanceDay, SecondsPerDay, true);
}

void ASimGameController::SpawnEntityAt(const FVector Position, const bool bSpawnHuman)
{
	if (bSpawnHuman && HumanClass)
	{
		AHuman* NewHuman = GetWorld()->SpawnActor<AHuman>(HumanClass, Position, FRotator::ZeroRotator);
		HumanActors.Add(NewHuman);
	}
	else if (!bSpawnHuman && ZombieClass)
	{
		AZombie* NewZombie = GetWorld()->SpawnActor<AZombie>(ZombieClass, Position, FRotator::ZeroRotator);
		ZombieActors.Add(NewZombie);
	}
}

void ASimGameController::AdvanceDay()
{
	if (CurrentDay >= MaxDays)
	{
		StopGame();
		return;
	}

	CurrentDay++;

	// If no humans left (including bitten humans) → the game ends
	if (HumanActors.Num() == 0)
	{
		StopGame();
	}
}

void ASimGameController::NotifyHumanConverted(AHuman* HumanVictim)
{
	if (HumanActors.Contains(HumanVictim))
	{
		HumanActors.Remove(HumanVictim);
	}

	// Spawn zombie at the same spot where human 
	SpawnEntityAt(HumanVictim->GetActorLocation(), false);
}

void ASimGameController::StopGame()
{
	if (GetWorldTimerManager().IsTimerActive(DayTimer))
	{
		GetWorldTimerManager().ClearTimer(DayTimer);
	}
}
