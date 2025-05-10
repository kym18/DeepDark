// Fill out your copyright notice in the Description page of Project Settings.


#include "DissolveWall.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ADissolveWall::ADissolveWall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Wall의 Mesh 입력
	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = WallMesh;
}

// Called when the game starts or when spawned
void ADissolveWall::BeginPlay()
{
	Super::BeginPlay();
	
	// Wall Material 가져와서 Dissolve 초기화
	if (WallMesh && WallMesh->GetMaterial(0))
	{
		if (UMaterialInterface* ParentMat = WallMesh->GetMaterial(0))
		{
			DMI_Dissolved = UMaterialInstanceDynamic::Create(ParentMat, this);
			WallMesh->SetMaterial(0, DMI_Dissolved);
			DMI_Dissolved->SetScalarParameterValue(FName("Dissolve"), -0.4f);
		}
	}
}

float ADissolveWall::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 현재 입는 데미지 계산
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHP -= ActualDamage;
	UE_LOG(LogTemp, Log, TEXT("Egg took %f damage, HP now %f"), ActualDamage, CurrentHP);

	if (CurrentHP <= 0.f)
	{
		// 0이면 타미어 실행
		GetWorldTimerManager().SetTimer(DissolveTimerHandleWall, this, &ADissolveWall::WallDissolved, 0.01f, true);
	}

	return ActualDamage;
}

void ADissolveWall::WallDissolved()
{
	if (!DMI_Dissolved)
	{
		return;
	}

	// 현재 Dissolve의 값 가져오고
	DMI_Dissolved->GetScalarParameterValue(FName("Dissolve"), CurrentVal);

	// 0.05f씩 더해서 Set
	float NewVal = CurrentVal + 0.01f;
	DMI_Dissolved->SetScalarParameterValue(FName("Dissolve"), NewVal);

	// Dissolve 값이 0.67f 이상이면 Destroy
	if (NewVal >= 0.67f)
	{
		GetWorldTimerManager().ClearTimer(DissolveTimerHandleWall);
		Destroy();
	}
}

// Called every frame
void ADissolveWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

