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
	TArray<UStaticMeshComponent*> PowerlineMeshesL;
	
	if (bInitialPolesSpawned)
	{
		for (int i=0; i < Spline->GetNumberOfSplinePoints(); i++)
		{
			PowerlineMeshesL.Add(AddPowerlineMeshToSpline(i));
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
			PowerlineMeshesL.Add(AddPowerlineMeshToSpline(i));
		}
	}

	return PowerlineMeshesL;
}

void APowergrid::UpdatePowerlines()
{
	DestroyPowerlinesAndCables();
	PowerPoleMeshes = SpawnPowerlineMeshesAlongSpline();
	
	for (int i=0; i< PowerPoleMeshes.Num() - 1; i++)
	{
		UStaticMeshComponent* MeshL = PowerPoleMeshes[i];
		int MeshIndexL = i;
		for (auto& name : MeshL->GetAllSocketNames())
		{
			FName SocketNameL = name;
			if (PowerPoleMeshes[MeshIndexL + 1] != nullptr )
			{
				UCableComponent* CableL = AddCableComponent(MeshL, SocketNameL);
				Cables.Add(CableL);

				for (auto &CableInfo: CableInfos)
				{
					if (CableInfo.CableSocket == SocketNameL)
					{
						FVector StartPointL = GetActorTransform().InverseTransformPosition(PowerPoleMeshes[MeshIndexL]->GetSocketLocation(SocketNameL));
						FVector EndPointL = GetActorTransform().InverseTransformPosition(PowerPoleMeshes[MeshIndexL + 1]->GetSocketLocation(SocketNameL));
						float CableLengthL = FVector::Distance(StartPointL, EndPointL);
						FCableInfo CableInfoL = FCableInfo(CableL, MeshIndexL, SocketNameL, CableLengthL, true, EndPointL);
						SetCableProperties(CableInfoL);
					}
				}
			}
		}
	}
}

void APowergrid::InitializePowerlineParams(UStaticMesh* PowerlineMeshToSet, int PolesNumToSet, float CableLengthToSet)
{
	PowerlineMesh = PowerlineMeshToSet;
	InitialPolesNum = PolesNumToSet;
	CableLength = CableLengthToSet;
}

void APowergrid::AddSplinePoints(const FVector& LineStart, const FVector& LineEnd)
{
	FVector DirectionL = (LineEnd - LineStart);
	FVector SpawnLocationL = FVector::ZeroVector;
	DirectionL.Normalize();
	float LineLengthL = RemainingLineLength + FVector::Dist(LineStart, LineEnd);
	int PolesL = FMath::TruncToInt(LineLengthL/CableLength);

	for (int i = 1; i <= PolesL; i++)
	{
		SpawnLocationL = LineStart - (DirectionL * RemainingLineLength) + (DirectionL * CableLength * i);
		Spline->AddSplinePoint(SpawnLocationL, ESplineCoordinateSpace::World);
	}
	RemainingLineLength = LineLengthL - (PolesL * CableLength);
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
	UCableComponent * CableL = CableInfoToSet.CableRef;

	CableL->bAttachEnd = true;
	CableL->EndLocation = CableInfoToSet.EndPoint;
	CableL->CableLength = CableInfoToSet.CableLength;
	CableL->SolverIterations = 10;
	CableL->bEnableStiffness = CableInfoToSet.bEnableCableStiffness;
}

UCableComponent* APowergrid::AddCableComponent(UStaticMeshComponent* MeshComponent, FName AttachSocketName)
{
	FVector SocketLocationL = GetActorTransform().InverseTransformPosition(MeshComponent->GetSocketLocation(AttachSocketName));

	UCableComponent* CableComponentL = NewObject<UCableComponent>(this);
	CableComponentL->RegisterComponent();
	CableComponentL->SetMobility(EComponentMobility::Movable);
	CableComponentL->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CableComponentL->SetRelativeLocation(SocketLocationL);
	CableComponentL->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
	CableComponentL->bAttachEnd = false;
	
	return CableComponentL;
}

USplineMeshComponent* APowergrid::AddPowerlineMeshToSpline(int SplineIndex)
{
	if (!PowerlineMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spline mesh not assigned"));
		return nullptr;
	}
	
	USplineMeshComponent* SplineMeshComponentL = NewObject<USplineMeshComponent>(this);
	SplineMeshComponentL->RegisterComponent();
	SplineMeshComponentL->SetMobility(EComponentMobility::Movable);
	
	FVector SpawnLocationL = Spline->GetLocationAtSplinePoint(SplineIndex,ESplineCoordinateSpace::World);
	FRotator SpawnRotatorL = Spline->GetRotationAtSplinePoint(SplineIndex,ESplineCoordinateSpace::World);
	
	SplineMeshComponentL->SetWorldLocation(SpawnLocationL);
	SplineMeshComponentL->SetWorldRotation(SpawnRotatorL);
	SplineMeshComponentL->AttachToComponent(Spline, FAttachmentTransformRules::KeepWorldTransform);
	SplineMeshComponentL->SetForwardAxis(ESplineMeshAxis::X, true);
	SplineMeshComponentL->SetStaticMesh(PowerlineMesh);
	SplineMeshComponentL->SetMaterial(0, PowerlineMesh->GetMaterial(0));

	return SplineMeshComponentL;
}



