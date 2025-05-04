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

	// 광물 생성
	MineralMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineralMesh"));
	RootComponent = MineralMesh;

	// 광물 체력바 생성
	HPBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HBBarWidget"));
	HPBarWidgetComponent->SetupAttachment(RootComponent);
	HPBarWidgetComponent->SetHiddenInGame(true);

	HitPoints = InitialHitPoints;

	// 부딪히는 Event
	OnActorBeginOverlap.AddDynamic(this, &ABaseMineral::OnOverlapBegin);
}

// Called when the game starts or when spawned
void ABaseMineral::BeginPlay()
{
	Super::BeginPlay();

	// 광물 MI의 Dissolve 초기화
	if (MineralMesh && MineralMesh->GetMaterial(0))
	{
		if (UMaterialInterface* ParentMat = MineralMesh->GetMaterial(0))
		{
			DMI_Dissolved = UMaterialInstanceDynamic::Create(ParentMat, this);
			MineralMesh->SetMaterial(0, DMI_Dissolved);
			DMI_Dissolved->SetScalarParameterValue(FName("Dissolve"), -0.4f);
		}
	}

	// 체력바 초기화
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

// Laser Collision과 부딪히면
void ABaseMineral::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)

{
	UE_LOG(LogTemp, Warning, TEXT(">>> OnComponentOverlap called: %s hit by %s"), *GetName(), *GetNameSafe(OtherActor));

	if (OtherActor && OtherActor->IsA<ALaserCollision>())
	{
		// 레이저 위치를 기준으로 스폰
		const FTransform LaserTransform = OtherActor->GetActorTransform();
		const FVector  LaserLocation = OtherActor->GetActorLocation();
		const FRotator LaserRotation = OtherActor->GetActorRotation();

		// 체력바 이름 초기화
		WB_HPBar->MineralName->SetText(FText::FromName(UnderMineralName));

		// 체력이 있을때
		if (HitPoints > 0.0)
		{
			// 체력 소모
			HitPoints -= 1.0;
			// 체력바 보이게
			HPBarWidgetComponent->SetHiddenInGame(false);

			// 체력바 점점 깎이는게 보이게
			if (WB_HPBar && WB_HPBar->HPBar)
			{
				float NewRatio = static_cast<float>(HitPoints / InitialHitPoints);
				WB_HPBar->HPBar->SetPercent(FMath::Clamp(NewRatio, 0.0f, 1.0f));
			}

			if (HitPoints <= 0.0f)
			{
				// 체력이 0이면 사라지는 모션 실행, 되긴 하는데 좀 어색하넹
				GetWorldTimerManager().SetTimer(DissolveTimerHandle, this, &ABaseMineral::HandleDissolveStep, 0.01f, true);
			}

			else
			{
				// 흡수 안되는 관상용
				if (UnderMineralBP)
				{
					FActorSpawnParameters Params;
					GetWorld()->SpawnActor<AActor>(UnderMineralBP, LaserTransform.GetLocation(), LaserTransform.GetRotation().Rotator(), Params);
				}

				// 실질적으로 흡수되는
				if (UnderMineralNS)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, UnderMineralNS, LaserLocation, LaserRotation);
				}
			}
		}
	}
}

// 광물 사라지게 하는 Dissolve 관련, 근데 안됨 왜지?
void ABaseMineral::HandleDissolveStep()
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

	// Dissolve 값이 0.67f 이상이면 Destroy, 근데 Destroy만 됨
	if (NewVal >= 0.67f)
	{
		GetWorldTimerManager().ClearTimer(DissolveTimerHandle);
		Destroy();
	}
}