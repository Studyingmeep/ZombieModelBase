// Copyright University of Inland Norway

#include "Human.h"
#include "SimGameController.h"
#include "Components/ArrowComponent.h"
#include "Zombie.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AHuman::AHuman()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	ActorCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	ActorCapsuleComponent->SetupAttachment(RootComponent);
	ActorCapsuleComponent->SetGenerateOverlapEvents(true);
	
	TargetArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("TargetArrow"));
	TargetArrow->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AHuman::BeginPlay()
{
	Super::BeginPlay();
	
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
	ActorCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TargetArrow->SetVisibility(false);
	TargetArrow->SetHiddenInGame(true);
	
	GetWorldTimerManager().SetTimer(InfectionTimer, this,  &AHuman::ReduceDaysLeftUntilZombie, 1.f / GameController->CurrentGameSpeed, true);
}

void AHuman::ReduceDaysLeftUntilZombie()
{
	UE_LOG(LogTemp, Warning, TEXT("Human %s is infected for %i days left!"), *GetName(), DaysUntilZombie);
	DaysUntilZombie--;
	if (DaysUntilZombie == 0 || DaysUntilZombie < 0)
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

	if (Cast<AHuman>(OtherActor))
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Human %s is overlapping with %s inside AHuman:ActorOverlap!"), *GetName(), *OtherActor->GetName());
	
	if (bIsBitten) return;
	UE_LOG(LogTemp, Warning, TEXT("Human %s is bitten by %s inside AHuman:ActorOverlap!"), *GetName(), *OtherActor->GetName());
	GetBitten();
}

