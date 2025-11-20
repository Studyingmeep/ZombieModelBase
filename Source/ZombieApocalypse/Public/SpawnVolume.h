// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

class UBoxComponent;

UCLASS()
class ZOMBIEAPOCALYPSE_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnVolume();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawn")
	TObjectPtr<UBoxComponent> Box;

	/// Returns a random point inside the box (World Space)
	UFUNCTION(BlueprintCallable, Category="Spawn")
	FVector GetRandomPoint() const;

};
