// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Human.generated.h"

class UArrowComponent;
class UCapsuleComponent;
class ASimGameController;

UCLASS()
class ZOMBIEAPOCALYPSE_API AHuman : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHuman();

	virtual void Tick(float DeltaTime) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	void GetBitten();
	void ReduceDaysLeftUntilZombie();
	void TurnIntoZombie();

	bool IsAlive() const { return bAlive; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsBitten = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsTargeted = false;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> ActorCapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> TargetArrow;

	UPROPERTY()
	ASimGameController* GameController;
	
	// --- AI & Movement ---
	FVector WanderDirection;
	float CurrentWanderTimer = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	float MinWanderTimer = 2.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	float MaxWanderTimer = 6.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	float WalkingSpeed = 200.f;

	// --- State ---
	bool bAlive = true;
	FTimerHandle InfectionTimer;
	int DaysUntilZombie = 15;
};
