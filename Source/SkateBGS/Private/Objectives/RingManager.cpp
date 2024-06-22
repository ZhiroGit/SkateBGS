// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/RingManager.h"
#include "Characters/SkateCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARingManager::ARingManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARingManager::BeginPlay()
{
	Super::BeginPlay();

	AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), ASkateCharacter::StaticClass());
	if (Actor)
	{
		PlayerRef = Cast<ASkateCharacter>(Actor);
		if (PlayerRef)
		{
			PlayerRef->RingCollected.AddDynamic(this, &ARingManager::SetNextRing);
		}

	}
	InitializeRings();

	
}

// Called every frame
void ARingManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARingManager::SetNextRing()
{
	if (RingIndex + 1 >= RingArray.Num()) return;

	RingIndex += 1;

	if (RingArray[RingIndex])
	{
		ARing* Ring = RingArray[RingIndex];
		Ring->SetRingActive();

		if (Ring != RingArray.Last() && RingArray[RingIndex + 1])
		{
			Ring = RingArray[RingIndex+1];
			Ring->Mesh->SetVisibility(true);
		}
	}
}

void ARingManager::InitializeRings()
{
	for (ARing* Ring : RingArray)
	{
		Ring->SetRingInactive();
		Ring->Mesh->SetVisibility(false);
	}

	if (RingArray[0])
	{
		ARing* Ring = RingArray[0];
		Ring->Mesh->SetVisibility(true);
		Ring->SetRingActive();

		if (RingArray[RingIndex + 1])
		{
			Ring = RingArray[RingIndex + 1];
			Ring->Mesh->SetVisibility(true);
		}
	}
}

