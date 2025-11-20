// Copyright University of Inland Norway

#include "ZombieApocalypse/Public/Zombie.h"

#include "SimGameController.h"
#include "ZombieApocalypse/Public/Human.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AZombie::AZombie()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionComponent = GetCapsuleComponent();
	CollisionComponent->SetGenerateOverlapEvents(true);
	
	
}

// Called when the game starts or when spawned
void AZombie::BeginPlay()
{
	Super::BeginPlay();
	
	PreviousPosition3SecondsAgo = GetActorLocation();
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASimGameController::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		ASimGameController* CustomActor = Cast<ASimGameController>(FoundActors[0]);
		if (CustomActor)
		{
			GameController = CustomActor;
		}
	}
	
	// Zombies start idle – scanning
	CurrentTarget = nullptr;

	// Scan for humans every ScanInterval seconds
	GetWorldTimerManager().SetTimer(
		ScanTimer, this,
		&AZombie::ScanForHumans,
		ScanInterval, true
	);
	
	GetWorldTimerManager().SetTimer(OldPositionTimerHandle, this, &AZombie::UpdateOldTargetPosition, 3.f, true);
}

// Called every frame
void AZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentTarget.IsValid())
	{
		MoveTowardTarget();
	}
	else
	{
		ScanForHumans();
	}
	
	if (CurrentTarget.Get())
	{
		if (CurrentTarget.Get()->GetActorLocation() == PreviousPosition3SecondsAgo)
		{
			CurrentTarget = nullptr;
		}
	}
}

void AZombie::UpdateOldTargetPosition()
{
	PreviousPosition3SecondsAgo = GetActorLocation();
}

void AZombie::SetInitialZombie()
{
	// Called when spawned – this ensures zombies always begin scanning
	CurrentTarget = nullptr;
	
	if (GetWorldTimerManager().IsTimerActive( ScanTimer ))
	{
		GetWorldTimerManager().ClearTimer( ScanTimer );
	}
	// Scan for humans every ScanInterval seconds
	GetWorldTimerManager().SetTimer(
		ScanTimer, this,
		&AZombie::ScanForHumans,
		ScanInterval, true
	);
}

void AZombie::SetGameController(ASimGameController* InGameController)
{
	GameController = InGameController;
}

void AZombie::ScanForHumans()
{
	if (GameController->HumanActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No humans in ScanForHumans in Zombie.cpp! Game over?"));
		return;
	}

	// Find the closest human within SearchRadius

	
	if (AHuman* Closest = FindClosestHuman(GameController->HumanActors))
	{
		CurrentTarget = Closest;
		Closest->bIsTargeted = true;  // Prevents zombies from stacking the same target
		MoveTowardTarget();
	}
	else
	{
		// Expand the search radius until max
		SearchRadius = FMath::Min(SearchRadius + 200.f, MaxSearchRadius);
		// TODO: Use this SearchRadius to actually search for zombies with.
	}
}

AHuman* AZombie::FindClosestHuman(TArray<AActor*>& Humans) const
{
	AHuman* Result = nullptr;
	float BestDist = FLT_MAX;

	FVector MyPos = GetActorLocation();

	for (AActor* A : Humans)
	{
		AHuman* H = Cast<AHuman>(A);
		if (!H || !H->IsAlive() || H->bIsTargeted) continue;

		float Dist = FVector::Distance(MyPos, H->GetActorLocation());

		if (Dist < SearchRadius && Dist < BestDist)
		{
			BestDist = Dist;
			Result = H;
		}
	}

	return Result;
}

AHuman* AZombie::FindClosestHuman(TArray<AHuman*>& Humans) const
{
	AHuman* Result = nullptr;
	float BestDist = FLT_MAX;

	FVector MyPos = GetActorLocation();

	for (AActor* A : Humans)
	{
		AHuman* H = Cast<AHuman>(A);
		if (!H || !H->IsAlive() || H->bIsTargeted || H->bIsBitten) continue;

		float Dist = FVector::Distance(MyPos, H->GetActorLocation());

		if (Dist < SearchRadius && Dist < BestDist)
		{
			BestDist = Dist;
			Result = H;
		}
	}

	return Result;
}

void AZombie::MoveTowardTarget()
{
	if (!CurrentTarget.IsValid() || !CurrentTarget->IsAlive())
	{
		CurrentTarget = nullptr;
		return;
	}
	
	CurrentTarget->bIsTargeted = true;

	FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation());
	Direction.Z = 0;
	Direction.Normalize();

	AddMovementInput(Direction, ZombieSpeed);
}

void AZombie::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (AHuman* Human = Cast<AHuman>(OtherActor))
    {
    	UE_LOG(LogTemp, Warning, TEXT("Zombie %s bites human %s!"), *GetName(), *Human->GetName());
	    if (!Human->IsAlive()) return;
        
    	// Clear the target and find a new one on the next scan
    	CurrentTarget = nullptr;
    	;
    	FindClosestHuman(GameController->HumanActors);
    }
}

