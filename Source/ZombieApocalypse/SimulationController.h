/// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SimulationController.generated.h"

/** 
 * @brief Struct for the Conveyor implementation. Used by SimulationController.
 * @details Takes in infected humans every timestep and counts down days remaining before zombified.
 */
USTRUCT(BlueprintType)
struct FBittenBatch
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Amount = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RemainingDays = 0.f;
};

/** 
 * @brief Struct for the Unreal DataTable. Used by SimulationHUD.
 * @details Keep variables UPROPERTY(EditAnywhere) with DataTables.
 */
USTRUCT(BlueprintType)
struct FPopulationDensityEffect : public FTableRowBase
{
	GENERATED_BODY()

	// X: population density sample point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DensityX = 0.f;
	
	// Y: bite multiplier / effect at that density
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BiteMultiplierY = 1.f;
};

UCLASS()
class ZOMBIEAPOCALYPSE_API ASimulationController : public AActor
{
	GENERATED_BODY()
	
public:	
	ASimulationController();
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override {}

	/// How many steps to run total 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Simulation")
	int32 MaxSteps = 120;

	/// ------ CONSTANTS ------
	
	/// Days for a bitten person to convert to a zombie
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulation Variables")
	float DaysToBecomeInfected = 15.f;
	
	/// Normal bites per zombie (used as base, scaled by density effect)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulation Variables")
	float NormalNumberOfBites = 1.f;
	
	/// Land area used to compute population density
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulation Variables")
	float LandArea = 1000.f;

	/// A "normal" population density used to normalize X when looking up the curve
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Simulation")
	float NormalPopulationDensity = 0.1f;
	
	/// Maximum capacity of the conveyor (Bitten_Two_capacity in Stella)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Simulation")
	float BittenTwoCapacity = 100.f;

	/// ------ SIMULATION STATE VARIABLES (read by the HUD) ------
	
	/// How many real seconds between calls to RunSimulationStep (controls realtime speed)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Simulation")
	float SimulationRealSecondsPerStep = 0.2f;

	/// How many simulation days pass per simulation step
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Simulation")
	float DaysPerStep = 1.0f;
	
	/// How many steps the simulation has completed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Simulation")
	int32 TimeStepsFinished = 0;
	
	/// Susceptible (Humans) - choose a number that has a clean sqrt!
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Simulation Variables")
	float Susceptible = 100.f;
	
	/// Zombies. Include Patient Zero! (1.f)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Simulation Variables")
	float Zombies = 1.f;	
	
	/// Just to check if we are correctly updating stocks - used in SimulationHUD
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulation Variables")
	float Bitten = 0.f;   

	/// ------ OTHER / REMAINING VARIABLES ------
	
	/// Unreal DataTable (soft pointer - loaded synchronously at BeginPlay)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Simulation Variables")
	TSoftObjectPtr<UDataTable> PopulationDensityEffectTable;
	
	/// Conveyor with bitten humans (future zombies)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation Variables")
	TArray<FBittenBatch> Conveyor;
	
	/// Turn on/off debug printing to the Output Log
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Simulation Variables")
	bool bShouldDebug = false;
	
	/// Vector of pairs filled by ReadDataFromTableToVectors() 
	/// (X = DensityX, Y = BiteMultiplierY)
	TArray<TPair<float, float>> GraphPts;

	/// Optional readouts useful for HUD or debugging (exposed as read-only)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation Debug")
	float LastGettingBitten = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation Debug")
	float LastOutflow = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation Debug")
	float LastNewlyInfected = 0.f;
	
private:
	/// Timer handle that triggers simulation steps
	FTimerHandle SimulationTimerHandle;
	
	/// Function to start running a timer with GetTimer()
	void StartSimulation();
	
	/// Deals with logic for every simulation step
	void RunSimulationStep();
	
	/// Function to stop the simulation - when MaxSteps are reached, or when there are no more humans.
	void StopSimulation();
	
	/// Function to read data from Unreal DataTable into the graphPts vector
	void ReadDataFromTableToVectors(const UDataTable* DT);

	/// Look up the bite multiplier from GraphPts for normalized X
	float GraphLookup(float X) const;
};
