// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h" 
#include "Zombie.generated.h"

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

private:
	
	
	
	// Current human being chased
	UPROPERTY()
	TWeakObjectPtr<AHuman> CurrentTarget;

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
	
private:
	void ScanForHumans();
	AHuman* FindClosestHuman(const TArray<AActor*>& Humans) const;
	void MoveTowardTarget();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* Overlapped,
	                    AActor* OtherActor,
	                    UPrimitiveComponent* OtherComp,
	                    int32 BodyIndex,
	                    bool bFromSweep,
	                    const FHitResult& SweepResult);

};
