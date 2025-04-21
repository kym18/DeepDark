// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserCollision.h"
#include "TimerManager.h"

// Sets default values
ALaserCollision::ALaserCollision()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALaserCollision::BeginPlay()
{
	Super::BeginPlay();

	// LaserCollision 나오면 일정 시간 후 사라지게
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &ALaserCollision::DestroySelf, 0.5f, false);
	
}

// Called every frame
void ALaserCollision::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// LaserCollision 나오면 일정 시간 후 사라지게
void ALaserCollision::DestroySelf()
{
	Destroy();
}
