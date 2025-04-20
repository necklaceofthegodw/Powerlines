// Fill out your copyright notice in the Description page of Project Settings.


#include "Powergrid.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"


APowergrid::APowergrid()
{
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
	RootComponent = Spline;
}

void APowergrid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CableInfos.Empty();
	CableInfos.Add(FCableInfo(nullptr, 0, FName("SocketLine1"), 500.0f, true, FVector::ZeroVector));
	CableInfos.Add(FCableInfo(nullptr, 0, FName("SocketLine2"), 500.0f, true, FVector::ZeroVector));
	CableInfos.Add(FCableInfo(nullptr, 0, FName("SocketLine3"), 500.0f, true, FVector::ZeroVector));
	CableInfos.Add(FCableInfo(nullptr, 0, FName("SocketLine4"), 500.0f, true, FVector::ZeroVector));
	
	//UpdatePowerlines();
}

TArray<UStaticMeshComponent*> APowergrid::SpawnPowerlineMeshesAlongSpline()
{
	TArray<UStaticMeshComponent*> powerlineMeshes;
	
	if (bInitialPolesSpawned)
	{
		for (int i=0; i < Spline->GetNumberOfSplinePoints(); i++)
		{
			powerlineMeshes.Add(AddPowerlineMeshToSpline(i));
		}
	}
	else
	{
		bInitialPolesSpawned = true;
		Spline->ClearSplinePoints();

		for (int i=0; i<InitialPolesNum;i++)
		{
			FVector Location = GetActorLocation() + GetActorForwardVector()*CableLength*i;
			Spline->AddSplinePoint(Location, ESplineCoordinateSpace::World);
			powerlineMeshes.Add(AddPowerlineMeshToSpline(i));
		}
	}

	return powerlineMeshes;
}

void APowergrid::UpdatePowerlines()
{
	DestroyPowerlinesAndCables();
	PowerPoleMeshes = SpawnPowerlineMeshesAlongSpline();
	
	for (int i=0; i < PowerPoleMeshes.Num() - 1; i++)
	{
		UStaticMeshComponent* mesh = PowerPoleMeshes[i];
		int meshIndex = i;
		for (auto& name : mesh->GetAllSocketNames())
		{
			FName socketName = name;
			if (PowerPoleMeshes[meshIndex + 1] != nullptr )
			{
				UCableComponent* cable = AddCableComponent(mesh, socketName);
				Cables.Add(cable);

				for (auto &CableInfo: CableInfos)
				{
					if (CableInfo.CableSocket == socketName)
					{
						FVector startPoint = GetActorTransform().InverseTransformPosition(PowerPoleMeshes[meshIndex]->GetSocketLocation(socketName));
						FVector endPoint = GetActorTransform().InverseTransformPosition(PowerPoleMeshes[meshIndex + 1]->GetSocketLocation(socketName));
						float cableLength = FVector::Distance(startPoint, endPoint);
						FCableInfo cableInfo = FCableInfo(cable, meshIndex, socketName, cableLength, true, endPoint);
						SetCableProperties(cableInfo);
					}
				}
			}
		}
	}
}

void APowergrid::SetRemainingLineLength(float LengthToSet)
{
	RemainingLineLength = LengthToSet;
	UE_LOG(LogTemp, Log, TEXT("Remaining line length: %f"), RemainingLineLength);

}

void APowergrid::InitializePowerlineParams(UStaticMesh* PowerlineMeshToSet, int PolesNumToSet, float CableLengthToSet)
{
	PowerlineMesh = PowerlineMeshToSet;
	InitialPolesNum = PolesNumToSet;
	CableLength = CableLengthToSet;
}

void APowergrid::AddSplinePoints(const FVector& LineStart, const FVector& LineEnd)
{
	// FVector direction = (LineEnd - LineStart);
	// FVector spawnLocation = FVector::ZeroVector;
	// direction.Normalize();
	// SetRemainingLineLength(LineStart, LineEnd);
	//
	// int poles = FMath::TruncToInt(RemainingLineLength/CableLength);
	//
	// for (int i = 1; i <= poles; i++)
	// {
	// 	spawnLocation = LineStart - (direction * RemainingLineLength) + (direction * CableLength * i);
	// 	Spline->AddSplinePoint(spawnLocation, ESplineCoordinateSpace::World);
	// }
	// RemainingLineLength -= (poles * CableLength);

	FVector direction = (LineEnd - LineStart);
	FVector spawnLocation = FVector::ZeroVector;

	direction.Normalize();
	float lineLength = RemainingLineLength + FVector::Dist(LineStart, LineEnd);
	int poles = FMath::TruncToInt(lineLength/CableLength);

	for (int i = 1; i <= poles; i++)
	{
		spawnLocation = LineStart - (direction * RemainingLineLength) + (direction * CableLength * i);
		Spline->AddSplinePoint(spawnLocation, ESplineCoordinateSpace::World);
	}
	RemainingLineLength = lineLength - (poles * CableLength);
}

void APowergrid::DestroyPowerlinesAndCables()
{
	for(auto &Mesh: PowerPoleMeshes)
	{
		Mesh->DetachFromParent();
		Mesh->DestroyComponent();
	}
	for(auto &Cable: Cables)
	{
		Cable->bAttachEnd = false;
		Cable->DetachFromParent();
		Cable->DestroyComponent();
	}
	Cables.Empty();
}

void APowergrid::SetCableProperties(FCableInfo &CableInfoToSet)
{
	UCableComponent * cable = CableInfoToSet.CableRef;

	cable->bAttachEnd = true;
	cable->EndLocation = CableInfoToSet.EndPoint;
	cable->CableLength = CableInfoToSet.CableLength;
	cable->SolverIterations = 10;
	cable->bEnableStiffness = CableInfoToSet.bEnableCableStiffness;
}

UCableComponent* APowergrid::AddCableComponent(UStaticMeshComponent* MeshComponent, FName AttachSocketName)
{
	FVector socketLocation = GetActorTransform().InverseTransformPosition(MeshComponent->GetSocketLocation(AttachSocketName));

	UCableComponent* cable = NewObject<UCableComponent>(this);
	cable->RegisterComponent();
	cable->SetMobility(EComponentMobility::Movable);
	cable->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	cable->SetRelativeLocation(socketLocation);
	cable->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
	cable->bAttachEnd = false;
	
	return cable;
}

USplineMeshComponent* APowergrid::AddPowerlineMeshToSpline(int SplineIndex)
{
	if (!PowerlineMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spline mesh not assigned"));
		return nullptr;
	}
	
	USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(this);
	splineMesh->RegisterComponent();
	splineMesh->SetMobility(EComponentMobility::Movable);
	
	FVector spawnLocation = Spline->GetLocationAtSplinePoint(SplineIndex,ESplineCoordinateSpace::World);
	FRotator spawnRotation = Spline->GetRotationAtSplinePoint(SplineIndex,ESplineCoordinateSpace::World);
	
	splineMesh->SetWorldLocation(spawnLocation);
	splineMesh->SetWorldRotation(spawnRotation);
	splineMesh->AttachToComponent(Spline, FAttachmentTransformRules::KeepWorldTransform);
	splineMesh->SetForwardAxis(ESplineMeshAxis::X, true);
	splineMesh->SetStaticMesh(PowerlineMesh);
	splineMesh->SetMaterial(0, PowerlineMesh->GetMaterial(0));

	return splineMesh;
}



