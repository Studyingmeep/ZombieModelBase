// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h" 
#include "Zombie.generated.h"

class ASimGameController;
class AHuman;

UCLASS()
class ZOMBIEAPOCALYPSE_API AZombie : public ACharacter
{
	GENERATED_BODY()

public:
	
	// Sets default values for this actor's properties
	AZombie();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UCapsuleComponent* CollisionComponent;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void UpdateOldTargetPosition();

private:
	
	TWeakObjectPtr<ASimGameController> GameController;
	
	// Current human being chased
	TWeakObjectPtr<AHuman> CurrentTarget;
	
	FTimerHandle OldPositionTimerHandle;
	FVector PreviousPosition3SecondsAgo;

	// Search radius for detecting humans
	float SearchRadius = 300.f;

	// Max search radius (keeps growing until a human is found)
	float MaxSearchRadius = 3000.f;

	// How often to scan for humans
	float ScanInterval = 1.f;

	FTimerHandle ScanTimer;

	// Movement speed
	float ZombieSpeed = 100.f;
	
public:
	// Called on spawn
	void SetInitialZombie();
	
	void SetGameController(ASimGameController* InGameController);
	
private:
	void ScanForHumans();
	AHuman* FindClosestHuman(TArray<AActor*>& Humans) const;
	AHuman* FindClosestHuman(TArray<AHuman*>& Humans) const;
	void MoveTowardTarget();
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
