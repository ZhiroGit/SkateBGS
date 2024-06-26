// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/Ring.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Characters/SkateCharacter.h"

// Sets default values
ARing::ARing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = Mesh;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());

	VFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara Effect"));
	VFX->SetupAttachment(GetRootComponent());

	OnMaterial = CreateDefaultSubobject<UMaterialInstance>(TEXT("On Material"));
	OffMaterial = CreateDefaultSubobject<UMaterialInstance>(TEXT("Off Material"));

}

// Called when the game starts or when spawned
void ARing::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ARing::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &ARing::OnSphereEndOverlap);
	
}

float ARing::TransformedSin()
{
	return Amplitude * FMath::Sin(RunningTime * Period);
}

float ARing::TransformedCosin()
{
	return Amplitude * FMath::Cos(RunningTime * Period);
}

void ARing::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASkateCharacter* Player = Cast<ASkateCharacter>(OtherActor);
	if (Player)
	{
		if (OverlapEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OverlapEffect, GetActorLocation());
		}
		if (OverlapSound)
		{
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), OverlapSound, GetActorLocation());
		}

		Player->CollectRing();

		Destroy();
	}
}

void ARing::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

// Called every frame
void ARing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;
	const float Z = TransformedSin();
	AddActorWorldOffset(FVector(0.f, 0.f, Z));

}

void ARing::SetRingInactive()
{
	if (Mesh)
	{
		Mesh->SetMaterial(0, OffMaterial);
	}

	if (Sphere)
	{
		Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
}

void ARing::SetRingActive()
{
	if (Mesh)
	{
		Mesh->SetMaterial(0, OnMaterial);
	}

	if (Sphere)
	{
		Sphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
}

