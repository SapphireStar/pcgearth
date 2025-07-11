#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NoiseLibrary.generated.h"

/**
 * Static function library for Simplex noise generation
 */
UCLASS()
class PCG_API UNoiseLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Blueprint callable function for 3D noise
    UFUNCTION(BlueprintCallable, Category = "Noise")
    static float SimplexNoise3D(FVector Point, int32 Seed = 0);

    // C++ only functions
    static float Evaluate3D(const FVector& Point, int32 Seed = 0);
    static float Evaluate3D(float X, float Y, float Z, int32 Seed = 0);
    static float Evaluate(FVector Point, int32 Seed = 0);

private:
    // Internal helper functions
    static void InitializePermutationTable(TArray<int32>& Random, int32 Seed);
    static double Dot(const int32* G, double X, double Y, double Z);
    static int32 FastFloor(double X);
    static void UnpackLittleUint32(int32 Value, uint8* Buffer);

    // Constants
    static constexpr int32 RandomSize = 256;
    static constexpr double Sqrt3 = 1.7320508075688772935;
    static constexpr double F3 = 1.0/3.0;
    static constexpr double G3 = 1.0/6.0;

    // Static data
    static const int32 Source[256];
    static const int32 Grad3[12][3];
    
    // Thread-safe permutation table cache
    static TMap<int32, TArray<int32>> PermutationCache;
    static FCriticalSection CacheMutex;
};


// Static member definitions


inline float UNoiseLibrary::SimplexNoise3D(FVector Point, int32 Seed)
{
    return Evaluate3D(Point, Seed);
}

inline float UNoiseLibrary::Evaluate3D(const FVector& Point, int32 Seed)
{
    return Evaluate3D(Point.X, Point.Y, Point.Z, Seed);
}

inline float UNoiseLibrary::Evaluate3D(float X, float Y, float Z, int32 Seed)
{
    // Get or create permutation table for this seed
    TArray<int32> Random;
    {
        FScopeLock Lock(&CacheMutex);
        if (TArray<int32>* CachedRandom = PermutationCache.Find(Seed))
        {
            Random = *CachedRandom;
        }
        else
        {
            InitializePermutationTable(Random, Seed);
            PermutationCache.Add(Seed, Random);
        }
    }

    double x = static_cast<double>(X);
    double y = static_cast<double>(Y);
    double z = static_cast<double>(Z);
    
    double n0 = 0, n1 = 0, n2 = 0, n3 = 0;

    // Skew the input space to determine which simplex cell we're in
    double s = (x + y + z) * F3;

    int32 i = FastFloor(x + s);
    int32 j = FastFloor(y + s);
    int32 k = FastFloor(z + s);

    double t = (i + j + k) * G3;

    // The x,y,z distances from the cell origin
    double x0 = x - (i - t);
    double y0 = y - (j - t);
    double z0 = z - (k - t);

    // Determine which simplex we are in
    int32 i1, j1, k1; // Offsets for second corner of simplex
    int32 i2, j2, k2; // Offsets for third corner of simplex

    if (x0 >= y0)
    {
        if (y0 >= z0)
        {
            // X Y Z order
            i1 = 1; j1 = 0; k1 = 0;
            i2 = 1; j2 = 1; k2 = 0;
        }
        else if (x0 >= z0)
        {
            // X Z Y order
            i1 = 1; j1 = 0; k1 = 0;
            i2 = 1; j2 = 0; k2 = 1;
        }
        else
        {
            // Z X Y order
            i1 = 0; j1 = 0; k1 = 1;
            i2 = 1; j2 = 0; k2 = 1;
        }
    }
    else
    {
        if (y0 < z0)
        {
            // Z Y X order
            i1 = 0; j1 = 0; k1 = 1;
            i2 = 0; j2 = 1; k2 = 1;
        }
        else if (x0 < z0)
        {
            // Y Z X order
            i1 = 0; j1 = 1; k1 = 0;
            i2 = 0; j2 = 1; k2 = 1;
        }
        else
        {
            // Y X Z order
            i1 = 0; j1 = 1; k1 = 0;
            i2 = 1; j2 = 1; k2 = 0;
        }
    }

    // Offsets for corners in (x,y,z) coords
    double x1 = x0 - i1 + G3;
    double y1 = y0 - j1 + G3;
    double z1 = z0 - k1 + G3;

    double x2 = x0 - i2 + F3;
    double y2 = y0 - j2 + F3;
    double z2 = z0 - k2 + F3;

    double x3 = x0 - 0.5;
    double y3 = y0 - 0.5;
    double z3 = z0 - 0.5;

    // Work out the hashed gradient indices of the four simplex corners
    int32 ii = i & 0xff;
    int32 jj = j & 0xff;
    int32 kk = k & 0xff;

    // Calculate the contribution from the four corners
    double t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if (t0 > 0)
    {
        t0 *= t0;
        int32 gi0 = Random[ii + Random[jj + Random[kk]]] % 12;
        n0 = t0 * t0 * Dot(Grad3[gi0], x0, y0, z0);
    }

    double t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if (t1 > 0)
    {
        t1 *= t1;
        int32 gi1 = Random[ii + i1 + Random[jj + j1 + Random[kk + k1]]] % 12;
        n1 = t1 * t1 * Dot(Grad3[gi1], x1, y1, z1);
    }

    double t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if (t2 > 0)
    {
        t2 *= t2;
        int32 gi2 = Random[ii + i2 + Random[jj + j2 + Random[kk + k2]]] % 12;
        n2 = t2 * t2 * Dot(Grad3[gi2], x2, y2, z2);
    }

    double t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if (t3 > 0)
    {
        t3 *= t3;
        int32 gi3 = Random[ii + 1 + Random[jj + 1 + Random[kk + 1]]] % 12;
        n3 = t3 * t3 * Dot(Grad3[gi3], x3, y3, z3);
    }

    // Add contributions from each corner to get the final noise value
    // The result is scaled to stay just inside [-1,1]
    return static_cast<float>((n0 + n1 + n2 + n3) * 32.0);
}

inline float UNoiseLibrary::Evaluate(FVector Point, int32 Seed)
{
    float noiseValue = (Evaluate3D(Point, Seed) + 1) * .5f;
    return noiseValue;
}

inline void UNoiseLibrary::InitializePermutationTable(TArray<int32>& Random, int32 Seed)
{
    Random.SetNum(RandomSize * 2);

    if (Seed != 0)
    {
        // Shuffle the array using the given seed
        uint8 F[4];
        UnpackLittleUint32(Seed, F);

        for (int32 i = 0; i < 256; i++)
        {
            Random[i] = Source[i] ^ F[0];
            Random[i] ^= F[1];
            Random[i] ^= F[2];
            Random[i] ^= F[3];

            Random[i + RandomSize] = Random[i];
        }
    }
    else
    {
        for (int32 i = 0; i < RandomSize; i++)
        {
            Random[i + RandomSize] = Random[i] = Source[i];
        }
    }
}

inline double UNoiseLibrary::Dot(const int32* G, double X, double Y, double Z)
{
    return G[0] * X + G[1] * Y + G[2] * Z;
}

inline int32 UNoiseLibrary::FastFloor(double X)
{
    return X >= 0 ? static_cast<int32>(X) : static_cast<int32>(X) - 1;
}

inline void UNoiseLibrary::UnpackLittleUint32(int32 Value, uint8* Buffer)
{
    Buffer[0] = static_cast<uint8>(Value & 0x00ff);
    Buffer[1] = static_cast<uint8>((Value & 0xff00) >> 8);
    Buffer[2] = static_cast<uint8>((Value & 0x00ff0000) >> 16);
    Buffer[3] = static_cast<uint8>((Value & 0xff000000) >> 24);
}