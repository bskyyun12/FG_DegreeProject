// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h" // GetLifetimeReplicatedProps

#include "AnimInstances/FPSAnimInterface.h"
#include "Components/HealthComponent.h"
#include "FPSPlayerControllerInterface.h"
#include "FPSPlayerController.h"

// Sets default values
AFPSCharacter::AFPSCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CameraContainer = CreateDefaultSubobject<UBoxComponent>(TEXT("CameraContainer"));
	CameraContainer->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraContainer);

	FPSArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPS_Arms"));
	FPSArms->SetupAttachment(FollowCamera);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDamageReceived.AddDynamic(this, &AFPSCharacter::OnDamageReceived);
	HealthComponent->OnHealthAcquired.AddDynamic(this, &AFPSCharacter::OnHealthAcquired);
	HealthComponent->OnDeath.AddDynamic(this, &AFPSCharacter::OnDeath);
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	CharacterMesh = GetMesh();
	if (!ensure(CharacterMesh != nullptr))
	{
		return;
	}
	CapsuleComponent = GetCapsuleComponent();
	if (!ensure(CapsuleComponent != nullptr))
	{
		return;
	}
}

void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSCharacter, bIsDead);
}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &AFPSCharacter::Pickup);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::Fire);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void AFPSCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AFPSCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AFPSCharacter::Pickup()
{
	if (CurrentFocus != nullptr)
	{
		PickupWeapon(CurrentFocus->GetWeaponType());

		// Temporary implementation
		if (bHasAnyWeapons == false)
		{
			EquipWeapon(CurrentFocus);
			bHasAnyWeapons = true;
		}
	}
}

void AFPSCharacter::Fire()
{
	if (CurrentWeapon != nullptr)
	{
		Server_Fire(CurrentWeapon, FollowCamera->GetComponentTransform());
	}
}

void AFPSCharacter::Server_Fire_Implementation(AFPSWeaponBase* Weapon, FTransform CameraTransform)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::Server_Fire_Implementation"));
	if (Weapon != nullptr)
	{
		Weapon->Server_FireWeapon(CameraTransform);
	}
}

void AFPSCharacter::OnBeginOverlapWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnBeginOverlapWeapon_Implementation()"));

	NumOfOverlappingWeapons++;
	UE_LOG(LogTemp, Warning, TEXT("OnBeginOverlapWeapon() NumOfOverlappingWeapons: %i"), NumOfOverlappingWeapons);

	if (UKismetSystemLibrary::K2_IsTimerActiveHandle(GetWorld(), PickupTraceTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Timer is already running"));
		return;
	}

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	World->GetTimerManager().SetTimer(PickupTraceTimerHandle, this, &AFPSCharacter::Client_CheckForWeapon, 0.1f, true, 0.f);
}

void AFPSCharacter::Client_CheckForWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("CheckForWeapon"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	FHitResult Hit;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * 300.f;
	DrawDebugLine(World, Start, End, FColor::Red, false, 0.1f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bIsHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	if (bIsHit)
	{
		AFPSWeaponBase* FocusedWeapon = Cast<AFPSWeaponBase>(Hit.GetActor());
		if (FocusedWeapon != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Weapon: %s"), *FocusedWeapon->GetName());
			CurrentFocus = FocusedWeapon;
		}

		DrawDebugPoint(World, Hit.ImpactPoint, 10.f, FColor::Green, false, 0.1f);
	}
	else
	{
		CurrentFocus = nullptr;
	}
}

void AFPSCharacter::OnEndOverlapWeapon_Implementation()
{
	NumOfOverlappingWeapons--;
	UE_LOG(LogTemp, Warning, TEXT("OnEndOverlapWeapon() NumOfOverlappingWeapons: %i"), NumOfOverlappingWeapons);

	if (NumOfOverlappingWeapons == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Clear Timer"));
		UKismetSystemLibrary::K2_ClearTimerHandle(GetWorld(), PickupTraceTimerHandle);
	}
}

void AFPSCharacter::PickupWeapon(EWeaponType WeaponType)
{
	Weapons.Add(WeaponType, 1);
	int16* Value = Weapons.Find(WeaponType);

	UE_LOG(LogTemp, Warning, TEXT("Player: %s picked up a weapon: %i, new amount: %d"), *this->GetName(), WeaponType, *Value);
}

bool AFPSCharacter::HasWeapon(EWeaponType WeaponType)
{
	return Weapons.Contains(WeaponType);
}

void AFPSCharacter::EquipWeapon(AFPSWeaponBase* Weapon)
{
	if (Weapon != nullptr && HasWeapon(Weapon->GetWeaponType()))
	{
		Server_EquipWeapon(Weapon);

		// Local
		if (!ensure(FPSArms != nullptr))
		{
			return;
		}
		Weapon->Client_OnClientWeaponEquipped(FPSArms);

		CurrentWeapon = Weapon;
	}
}

void AFPSCharacter::Server_EquipWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	Weapon->SetOwner(this);
	Weapon->Server_OnRepWeaponEquipped(CharacterMesh);
}

void AFPSCharacter::OnDamageReceived()
{
	UE_LOG(LogTemp, Warning, TEXT("( %s ) received damage."), *this->GetName());

}

void AFPSCharacter::OnHealthAcquired()
{
	UE_LOG(LogTemp, Warning, TEXT("( %s ) acquired health."), *this->GetName());
}

void AFPSCharacter::OnDeath()
{
	bIsDead = true;	// OnRep_bIsDead()
	CollisionHandleOnDeath();

	FTimerHandle RespawnTimer;
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, [&]()
		{
			RespawnPlayer();
		}, RespawnDelay, false);
}

void AFPSCharacter::RespawnPlayer()
{
	bIsDead = false;	// OnRep_bIsDead()
	CollisionHandleOnRespawn();

	if (HealthComponent != nullptr)
	{
		HealthComponent->Reset();
	}

	AFPSPlayerController* FPSController = Cast<AFPSPlayerController>(GetController());
	if (FPSController != nullptr)
	{
		if (UKismetSystemLibrary::DoesImplementInterface(FPSController, UFPSPlayerControllerInterface::StaticClass()))
		{
			IFPSPlayerControllerInterface::Execute_RespawnPlayer(FPSController);
		}
	}
}

void AFPSCharacter::OnRep_bIsDead()
{
	if (bIsDead)
	{
		CollisionHandleOnDeath();
	}
	else
	{
		CollisionHandleOnRespawn();
	}
}

void AFPSCharacter::CollisionHandleOnDeath()
{
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	CharacterMesh->SetSimulatePhysics(true);

	CameraContainer->SetCollisionProfileName(TEXT("IgnoreCharacter"));
	CameraContainer->SetSimulatePhysics(true);
}

void AFPSCharacter::CollisionHandleOnRespawn()
{
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CharacterMesh->SetSimulatePhysics(false);
	CharacterMesh->SetCollisionProfileName(TEXT("CharacterMesh"));
	CharacterMesh->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

	CameraContainer->SetSimulatePhysics(false);
	CameraContainer->SetCollisionProfileName(TEXT("NoCollision"));
	CameraContainer->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
}