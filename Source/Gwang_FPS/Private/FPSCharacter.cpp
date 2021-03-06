// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFPSCharacter::AFPSCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(RootComponent);

	FPSArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPS_Arms"));
	FPSArms->SetupAttachment(FollowCamera);
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
}

//void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &AFPSCharacter::Pickup);

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
	if (CurrentFocusWeapon != nullptr)
	{
		PickupWeapon(CurrentFocusWeapon->GetWeaponType());
	}
}

void AFPSCharacter::OnBeginOverlapWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	NumOfOverlappingWeapons++;
	UE_LOG(LogTemp, Warning, TEXT("OnBeginOverlapWeapon() NumOfOverlappingWeapons: %i"), NumOfOverlappingWeapons);

	if (UKismetSystemLibrary::K2_IsTimerActiveHandle(GetWorld(), PickupTraceTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Timer is already running"));
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(PickupTraceTimerHandle, []()
		{
			UE_LOG(LogTemp, Warning, TEXT("Timer check"));
			// Todo: Display UI when player looks at a weapon, set CurrentFocusWeapon
		}, 0.5f, true, 0.f);
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
	if (HasWeapon(Weapon->GetWeaponType()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player: %s equipped a weapon: %i"), *this->GetName(), Weapon->GetWeaponType());
		Weapon->OnWeaponEquipped();
	}
}

