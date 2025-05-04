// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseMineral.h"
#include "BaseMineralTag.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/ProgressBar.h"
#include "TimerManager.h"
#include "LaserCollision.h"

// Sets default values
ABaseMineral::ABaseMineral()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ���� ����
	MineralMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineralMesh"));
	RootComponent = MineralMesh;

	// ���� ü�¹� ����
	HPBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HBBarWidget"));
	HPBarWidgetComponent->SetupAttachment(RootComponent);
	HPBarWidgetComponent->SetHiddenInGame(true);

	HitPoints = InitialHitPoints;

	// �ε����� Event
	OnActorBeginOverlap.AddDynamic(this, &ABaseMineral::OnOverlapBegin);
}

// Called when the game starts or when spawned
void ABaseMineral::BeginPlay()
{
	Super::BeginPlay();

	// ���� MI�� Dissolve �ʱ�ȭ
	if (MineralMesh && MineralMesh->GetMaterial(0))
	{
		if (UMaterialInterface* ParentMat = MineralMesh->GetMaterial(0))
		{
			DMI_Dissolved = UMaterialInstanceDynamic::Create(ParentMat, this);
			MineralMesh->SetMaterial(0, DMI_Dissolved);
			DMI_Dissolved->SetScalarParameterValue(FName("Dissolve"), -0.4f);
		}
	}

	// ü�¹� �ʱ�ȭ
	if (HPBarWidgetComponent)
	{
		HPBarWidgetComponent->SetHiddenInGame(true);

		if (UBaseMineralTag* TagWidget = Cast<UBaseMineralTag>(HPBarWidgetComponent->GetUserWidgetObject()))
		{
			WB_HPBar = TagWidget;

			if (TagWidget->HPBar)
			{
				float InitialRatio = static_cast<float>(HitPoints / InitialHitPoints);
				TagWidget->HPBar->SetPercent(FMath::Clamp(InitialRatio, 0.0f, 1.0f));
			}
		}
	}
}

// Called every frame
void ABaseMineral::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Laser Collision�� �ε�����
void ABaseMineral::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)

{
	UE_LOG(LogTemp, Warning, TEXT(">>> OnComponentOverlap called: %s hit by %s"), *GetName(), *GetNameSafe(OtherActor));

	if (OtherActor && OtherActor->IsA<ALaserCollision>())
	{
		// ������ ��ġ�� �������� ����
		const FTransform LaserTransform = OtherActor->GetActorTransform();
		const FVector  LaserLocation = OtherActor->GetActorLocation();
		const FRotator LaserRotation = OtherActor->GetActorRotation();

		// ü�¹� �̸� �ʱ�ȭ
		WB_HPBar->MineralName->SetText(FText::FromName(UnderMineralName));

		// ü���� ������
		if (HitPoints > 0.0)
		{
			// ü�� �Ҹ�
			HitPoints -= 1.0;
			// ü�¹� ���̰�
			HPBarWidgetComponent->SetHiddenInGame(false);

			// ü�¹� ���� ���̴°� ���̰�
			if (WB_HPBar && WB_HPBar->HPBar)
			{
				float NewRatio = static_cast<float>(HitPoints / InitialHitPoints);
				WB_HPBar->HPBar->SetPercent(FMath::Clamp(NewRatio, 0.0f, 1.0f));
			}

			if (HitPoints <= 0.0f)
			{
				// ü���� 0�̸� ������� ��� ����, �Ǳ� �ϴµ� �� ����ϳ�
				GetWorldTimerManager().SetTimer(DissolveTimerHandle, this, &ABaseMineral::HandleDissolveStep, 0.01f, true);
			}

			else
			{
				// ��� �ȵǴ� �����
				if (UnderMineralBP)
				{
					FActorSpawnParameters Params;
					GetWorld()->SpawnActor<AActor>(UnderMineralBP, LaserTransform.GetLocation(), LaserTransform.GetRotation().Rotator(), Params);
				}

				// ���������� ����Ǵ�
				if (UnderMineralNS)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, UnderMineralNS, LaserLocation, LaserRotation);
				}
			}
		}
	}
}

// ���� ������� �ϴ� Dissolve ����, �ٵ� �ȵ� ����?
void ABaseMineral::HandleDissolveStep()
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

	// Dissolve ���� 0.67f �̻��̸� Destroy, �ٵ� Destroy�� ��
	if (NewVal >= 0.67f)
	{
		GetWorldTimerManager().ClearTimer(DissolveTimerHandle);
		Destroy();
	}
}