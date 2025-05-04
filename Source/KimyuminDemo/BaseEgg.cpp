// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseEgg.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ABaseEgg::ABaseEgg()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Egg�� Mesh �Է�
	EggMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EggMesh"));
	RootComponent = EggMesh;
}

// Called when the game starts or when spawned
void ABaseEgg::BeginPlay()
{
	Super::BeginPlay();
	
	// Egg Material �����ͼ� Dissolve �ʱ�ȭ
	if (EggMesh && EggMesh->GetMaterial(0))
	{
		if (UMaterialInterface* ParentMat = EggMesh->GetMaterial(0))
		{
			DMI_Dissolved = UMaterialInstanceDynamic::Create(ParentMat, this);
			EggMesh->SetMaterial(0, DMI_Dissolved);
			DMI_Dissolved->SetScalarParameterValue(FName("Dissolve"), -0.4f);
		}
	}
}

float ABaseEgg::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// ���� �Դ� ������ ���
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHP -= ActualDamage;
	UE_LOG(LogTemp, Log, TEXT("Egg took %f damage, HP now %f"), ActualDamage, CurrentHP);

	if (CurrentHP <= 0.f)
	{
		// 0�̸� Ÿ�̾� ����
		GetWorldTimerManager().SetTimer(DissolveTimerHandleEgg, this, &ABaseEgg::EggDeath, 0.01f, true);
	}

	return ActualDamage;
}

void ABaseEgg::EggDeath()
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
		GetWorldTimerManager().ClearTimer(DissolveTimerHandleEgg);
		Destroy();
	}
}

// Called every frame
void ABaseEgg::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

