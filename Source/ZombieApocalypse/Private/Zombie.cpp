// Copyright University of Inland Norway

#include "ZombieApocalypse/Public/Zombie.h"
#include "Human.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AZombie::AZombie()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionComponent = GetCapsuleComponent();
	CollisionComponent->SetGenerateOverlapEvents(true);
	
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AZombie::OnOverlapBegin);
}

// Called when the game starts or when spawned
void AZombie::BeginPlay()
{
	Super::BeginPlay();
	
	// Zombies start idle – scanning
	CurrentTarget = nullptr;

	// Scan for humans every ScanInterval seconds
	GetWorldTimerManager().SetTimer(
		ScanTimer, this,
		&AZombie::ScanForHumans,
		ScanInterval, true
	);
	
}

// Called every frame
void AZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentTarget.IsValid())
	{
		MoveTowardTarget();
	}
}

void AZombie::SetInitialZombie()
{
	// Called when spawned – this ensures zombies always begin scanning
	CurrentTarget = nullptr;
}

void AZombie::ScanForHumans()
{
	if (CurrentTarget.IsValid()) return;  // Already chasing someone

	TArray<AActor*> FoundHumans;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHuman::StaticClass(), FoundHumans);

	if (FoundHumans.Num() == 0) return;

	// Find the closest human within SearchRadius

	if (AHuman* Closest = FindClosestHuman(FoundHumans))
	{
		CurrentTarget = Closest;
		Closest->bIsTargeted = true;  // Prevents zombies from stacking the same target
	}
	else
	{
		// Expand the search radius until max
		SearchRadius = FMath::Min(SearchRadius + 200.f, MaxSearchRadius);
	}
}

AHuman* AZombie::FindClosestHuman(const TArray<AActor*>& Humans) const
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

void AZombie::MoveTowardTarget()
{
	if (!CurrentTarget.IsValid() || !CurrentTarget->IsAlive())
	{
		if (CurrentTarget.IsValid())
			CurrentTarget->bIsTargeted = false;

		CurrentTarget = nullptr;
		return;
	}

	FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation());
	Direction.Z = 0;
	Direction.Normalize();

	AddMovementInput(Direction, ZombieSpeed);
}

void AZombie::OnOverlapBegin(UPrimitiveComponent* Overlapped, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Zombie %s is overlapping with %s"), *GetName(), *Overlapped->GetOwner()->GetName());
	
	if (Overlapped->GetOwner() == Cast<AHuman>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Zombie %s bites %s"), *GetName(), *OtherActor->GetName());
		AHuman* Human = Cast<AHuman>(OtherActor);
		if (!Human->IsAlive()) return;

		// Bite human!
		Human->GetBitten();

		// Clear the target and find a new one on the next scan
		CurrentTarget = nullptr;
	}
}

