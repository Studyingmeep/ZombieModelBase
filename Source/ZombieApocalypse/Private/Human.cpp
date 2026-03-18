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

    // --- THE HUMAN MAGNET (Follow Player) ---
    if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
       // If the human is within 750 units of the player, they are "Collected"
       if (FVector::Distance(GetActorLocation(), PlayerPawn->GetActorLocation()) < 550.f)
       {
          FVector DirToPlayer = (PlayerPawn->GetActorLocation() - GetActorLocation());
          DirToPlayer.Z = 0;
          DirToPlayer.Normalize();
          
          // Rotate to face the player!
          if (!DirToPlayer.IsNearlyZero())
          {
              SetActorRotation(DirToPlayer.Rotation());
          }
          
          // Follow the player slightly slower than the player's run speed
          AddActorWorldOffset(DirToPlayer * 625.f * DeltaTime, true);
          
          // IMPORTANT: Return here so they ignore the Zombie panic and Wander logic!
          return; 
       }
    }
    
    const FVector MyLoc = GetActorLocation();
    FVector FleeDir = FVector::ZeroVector;
    bool bIsPanicking = false;

    // 1. Scan for nearby zombies to run away from
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
       
       // Rotate to face the escape route!
       if (!FleeDir.IsNearlyZero())
       {
           SetActorRotation(FleeDir.Rotation());
       }
       
       // 400.f is the speed multiplier
       AddActorWorldOffset(FleeDir * 450.f * DeltaTime, true);
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
       
       // 3. The Electric Fence (Check bounds BEFORE we move!)
       if (GameController && GameController->SpawnVolumeActor)
       {
          const FVector Center = GameController->SpawnVolumeActor->GetActorLocation();

          // If they wander more than 3500 units away from the center of the arena...
          if (FVector::Distance(MyLoc, Center) > 3500.f) 
          {
             // Calculate the direction back to the exact center
             FVector DirectionToCenter = (Center - MyLoc);
             DirectionToCenter.Z = 0;
             DirectionToCenter.Normalize();
          
             // Override their wander direction so they immediately walk back inside
             WanderDirection = DirectionToCenter; 
             
             // Reset the timer to 1 second so they scatter again after bumping the wall!
             CurrentWanderTimer = 1.0f;
          }
       }
        
       // Rotate to face the wandering direction!
       if (!WanderDirection.IsNearlyZero())
       {
           SetActorRotation(WanderDirection.Rotation());
       }
       
       // 200.f is the slow walking speed. 
       AddActorWorldOffset(WanderDirection * WalkingSpeed * DeltaTime, true);
    }
	
	// Fake Gravity: Constantly pull them down! (The floor collision will stop them)
	AddActorWorldOffset(FVector(0.f, 0.f, -500.f * DeltaTime), true);
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

