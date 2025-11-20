// Copyright University of Inland Norway


#include "ZombieApocalypse/Public/SpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	RootComponent = Box;
	
	// Optionally set defaults for editor visualization
	Box->SetBoxExtent(FVector(500.f, 500.f, 100.f)); // default 1000m^2 footprint
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

FVector ASpawnVolume::GetRandomPoint() const
{
	if (Box != nullptr && Box->IsRegistered())
	{
		const FVector Origin = Box->Bounds.Origin;
		const FVector Extent = Box->Bounds.BoxExtent;
		return UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
	}
	
	// Fallback: use actor bounds (works even if Box is missing)
	const FBox ActorBox = GetComponentsBoundingBox(true);
	if (ActorBox.IsValid)
	{
		return UKismetMathLibrary::RandomPointInBoundingBox(ActorBox.GetCenter(), ActorBox.GetExtent());
	}

	// Very safe fallback
	UE_LOG(LogTemp, Warning, TEXT("SpawnVolume::GetRandomPoint fallback used (no box bounds). Returning actor location."));
	return GetActorLocation();
}

