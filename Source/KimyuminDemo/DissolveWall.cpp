// Fill out your copyright notice in the Description page of Project Settings.


#include "DissolveWall.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ADissolveWall::ADissolveWall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Wall�� Mesh �Է�
	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = WallMesh;
}

// Called when the game starts or when spawned
void ADissolveWall::BeginPlay()
{
	Super::BeginPlay();
	
	// Wall Material �����ͼ� Dissolve �ʱ�ȭ
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
	// ���� �Դ� ������ ���
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHP -= ActualDamage;
	UE_LOG(LogTemp, Log, TEXT("Egg took %f damage, HP now %f"), ActualDamage, CurrentHP);

	if (CurrentHP <= 0.f)
	{
		// 0�̸� Ÿ�̾� ����
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

	// ���� Dissolve�� �� ��������
	DMI_Dissolved->GetScalarParameterValue(FName("Dissolve"), CurrentVal);

	// 0.05f�� ���ؼ� Set
	float NewVal = CurrentVal + 0.01f;
	DMI_Dissolved->SetScalarParameterValue(FName("Dissolve"), NewVal);

	// Dissolve ���� 0.67f �̻��̸� Destroy
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

