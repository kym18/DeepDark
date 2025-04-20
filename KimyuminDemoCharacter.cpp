// Copyright Epic Games, Inc. All Rights Reserved.

#include "KimyuminDemoCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Components/ChildActorComponent.h"

#include "MyGameInstance.h"
#include "Flare/Flare.h"
#include "Particles/ParticleSystem.h"

#include "CableComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Mineral.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AKimyuminDemoCharacter

AKimyuminDemoCharacter::AKimyuminDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)


	// Grapple
	GrappleStartLocation = CreateDefaultSubobject<USceneComponent>(TEXT("GrappleStartLocation"));
	GrappleStartLocation->SetupAttachment(RootComponent);

	GrappleCable = CreateDefaultSubobject<UCableComponent>(TEXT("GrappleCable"));
	GrappleCable->SetupAttachment(GrappleStartLocation);


	 // 1. FirstPersonī�޶�
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

		// 2. LaserMesh
		LaserMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMesh"));
		LaserMesh->SetupAttachment(FirstPersonCamera);

			// 3. LaserArrow (LaserMesh �Ʒ�)
			LaserArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LaserArrow"));
			LaserArrow->SetupAttachment(LaserMesh);

				// 4. LaserArrow �Ʒ� LaserSphere
				//LaserSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserSphere"));
				//LaserSphere->SetupAttachment(LaserArrow);

					// 5. LaserMesh �Ʒ� �ٸ� �޽õ�
					DissolveLaser = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DissolveLaser"));
					DissolveLaser->SetupAttachment(LaserArrow);
					DissolveLaser->SetVisibility(false);

					Laser = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Laser"));
					Laser->SetupAttachment(LaserArrow);
					Laser->SetVisibility(false);

		// 6. RifleMesh (��Ʈ�� ���� ����, FirstPersonCamera�� ���� ���� ����)
		RifleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RifleMesh"));
		RifleMesh->SetupAttachment(FirstPersonCamera); // �Ǵ� SetupAttachment(FirstPersonCamera);

	isInCave = false;
	CurrentCharacterIndex = 0;

	//���
	modeNumber = 0;

	//����
	flareNum = 4;
	flareSpeed = 1000.f;

	//���
	isDash = false;

	//����
	isPictorialOpen = false;

	//���� ����
	grappleDistance = 3000.f;
}

void AKimyuminDemoCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetDefaultMode();

	//���� �ν��Ͻ� ����
	UGameInstance* GameInstance = GetGameInstance();
	if (UMyGameInstance* my_game_instance = Cast<UMyGameInstance>(GameInstance)){
		myGameInstance = my_game_instance;
	}

	//���� ����
	if (WB_UIClass && WB_UIPictorialBook && WB_UIMapSelect && WB_UIStore) {
		if (UUserWidget* wb_ui = CreateWidget<UUserWidget>(GetWorld(), WB_UIClass)) {
			if (isInCave) {
				wb_ui->AddToViewport();
			}
		}

		if (UUserWidget* pictorial_ui = CreateWidget<UUserWidget>(GetWorld(), WB_UIPictorialBook)) {
			pictorialBook = pictorial_ui;
		}

		if (UUserWidget* map_select_ui = CreateWidget<UUserWidget>(GetWorld(), WB_UIMapSelect)) {
			wbMapSelect = map_select_ui;
		}

		if (UUserWidget* store_ui = CreateWidget<UUserWidget>(GetWorld(), WB_UIStore)) {
			wbStore = store_ui;
		}
	}

	//�÷��̾� ī�޶� ����
	APlayerController* player_controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (player_controller && player_controller->PlayerCameraManager) {
		player_controller->PlayerCameraManager->ViewPitchMin = -50.0f;
		player_controller->PlayerCameraManager->ViewPitchMax = 50.0f;
	}

	//��� �ý���
	//���ּ� �ۿ����� ���� �ǵ��� ���� ����(�ӽ�)
	UE_LOG(LogTemp, Warning, TEXT("Oxygen Decreased OutOutOut InCave"));
	if (isInCave) {
		FTimerHandle OxygenTimerHandle;
		UE_LOG(LogTemp, Warning, TEXT("Oxygen Decreased Is InCave"));
		GetWorldTimerManager().SetTimer(
			OxygenTimerHandle,
			this,
			&AKimyuminDemoCharacter::DecreaseOxygen,
			1.0f,
			true
		);
	}

}

void AKimyuminDemoCharacter::DecreaseOxygen() 
{
	if (Oxygen <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("character die"));
		
		return;
	}
	Oxygen -= OxygenConsumptionRate;

}

void AKimyuminDemoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DeltaSeconds = DeltaTime;
}

void AKimyuminDemoCharacter::ModeChange()
{
	if (!isInCave) {
		SetDefaultMode();
	}
	else {
		if (modeNumber == 0) {
			SetDefaultMode();
		}
		else {
			SetWeaponMode();
		}
	}
}

void AKimyuminDemoCharacter::SetDefaultMode()
{
	modeNumber = 0;

	LaserMesh->SetVisibility(false);
	Laser->SetVisibility(false);
	DissolveLaser->SetVisibility(false);
	RifleMesh->SetVisibility(false);
	FirstPersonCamera->SetActive(false);
	FollowCamera->SetActive(true);
}

void AKimyuminDemoCharacter::SetWeaponMode()
{
	RifleMesh->SetVisibility(false);
	LaserMesh->SetVisibility(false);
	if (modeNumber == 1) {
		RifleMesh->SetVisibility(true);
	}
	else if (modeNumber == 2) {
		LaserMesh->SetVisibility(true);
	}
	FirstPersonCamera->SetActive(true);
	FollowCamera->SetActive(false);
}


void AKimyuminDemoCharacter::SpawnFlare()
{
	if (flareNum <= 0) {
		return;
	}

	flareNum--;

	if (!BP_Flare) return;

	// 1. ���� ��ġ �� ȸ��
	FVector forward_vector = GetActorForwardVector();
	FVector spawn_location = GetActorLocation() + forward_vector * 40;

	// 2. ���� �Ķ����
	FActorSpawnParameters spawn_Params;
	spawn_Params.Owner = this;
	spawn_Params.Instigator = GetInstigator();

	// 3. ���� ����
	AActor* spawned_actor = GetWorld()->SpawnActor<AActor>(BP_Flare, spawn_location, GetActorRotation(), spawn_Params);
	if (!spawned_actor) return;

	AFlare* flare = Cast<AFlare>(spawned_actor);
	if (!flare) return;


	// 4. ���� �ӵ� ����
	FVector velocity = (forward_vector + FVector(0.f, 0.f, 1.f)) * flareSpeed;

	// ����
	if (UStaticMeshComponent* flare_mesh = flare->FlareMesh){
		flare_mesh->SetSimulatePhysics(true);
		flare_mesh->SetPhysicsLinearVelocity(velocity);
	}

}

void AKimyuminDemoCharacter::DashFlipFlop()
{
	if (!isDash) {
	//�ȱ� -> ��� ��
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
	else {
	//��� �� -> �ȱ�
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}
	isDash = !isDash;
}

void AKimyuminDemoCharacter::PictorialFlipFlop()
{
	if (!pictorialBook) return;
	if (!isInCave) return;

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!pictorialBook->IsInViewport()) {
		pictorialBook->AddToViewport();

		// ���콺 Ŀ�� ON
		controller->bShowMouseCursor = true;

		// �Է� ���: ���� + UI
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(pictorialBook->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		controller->SetInputMode(InputMode);
	}
	else {
		pictorialBook->RemoveFromParent();

		// ���콺 Ŀ�� OFF
		controller->bShowMouseCursor = false;

		// �Է� ���: ����
		FInputModeGameOnly InputMode;
		controller->SetInputMode(InputMode);
	}
}

//��ȿ������. �ٲ����
void AKimyuminDemoCharacter::MapAndStoreFlipFlop()
{
	//�� �Ǵ� ���� ���߿� �ϳ��� ������ return
	if (!wbMapSelect || !wbStore) return;

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	//�� ���� UI ����
	if(isOverlapMapSelectZone){
		if (!wbMapSelect->IsInViewport()) {
			wbMapSelect->AddToViewport();
			// ���콺 Ŀ�� ON
			controller->bShowMouseCursor = true;

			// �Է� ���: ���� + UI
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(wbMapSelect->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(true);
			controller->SetInputMode(InputMode);
		}
		else {
			wbMapSelect->RemoveFromParent();

			// ���콺 Ŀ�� OFF
			controller->bShowMouseCursor = false;

			// �Է� ���: ����
			FInputModeGameOnly InputMode;
			controller->SetInputMode(InputMode);
		}
	}

	//���� UI ����
	if (isOverlapStore) {
		if (!wbStore->IsInViewport()) {
			wbStore->AddToViewport();
			// ���콺 Ŀ�� ON
			controller->bShowMouseCursor = true;

			// �Է� ���: ���� + UI
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(wbStore->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(true);
			controller->SetInputMode(InputMode);
		}
		else {
			wbStore->RemoveFromParent();

			// ���콺 Ŀ�� OFF
			controller->bShowMouseCursor = false;

			// �Է� ���: ����
			FInputModeGameOnly InputMode;
			controller->SetInputMode(InputMode);
		}
	}
}



//////////////////////////////////////////////////////////////////////////
// Input

void AKimyuminDemoCharacter::LeftMouseBtnPressed()
{
	if (modeNumber == 1) { //��
		// 1. �ݵ� ����
		RifleMesh->AddLocalRotation(FRotator(-2.0f, 0.f, 0.f)); // ���� �ݵ�

		// 2. ���� Ʈ���̽� ���
		FVector Start = FirstPersonCamera->GetComponentLocation();
		FVector End = Start + FirstPersonCamera->GetForwardVector() * 100000.0f;

		FHitResult Hit;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);
		TraceParams.bReturnPhysicalMaterial = true;

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			Start,
			End,
			ECC_Visibility,
			TraceParams
		);

		// 3. �ǰ� ó��
		if (bHit){
			// ����Ʈ ����
			if (ExplosionFX){
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ExplosionFX,
					Hit.ImpactPoint,
					FRotator::ZeroRotator,
					FVector(1.0f) // Scale
				);
			}

			// ������ ����
			UGameplayStatics::ApplyDamage(
				Hit.GetActor(),
				10.0f,
				GetController(),
				this,
				UDamageType::StaticClass()
			);
		}

		// 4. �ݵ� ����
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this](){
			RifleMesh->AddLocalRotation(FRotator(2.0f, 0.f, 0.f)); // ����ġ
		}, 0.05f, false);
	}
	else if(modeNumber == 2) { //������
		Laser->SetVisibility(true);

		StartFiringLaser();
	}
	else if (modeNumber == 3) { //Ư�� �ɷ�
		//0�� Ž�谡, 1�� ���� Ưȭ
		if (CurrentCharacterIndex == 0) {
			IsHoldingF = true;
			// 1. ����Ʈ���̽� ���۰� �� ��ġ
			FVector Start = FirstPersonCamera->GetComponentLocation();
			FVector Direction = FirstPersonCamera->GetForwardVector();
			FVector End = Start + Direction * grappleDistance;

			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this); // �ڱ� �ڽ� ����
			Params.bReturnPhysicalMaterial = false;

			// 2. ����Ʈ���̽�
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				Hit,
				Start,
				End,
				ECC_Visibility,
				Params
			);

			// 3. ��Ʈ�� ��� Dissolve ���� ����
			if (bHit) {
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(Hit.ImpactPoint);
				SpawnTransform.SetRotation(FQuat::Identity);
				SpawnTransform.SetScale3D(FVector(1.0f));

				AActor* Spawned = GetWorld()->SpawnActor<AActor>(DissolveClass, SpawnTransform);
				currentHole = Spawned;
			}
		}
		else if (CurrentCharacterIndex == 1) {
			// 1. ���� ����
			isGrappling = true;
			FRotator CameraRotate = FRotator::ZeroRotator; // ���� ȸ�� ó���� ���
			GetCharacterMovement()->GravityScale = 1.0f;

			// 2. ����Ʈ���̽� ���� ���
			FVector Start = FirstPersonCamera->GetComponentLocation();
			FVector Forward = FirstPersonCamera->GetForwardVector();
			FVector End = Start + (Forward * grappleDistance);

			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			Params.bReturnPhysicalMaterial = false;

			// 3. Ʈ���̽� ����
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				Hit,
				Start,
				End,
				ECC_Visibility,
				Params
			);

			// 4. ��Ʈ ���� �� ȸ�� ���
			if (bHit)
			{
				FVector TraceDir = (Hit.ImpactPoint - Start).GetSafeNormal();
				CameraRotate = TraceDir.ToOrientationRotator();

				FVector grapple_location = Hit.Location;

				// 1. ���� ����
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(grapple_location);
				SpawnTransform.SetRotation(GetActorRotation().Quaternion());
				SpawnTransform.SetScale3D(FVector(1.0f));
				Hook = GetWorld()->SpawnActor<AActor>(GrappleHookClass, SpawnTransform);

				GrappleCable->SetVisibility(true);

				GrappleCable->SetWorldLocation(grapple_location);

				FVector CurrentLocation = GetActorLocation();
				FVector Direction = (grapple_location - CurrentLocation).GetSafeNormal();
				FVector LaunchVelocity = Direction * 3000.f;

				//����!!!
				LaunchCharacter(LaunchVelocity, false, false);
				FTimerHandle GrappleResetTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(GrappleResetTimerHandle, this, &AKimyuminDemoCharacter::ResetGrappleHook, 0.45f, false);
			}
			else {
				ResetGrappleHook();
			}
		}
	}
}

void AKimyuminDemoCharacter::ResetGrappleHook()
{

	// 1. ���̺� �����
	if (GrappleCable) {
		GrappleCable->SetVisibility(false);
	}

	// 2. �� ����
	if (IsValid(Hook)) {
		Hook->Destroy();
		Hook = nullptr;
	}

	// 3. �߷� ������ ����
	GetCharacterMovement()->GravityScale = 1.0f;

	// 4. ���� �ʱ�ȭ
	isGrappling = false;

	// 5. ������ �� CanGrapple = true
	FTimerHandle GrappleCooldownTimer;
	GetWorld()->GetTimerManager().SetTimer(
		GrappleCooldownTimer,
		this,
		&AKimyuminDemoCharacter::EnableGrapple,
		2.0,
		false
	);
}

void AKimyuminDemoCharacter::EnableGrapple(){
	isGrappling = true;
}

void AKimyuminDemoCharacter::StartFiringLaser()
{
	GetWorld()->GetTimerManager().SetTimer(LaserTimerHandle, this, &AKimyuminDemoCharacter::FireLaserTick, 0.05f, true);
}

void AKimyuminDemoCharacter::FireLaserTick()
{
	if (!LaserArrow || !Laser) return;

	FVector Start = LaserArrow->GetComponentLocation();
	FVector End = Start + FirstPersonCamera->GetForwardVector() * 100000.0f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit) {
		float DistanceToWall = Hit.Distance;

		// 1. �޽� ���� ����
		FVector NewScale = FVector(DistanceToWall * 1.0f, 0.17f, 0.17f); // Scale�� �ణ ���� �ʿ�
		Laser->SetWorldScale3D(NewScale);

		// 2. �޽� ȸ�� ����
		//FRotator RotationAdjust = FRotator(0.f, 0.f, 5.f); // �������Ʈ�� ���� ���� ����
		//Laser->AddLocalRotation(RotationAdjust);

		 // ���� ���� ��� (������ �ٶ󺸰�)
		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(Start, End);
		Laser->SetWorldRotation(LookAtRot);

		// 3. �浹 ����Ʈ �� ������
		if (LaserImpactFX){
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LaserImpactFX, Hit.ImpactPoint);
		}

		UGameplayStatics::ApplyDamage(
			Hit.GetActor(),
			10.0f,
			GetController(),
			this,
			UDamageType::StaticClass()
		);

		// 4. �浹 ���� ���� (BP_LaserCollision)
		if (LaserCollisionClass) {
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(Hit.ImpactPoint);
			GetWorld()->SpawnActor<AActor>(LaserCollisionClass, SpawnTransform);
		}

		// 5. ���� ĳ���� �õ�
		if (AMineral* Mineral = Cast<AMineral>(Hit.GetActor())){
			// TODO: ���ϴ� ó�� �߰�
		}
	}
}


void AKimyuminDemoCharacter::LeftMouseBtnReleased()
{
	if (modeNumber == 2) { //������
		UE_LOG(LogTemp, Warning, TEXT("modeNumber==2"));
		Laser->SetVisibility(false);
		Laser->SetRelativeScale3D(FVector(2, 2, 2));
		GetWorld()->GetTimerManager().ClearTimer(LaserTimerHandle);
	}
	else if (modeNumber == 3) {
		if (!IsHoldingF) return;
		if (currentHole) {
			currentHole->Destroy();
		}
		IsHoldingF = false;
	}
}

void AKimyuminDemoCharacter::RightMouseBtnPressed()
{
	if (modeNumber != 2) return;

	// 1. DissolveLaser Ȱ��ȭ
	DissolveLaser->SetVisibility(true);

	// 2. ������ ������Ʈ�� Ÿ�̸� ����
	GetWorld()->GetTimerManager().SetTimer(DissolveTimerHandle, FTimerDelegate::CreateLambda([this]()
		{
			FVector Start = LaserArrow->GetComponentLocation();
			FVector Direction = LaserArrow->GetForwardVector();
			FVector End = Start + Direction * 100000.0f;

			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

			if (bHit){
				float DistanceToWall = Hit.Distance;

				// 3. DissolveLaser ũ�� ����
				DissolveLaser->SetRelativeScale3D(FVector(DistanceToWall, 2.f, 2.f)); // �Ÿ� ����
				DissolveLaser->AddLocalRotation(FRotator(5.f, 0.f, 0.f));

				// 4. ��Ʈ ������ Dissolve ����Ʈ ����
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(Hit.ImpactPoint);
				SpawnTransform.SetRotation(FQuat::Identity);
				SpawnTransform.SetScale3D(FVector(1.f));

				GetWorld()->SpawnActor<AActor>(DissolveCircleClass, SpawnTransform);
			}

		}), 0.05f, true);
}

void AKimyuminDemoCharacter::RightMouseBtnReleased()
{
	if (modeNumber != 2) return;

	DissolveLaser->SetVisibility(false);
	DissolveLaser->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));
	
	GetWorld()->GetTimerManager().ClearTimer(DissolveTimerHandle);
}

void AKimyuminDemoCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AKimyuminDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKimyuminDemoCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKimyuminDemoCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AKimyuminDemoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AKimyuminDemoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// �׸��� �̰��� ���� ȹ�� �� �ۼ�Ƽ�� �ø��� �Žô�...
void AKimyuminDemoCharacter::UpdateMineral(FName MineralName)
{
	if (double* Existing = MineralList.Find(MineralName))
	{
		*Existing += 0.1;
	}

	else
	{
		MineralList.Add(MineralName, 0.1f);
	}
}
