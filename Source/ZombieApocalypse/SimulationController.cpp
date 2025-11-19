#include "SimulationController.h"
#include "TimerManager.h"
#include "Engine/World.h"

ASimulationController::ASimulationController()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASimulationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Try to synchronously load the DataTable (soft pointer -> UDataTable*)
    const UDataTable* LoadedTable = nullptr;
    if (PopulationDensityEffectTable.IsValid())
    {
        LoadedTable = PopulationDensityEffectTable.Get();
    }
    else
    {
        LoadedTable = PopulationDensityEffectTable.LoadSynchronous();
    }

    if (!LoadedTable)
    {
        UE_LOG(LogTemp, Error, TEXT("PopulationDensityEffectTable FAILED to load! Make sure it's assigned in the BP."));
    }
    else
    {
        ReadDataFromTableToVectors(LoadedTable);
        UE_LOG(LogTemp, Warning, TEXT("PopulationDensityEffectTable loaded with %d rows"), LoadedTable->GetRowNames().Num());
    }
    
    StartSimulation();
}

void ASimulationController::StartSimulation()
{
    // Avoid invalid timer interval
    const float Interval = FMath::Max(0.001f, SimulationRealSecondsPerStep);

    GetWorldTimerManager().SetTimer(
        SimulationTimerHandle,
        this,
        &ASimulationController::RunSimulationStep,
        Interval,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Simulation started (interval=%.3f sec)."), Interval);
}

void ASimulationController::StopSimulation()
{
    if (GetWorldTimerManager().IsTimerActive(SimulationTimerHandle))
    {
        GetWorldTimerManager().ClearTimer(SimulationTimerHandle);
    }
    UE_LOG(LogTemp, Warning, TEXT("Simulation stopped after %d steps."), TimeStepsFinished);
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
    
    // Use DaysPerStep as the simulation dt (Stella uses DT = 1)
    const float DT = DaysPerStep;

    // ------ STEP 1: Compute current BittenTwo (sum of conveyor content) ------
    float BittenTwo = 0.f;
    for (const auto& B : Conveyor)
    {
        BittenTwo += B.Amount;
    }

    // Current non-zombie population (Susceptible + those on conveyor)
    const float NonZombiePop = Susceptible + BittenTwo;

    // Population density (people per land area)
    const float PopDensity = (LandArea > 0.f) ? (NonZombiePop / LandArea) : 0.f;

    // Normalize the X value used for graph lookup
    const float NormalDen = (NormalPopulationDensity > 0.f) ? NormalPopulationDensity : 1.f;
    const float X = PopDensity / NormalDen;

    // Lookup density multiplier from table (interpolated)
    const float PopDensityEffect = GraphLookup(X);

    // ------ STEP 2: How many people bitten this step? ------
    // Bites per zombie scaled by density multiplier
    const float BitesPerZombie = NormalNumberOfBites * PopDensityEffect;
    const int32 TotalBitten = FMath::RoundToInt(Zombies * BitesPerZombie);

    // Scale to how many of those bites hit Susceptible (proportional)
    const float Denominator = FMath::Max(NonZombiePop, 1.f);
    
    // Stella: number_of_bites_from_total_zombies_on_susceptible = round((Susceptible / denom) * total_bitten_per_day)
    const int32 BitesOnSusceptible = FMath::RoundToInt((Susceptible / Denominator) * (float)TotalBitten);
    
    float GettingBitten = static_cast<float>(BitesOnSusceptible);
    // Clamp so we don't bite more people than exist; original code used floor to get integer-like behaviour
    GettingBitten = FMath::Min(GettingBitten, FMath::FloorToFloat(Susceptible));

    // ------ record debug / HUD readouts ------
    LastGettingBitten = GettingBitten;

    // ------ STEP 3: Advance conveyor (countdown) ------
    for (auto& B : Conveyor)
    {
        B.RemainingDays -= DT;
    }

    // Remove finished batches and collect how many become zombies this step
    float OutflowPeople = 0.f;
    for (int32 i = Conveyor.Num() - 1; i >= 0; --i)
    {
        if (Conveyor[i].RemainingDays <= 0.f)
        {
            OutflowPeople += Conveyor[i].Amount;
            Conveyor.RemoveAt(i);
        }
    }
    LastOutflow = OutflowPeople;

    // ------ STEP 4: Add new bitten to conveyor ------
    float CurrentContent = 0.f;
    for (const auto& B : Conveyor) CurrentContent += B.Amount;

    const float FreeCap = FMath::Max(0.f, BittenTwoCapacity - CurrentContent);
    const float InflowPeople = FMath::Max(0.f, FMath::Min(GettingBitten, FreeCap));
    
    if (InflowPeople > 0.f)
    {
        FBittenBatch NewBatch;
        NewBatch.Amount = GettingBitten;
        NewBatch.RemainingDays = DaysToBecomeInfected;
        Conveyor.Add(NewBatch);
    }

    // ------ STEP 5: Outflow becomes zombies, conversion 1:1 default ------
    const float BecomingZombies = OutflowPeople * 1.f; // CONVERSION_FROM_PEOPLE_TO_ZOMBIES = 1
    LastNewlyInfected = BecomingZombies;

    // ------ STEP 6: Update stocks (Susceptible & Zombies) ------
    Susceptible = FMath::Max(0.f, Susceptible - GettingBitten * DT);
    Zombies = FMath::Max(0.f, Zombies + BecomingZombies * DT);

    // ------ STEP 7: Update Bitten for HUD (sum of conveyor) ------
    float NewBittenTwo = 0.f;
    for (const auto& B : Conveyor) NewBittenTwo += B.Amount;
    Bitten = NewBittenTwo;

    // Debug log
    if (bShouldDebug)
    {
        UE_LOG(LogTemp, Log, TEXT(
            "Step %d | PopDensity=%.4f X=%.4f Mult=%.4f TotalBitten=%d GettingBitten=%.3f Outflow=%.3f NewZ=%.3f S=%.3f Z=%.3f BittenOnConv=%.3f"
        ), TimeStepsFinished, PopDensity, X, PopDensityEffect, TotalBitten, GettingBitten, OutflowPeople, BecomingZombies, Susceptible, Zombies, Bitten);
    }
    
    // If no susceptible remain, stop early
    if (Susceptible <= 0.f && Bitten <= 0.f)
    {
        StopSimulation();
    }
}

// Function to read data from Unreal DataTable into the graphPts vector
void ASimulationController::ReadDataFromTableToVectors(const UDataTable* DT)
{
    GraphPts.Empty();

    if (!DT) return;

    const TArray<FName> RowNames = DT->GetRowNames();
    GraphPts.Reserve(RowNames.Num());

    for (const auto& Name : RowNames)
    {
        if (const FPopulationDensityEffect* Row = DT->FindRow<FPopulationDensityEffect>(Name, ""))
        {
            GraphPts.Emplace(Row->DensityX, Row->BiteMultiplierY);
        }
    }

    // Sort by X to ensure interpolation works even if the table wasn't sorted
    GraphPts.Sort([](const TPair<float, float>& A, const TPair<float, float>& B) {
        return A.Key < B.Key;
    });
}

float ASimulationController::GraphLookup(float X) const
{
    if (GraphPts.Num() == 0) return 1.f; // default multiplier

    // If X is out of range, clamp to min/max Y
    if (X <= GraphPts[0].Key) return GraphPts[0].Value;
    if (X >= GraphPts.Last().Key) return GraphPts.Last().Value;

    // Linear interpolation between the two surrounding points
    for (int32 i = 0; i < GraphPts.Num() - 1; ++i)
    {
        const float X1 = GraphPts[i].Key;
        const float X2 = GraphPts[i + 1].Key;

        if (X >= X1 && X <= X2)
        {
            const float Y1 = GraphPts[i].Value;
            const float Y2 = GraphPts[i + 1].Value;
            const float Alpha = (X2 - X1) > 0.f ? (X - X1) / (X2 - X1) : 0.f;
            return FMath::Lerp(Y1, Y2, Alpha);
        }
    }

    return GraphPts.Last().Value;
}