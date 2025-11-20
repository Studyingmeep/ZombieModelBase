// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimGameController.generated.h"

class ASpawnVolume;
class AZombie;
class AHuman;

UCLASS()
class ZOMBIEAPOCALYPSE_API ASimGameController : public AActor
{
	GENERATED_BODY()
	
	/// For handling day advances after real-life seconds defined by SecondsPerDay.
	FTimerHandle DayTimer;
	
	

public:	
	// Sets default values for this actor's properties
	ASimGameController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void AdvanceDay();
	void StopGame();

public:
	
	/// Spawn a human or zombie at location
	UFUNCTION(BlueprintCallable)
	void SpawnEntityAt(FVector Position, bool bSpawnHuman);
	
	/// Called when a human is converted
	void NotifyHumanConverted(AHuman* HumanVictim);
	
	UFUNCTION(BlueprintCallable)
	void PauseGame();

	UFUNCTION(BlueprintCallable)
	void ResumeGame();

	UFUNCTION(BlueprintCallable)
	void SetGameSpeed(float Multiplier);
	
	// --- UPROPERTIES ---
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup")
	ASpawnVolume* SpawnVolumeActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup")
	TSubclassOf<AHuman> HumanClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup")
	TSubclassOf<AZombie> ZombieClass;
	
	/// Lists of actors for game logic
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AHuman*> HumanActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AZombie*> ZombieActors;
	
	bool bPatientZeroSpawned = false;

	/// How many real-life seconds pass for a day to be done?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation")
	float SecondsPerDay = 1.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Simulation")
	float CurrentGameSpeed = 1.0f;
	
	/// How many days (steps) the simulation has completed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Simulation")
	int32 CurrentDay = 0;
	
	/// How many days (steps) to run total 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Simulation")
	int32 MaxDays = 120; // 2 mins at 1 day/sec
	
	/// Susceptible (Humans) - choose a number that has a clean sqrt!
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Simulation Variables")
	float Susceptible = 100.f;
	
	/// Zombies.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Simulation Variables")
	float Zombies = 0.f;	
	
	/// Just to check if we are correctly updating stocks - used in SimulationHUD
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulation Variables")
	float Bitten = 0.f;  
	
	// Have we paused the game?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation")
	bool bIsPaused = false;
	
	// Enable debug lines between zombie and current target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation")
	bool bDebugLinesEnabled = false;

public:
	
	
	
	
};
