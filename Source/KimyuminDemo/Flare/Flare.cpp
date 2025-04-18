// Fill out your copyright notice in the Description page of Project Settings.


#include "Flare.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"

#include "../KimyuminDemoCharacter.h"

// Sets default values
AFlare::AFlare()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    // CapsuleComponent 积己 棺 风飘 汲沥
    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    RootComponent = Capsule;

    // StaticMeshComponent (Flare) 积己 棺 何馒
    FlareMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlareMesh"));
    FlareMesh->SetupAttachment(Capsule);

    // PointLightComponent 积己 棺 Flare俊 何馒
    //PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
    //PointLight->SetupAttachment(FlareMesh);

    flareLifeTime = 30.f;
}

// Called when the game starts or when spawned
void AFlare::BeginPlay()
{
	Super::BeginPlay();

    ACharacter* character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    my_character = Cast<AKimyuminDemoCharacter>(character);
    if (!my_character) return;
    
    FTimerHandle FlareTimerHandle;
    GetWorldTimerManager().SetTimer(FlareTimerHandle, this, &AFlare::OnFlareExpired, flareLifeTime, false);
}

void AFlare::OnFlareExpired()
{
    //if (PointLight){
    //    PointLight->DestroyComponent();
    //}

    if (my_character){
        if (my_character->flareNum < 4){
            my_character->flareNum++;
        }
    }
}


