// Fill out your copyright notice in the Description page of Project Settings.


#include "CurrentMap.h"

// Sets default values
ACurrentMap::ACurrentMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MapMesh"));
	RootComponent = MapMesh;

	//초기 값.
	TargetEmissiveValue = 0.1f;
	TargetOpacityValue = 1.f;
	TargetDissolveValue = - 4.f;

	IsChange = false;
	IsOpenMap = true;
}

void ACurrentMap::ConvertMapMode()
{
	IsChange = true;
	IsOpenMap = true;

	MI_default = MapMesh->GetMaterial(0);

	

	if (DMI_map) {
		MapMesh->SetMaterial(0, DMI_map);

		//현재 값 초기화
		CurrentEmissiveValue = 0.0f;
		CurrentOpacityValue = 1.f;
		//CurrentDissolveValue = -4.f;

		//목표 값
		TargetEmissiveValue = 0.01f;
		TargetOpacityValue = 0.3f;
		//TargetDissolveValue = 0.f;

	}
}

void ACurrentMap::ConvertDefaultMode()
{
	IsChange = true;
	IsOpenMap = false;

	//현재 값 초기화
	CurrentEmissiveValue = TargetEmissiveValue;
	CurrentOpacityValue = TargetOpacityValue;
	//CurrentDissolveValue = TargetDissolveValue;
	//목표 값
	TargetEmissiveValue = 0.0f;
	TargetOpacityValue = 1.f;
	//TargetDissolveValue = -4.f;
}

// Called when the game starts or when spawned
void ACurrentMap::BeginPlay()
{
	Super::BeginPlay();
	DMI_map = UMaterialInstanceDynamic::Create(MI_Map, this);
}

// Called every frame
void ACurrentMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsChange) {
		bool check = false;
		if (IsOpenMap && TargetEmissiveValue > CurrentEmissiveValue) {
			CurrentEmissiveValue += DeltaTime;
			check = true;
		}
		else if (!IsOpenMap && TargetEmissiveValue < CurrentEmissiveValue) {
			CurrentEmissiveValue -= DeltaTime;
			check = true;
		}

		if (IsOpenMap && TargetOpacityValue > CurrentOpacityValue) {
			CurrentOpacityValue += DeltaTime;
			check = true;
		}
		else if (!IsOpenMap && TargetOpacityValue < CurrentOpacityValue) {
			CurrentOpacityValue -= DeltaTime;
			check = true;
		}

		if (check == false) {
			//다시 원래 메시로 돌리기
			if (abs(TargetOpacityValue - 1.f) <= 0.00001f) {
				MapMesh->SetMaterial(0, MI_default);
			}
			TargetEmissiveValue = CurrentEmissiveValue;
			TargetOpacityValue = CurrentOpacityValue;

			IsChange = false;
		}

		//맵 머티리얼 변경
		DMI_map->SetScalarParameterValue(FName("Emissive Value"), CurrentEmissiveValue);
		DMI_map->SetScalarParameterValue(FName("Opacity Value"), CurrentOpacityValue);
	}
}

