// Copyright University of Inland Norway

#include "ZombieApocalypse/Public/Human.h"

// Sets default values
AHuman::AHuman()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHuman::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHuman::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHuman::SetTargeted(const bool bTarget)
{
	bIsTargeted = bTarget;
	TargetMesh->SetVisibility(bIsTargeted);
}

