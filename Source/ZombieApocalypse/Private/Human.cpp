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
	bIsTargeted = false;
	bIsBitten = true;
	ActorCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TargetArrow->SetVisibility(false);
	TargetArrow->SetHiddenInGame(true);
	
	GetWorldTimerManager().SetTimer(InfectionTimer, this,  &AHuman::TurnIntoZombie, 15.f, false);
}

void AHuman::TurnIntoZombie()
{
	GameController->NotifyHumanConverted(this);
	bAlive = false;
	bIsBitten = false;
	bIsTargeted = false;
	Destroy();
}

void AHuman::SetTargeted(const bool bTarget)
{
	if (bIsBitten) return;
	bIsTargeted = bTarget;
	TargetArrow->SetVisibility(bIsTargeted);
	TargetArrow->SetHiddenInGame(false);
}

void AHuman::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bIsBitten) return;
	
	if (OtherActor == Cast<AZombie>(OtherActor))
	{
		GetBitten();
	}
}

