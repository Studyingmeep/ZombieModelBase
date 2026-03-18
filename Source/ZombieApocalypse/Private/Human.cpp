// Copyright University of Inland Norway

#include "Human.h"
#include "SimGameController.h"
#include "SpawnVolume.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h" // Required for Pawn capsules!
#include "Zombie.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AHuman::AHuman()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ActorCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
	RootComponent = ActorCapsuleComponent;
	ActorCapsuleComponent->SetGenerateOverlapEvents(true);
	
	TargetArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("TargetArrow"));
	TargetArrow->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AHuman::BeginPlay()
{
	Super::BeginPlay();
	
	WalkingSpeed = FMath::RandRange(100.f, 300.f);
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASimGameController::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		if (ASimGameController* CustomActor = Cast<ASimGameController>(FoundActors[0]))
		{
			GameController = CustomActor;
		}
	}
}

// Called every frame
void AHuman::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If they are dead or incubating, they shouldn't be walking!
	if (!bAlive || bIsBitten) return;

	const FVector MyLoc = GetActorLocation();
	FVector FleeDir = FVector::ZeroVector;
	bool bIsPanicking = false;

	// 1. Scan for nearby zombies to run away from
	// (Make sure GameController and ZombieActors exist/are accessible!)
	if (GameController)
	{
		for (AActor* Actor : GameController->ZombieActors)
		{
			if (const AZombie* Z = Cast<AZombie>(Actor))
			{
				if (FVector::Distance(MyLoc, Z->GetActorLocation()) < 500.f)
				{
					FleeDir += (MyLoc - Z->GetActorLocation());
					bIsPanicking = true;
				}
			}
		}
	}

	// 2. Choose the movement state
	if (bIsPanicking)
	{
		// Sprint away from the horde
		FleeDir.Z = 0;
		FleeDir.Normalize();
		// 500.f is the speed multiplier. Multiply by DeltaTime so it stays smooth regardless of frame rate!
		AddActorWorldOffset(FleeDir * 400.f * DeltaTime, true);
	}
	else
	{
		CurrentWanderTimer -= DeltaTime;
		if (CurrentWanderTimer <= 0.f)
		{
			// Pick a random 2D direction
			WanderDirection = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 0.f);
			WanderDirection.Normalize();
			CurrentWanderTimer = FMath::RandRange(MinWanderTimer, MaxWanderTimer);
		}
        
		// 200.f is the slow walking speed. 
		AddActorWorldOffset(WanderDirection * 200.f * DeltaTime, true);
		
		if (GameController && GameController->SpawnVolumeActor)
		{
			const FVector Center = GameController->SpawnVolumeActor->GetActorLocation();

			// If they wander more than 1000 units away from the center of the arena...
			if (const FVector MyCurrentLocation = GetActorLocation(); FVector::Distance(MyCurrentLocation, Center) > 3500.f) 
			{
				// Calculate the direction back to the exact center
				FVector DirectionToCenter = (Center - MyCurrentLocation);
				DirectionToCenter.Z = 0;
				DirectionToCenter.Normalize();
			
				// Override their wander direction so they immediately walk back inside
				WanderDirection = DirectionToCenter; 
				
				// THE FIX: Reset the timer to 1 second so they scatter again after bumping the wall!
				CurrentWanderTimer = 1.0f;
			}
		}
	}
	
}

void AHuman::GetBitten()
{
	if (bIsBitten)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Human %s is bitten!"), *GetName());
	bIsTargeted = false;
	bIsBitten = true;
	
	SetActorEnableCollision(false);
	ActorCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TargetArrow->SetVisibility(false);
	TargetArrow->SetHiddenInGame(true);
	
	GetWorldTimerManager().SetTimer(InfectionTimer, this,  &AHuman::ReduceDaysLeftUntilZombie, 1.f / GameController->CurrentGameSpeed, true);
}

void AHuman::ReduceDaysLeftUntilZombie()
{
	DaysUntilZombie--;
	if (DaysUntilZombie <= 0)
	{
		TurnIntoZombie();
	}
}

void AHuman::TurnIntoZombie()
{
	GameController->NotifyHumanConverted(this);
	bAlive = false;
	bIsBitten = false;
	bIsTargeted = false;
	Destroy();
}

void AHuman::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	if (!Cast<AZombie>(OtherActor) || bIsBitten) return;
	GetBitten();
}

