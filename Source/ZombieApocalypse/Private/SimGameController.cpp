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

	// Spawn the total population (100 humans + 1 who will become the zombie)
	for (int i = 0; i < Susceptible + 1; i++) 
	{
		const FVector Pos = SpawnVolumeActor->GetRandomPoint();
		SpawnEntityAt(Pos, true);
	}
	
	// Patient Zero Selection
	if (HumanActors.Num() > 0)
	{
		// 1. Pick a random index from the human array
		const int32 RandomIndex = FMath::RandRange(0, HumanActors.Num() - 1);

		if (AHuman* UnluckyHuman = Cast<AHuman>(HumanActors[RandomIndex]))
		{
			// 2. Save their exact location
			const FVector PatientZeroLocation = UnluckyHuman->GetActorLocation();
            
			// 3. Remove them from the human array and destroy the actor
			HumanActors.RemoveAt(RandomIndex);
			UnluckyHuman->Destroy();
            
			// 4. Spawn the zombie in that exact spot (SpawnEntityAt handles the Zombie array!)
			SpawnEntityAt(PatientZeroLocation, false);
		}
	}
	// ------------------------------
	
	GetWorldTimerManager().SetTimer(DayTimer, this, &ASimGameController::AdvanceDay, SecondsPerDay, true);
}

void ASimGameController::SpawnEntityAt(const FVector Position, const bool bSpawnHuman)
{
	FVector SafeSpawnPos = Position + FVector(0.f, 0.f, 150.f);
	
	if (bSpawnHuman && HumanClass)
	{
		AHuman* NewHuman = GetWorld()->SpawnActor<AHuman>(HumanClass, SafeSpawnPos, FRotator::ZeroRotator);
		HumanActors.Add(NewHuman);
	}
	else if (!bSpawnHuman && ZombieClass)
	{
		if (AZombie* NewZombie = GetWorld()->SpawnActor<AZombie>(ZombieClass, SafeSpawnPos + FVector(0, 0, 100), FRotator::ZeroRotator))
		{
			NewZombie->SetGameController(this);
			NewZombie->SetInitialZombie();
			Zombies++;
			ZombieActors.Add(NewZombie);
		}
	}
}

void ASimGameController::PauseGame()
{
	UGameplayStatics::SetGamePaused(this, true);
}

void ASimGameController::ResumeGame()
{
	UGameplayStatics::SetGamePaused(this, false);
}

void ASimGameController::SetGameSpeed(const float Multiplier)
{
	CurrentGameSpeed = Multiplier;
	SecondsPerDay = 1.0f / Multiplier;

	// Restart the timer with the new interval
	if (GetWorldTimerManager().IsTimerActive(DayTimer))
	{
		GetWorldTimerManager().ClearTimer(DayTimer);
	}

	GetWorldTimerManager().SetTimer(
		DayTimer,
		this,
		&ASimGameController::AdvanceDay,
		SecondsPerDay,
		true
	);
	
	// ⭐ The magic line — speeds up animations, movement, everything
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), CurrentGameSpeed);
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
		Susceptible--;
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
	
	PauseGame();
}

void ASimGameController::RescueHuman(AHuman* HumanToRescue)
{
	// Double-check that the human is valid and still in the active array
	if (HumanToRescue && HumanActors.Contains(HumanToRescue))
	{
		// 1. Increase the score and reduce humans available!
		RescuedHumans++;
		Susceptible--;
        
		// 2. Remove them from the simulation math
		HumanActors.Remove(HumanToRescue);
        
		// 3. Delete the actor safely from the world
		HumanToRescue->Destroy();
        
		UE_LOG(LogTemp, Warning, TEXT("Human Rescued! Total Saved: %d"), RescuedHumans);
		
		// 4. Check Win Condition
		if (HumanActors.Num() == 0)
		{
			StopGame();
			UGameplayStatics::SetGamePaused(GetWorld(), true);
			TriggerEndScreen(true, RescuedHumans); // Pops the Victory UI!
		}
	}
}

void ASimGameController::PlayerCaught()
{
	StopGame(); // Pauses the simulation math
	UGameplayStatics::SetGamePaused(GetWorld(), true); // Freezes the engine
	TriggerEndScreen(false, RescuedHumans); // Pops the Defeat UI!
}
