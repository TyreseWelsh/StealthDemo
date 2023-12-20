// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Basic.h"

// Sets default values
AEnemy_Basic::AEnemy_Basic()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemy_Basic::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemy_Basic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy_Basic::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

