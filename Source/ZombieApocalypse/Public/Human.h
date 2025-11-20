// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Human.generated.h"

class UArrowComponent;
class UCapsuleComponent;
class ASimGameController;

UCLASS()
class ZOMBIEAPOCALYPSE_API AHuman : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHuman();

protected:
	
	UPROPERTY()
	ASimGameController* GameController;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UCapsuleComponent* ActorCapsuleComponent;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bAlive = true;
	FTimerHandle InfectionTimer;
	int DaysUntilZombie = 15;
	
public:	

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UArrowComponent> TargetArrow;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsBitten = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsTargeted = false;

	bool IsAlive() const { return bAlive; }
	void GetBitten();
	void ReduceDaysLeftUntilZombie();
	void TurnIntoZombie();
	void SetTargeted(bool bTarget);

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
