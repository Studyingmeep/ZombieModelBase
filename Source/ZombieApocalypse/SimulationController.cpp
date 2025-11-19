#include "SimulationController.h"

ASimulationController::ASimulationController()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASimulationController::BeginPlay()
{
    Super::BeginPlay();
    
    UDataTable* LoadedTable = PopulationDensityEffectTable.LoadSynchronous();
    
    // Checking if the DataTable is assigned
    if (!LoadedTable)
    {
        UE_LOG(LogTemp, Error, TEXT("PopulationDensityEffectTable FAILED to load!"));
    }
    else
    {
        // Table found, read data into vector
        ReadDataFromTableToVectors(LoadedTable);
        UE_LOG(LogTemp, Warning, TEXT("PopulationDensityEffectTable loaded with %d rows"), LoadedTable->GetRowNames().Num());
    }
    
    StartSimulation();
}

void ASimulationController::StartSimulation()
{
    GetWorldTimerManager().SetTimer(
        SimulationTimerHandle,
        this,
        &ASimulationController::RunSimulationStep,
        SimulationRealSecondsPerStep,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Simulation started using timers."));
}

void ASimulationController::StopSimulation()
{
    GetWorldTimerManager().ClearTimer(SimulationTimerHandle);
    UE_LOG(LogTemp, Warning, TEXT("Simulation stopped after %d steps"), TimeStepsFinished);
}

void ASimulationController::RunSimulationStep()
{
    // Stop if finished
    if (TimeStepsFinished >= MaxSteps)
    {
        StopSimulation();
        return;
    }

    TimeStepsFinished++;

    // const float DT = 1.f; 1 day per simulation step (or your own)

    // ------ STEP 1: Compute current BittenTwo ------
    float BittenTwo = 0.f;
    for (const auto& B : Conveyor) 
    {
       BittenTwo += B.Amount;
    }

    // TODO: Check if this below logic actually makes sense with the DataTable and all.

    const float NonZombiePop = Susceptible + BittenTwo;
    const float PopDensity = NonZombiePop / LandArea;

    const FPopulationDensityEffect PopData;
    const float X = PopDensity / PopData.BiteMultiplierY;
    
    const float PopDensityEffect = GraphLookup(X);

    // ------ STEP 2: How many people bitten today? ------
    const float BitesPerZombie = NormalNumberOfBites * PopDensityEffect;
    const float TotalBitten = Zombies * BitesPerZombie;

    // Scale to how many of those bites hit Susceptible
    const float Denominator = FMath::Max(NonZombiePop, 1.f);
    float GettingBitten = (Susceptible / Denominator) * TotalBitten;

    // Clamp so we never bite more people than exist
    GettingBitten = FMath::Min(GettingBitten, FMath::FloorToFloat(Susceptible));

    // ------ STEP 3: Advance conveyor ------
    float OutflowPeople = 0.f;

    for (auto& B : Conveyor)
    {
        B.RemainingDays -= DaysPerStep;
    }

    // Remove finished batches
    for (int32 i = Conveyor.Num() - 1; i >= 0; i--)
    {
        if (Conveyor[i].RemainingDays <= 0.f)
        {
            OutflowPeople += Conveyor[i].Amount;
            Conveyor.RemoveAt(i);
        }
    }

    // ------ STEP 4: Add new bitten to conveyor (respecting capacity) ------
    float CurrentContent = 0.f;
    for (auto& B : Conveyor) CurrentContent += B.Amount;

    if (GettingBitten > 0.f)
    {
        FBittenBatch NewBatch;
        NewBatch.Amount = GettingBitten;
        NewBatch.RemainingDays = DaysToBecomeInfected;
        Conveyor.Add(NewBatch);
    }

    // ------ STEP 5: Outflow becomes zombies, conversion 1:1 default ------
    float BecomingZombies = OutflowPeople; 

    // ------ STEP 6: Update stocks ------
    Susceptible = FMath::Max(0.f, Susceptible - GettingBitten);
    Zombies = FMath::Max(0.f, Zombies + BecomingZombies);

    // ------ STEP 7: Update your Bitten variable for HUD ------
    float NewBittenTwo = 0.f;
    for (auto& B : Conveyor) NewBittenTwo += B.Amount;
    Bitten = NewBittenTwo;

    if (bShouldDebug)
    {
        UE_LOG(LogTemp, Log, TEXT("Step %d: S=%f, Bitten=%f, Z=%f"), TimeStepsFinished, Susceptible, Bitten, Zombies);
    }
    
    if (Susceptible <= 0)
    {
        StopSimulation();
    }
}

// Function to read data from Unreal DataTable into the graphPts vector
void ASimulationController::ReadDataFromTableToVectors(const UDataTable* DT)
{
    GraphPts.Empty();
    
    for (const auto& Name : DT->GetRowNames())
    {
        if (FPopulationDensityEffect* Row = DT->FindRow<FPopulationDensityEffect>(Name, ""))
        {
            GraphPts.Emplace(Row->DensityX, Row->BiteMultiplierY);
        }
    }

    // Quickly sort through the data via X key values if variables are not sorted.
    GraphPts.Sort([](const TPair<float, float>& A, const TPair<float, float>& B) {
        return A.Key < B.Key;
    });
}

float ASimulationController::GraphLookup(const float X)
{
    if (GraphPts.Num() == 0) return 1.f;

    if (X <= GraphPts[0].Key) return GraphPts[0].Value;
    if (X >= GraphPts.Last().Key) return GraphPts.Last().Value;

    for (int i = 0; i < GraphPts.Num() - 1; i++)
    {
        float X1 = GraphPts[i].Key;
        float X2 = GraphPts[i+1].Key;

        if (X >= X1 && X <= X2)
        {
            float Y1 = GraphPts[i].Value;
            float Y2 = GraphPts[i+1].Value;

            float Alpha = (X - X1) / (X2 - X1);
            return FMath::Lerp(Y1, Y2, Alpha);
        }
    }

    return GraphPts.Last().Value;
}
 