// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ring.h"
#include "RingManager.generated.h"

UCLASS()
class SKATEBGS_API ARingManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARingManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	TArray<ARing*> RingArray;

	class ASkateCharacter* PlayerRef;

	UFUNCTION()
	void SetNextRing();

private:
	int32 RingIndex = 0;
	void InitializeRings();

};
