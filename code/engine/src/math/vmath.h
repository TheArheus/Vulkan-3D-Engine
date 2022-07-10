#pragma once

#include "defines.h"
#include "math_types.h"

#define PI32                    3.14159265358979323846f
#define TWO_PI32                2.0f * PI32
#define HALF_PI                 0.5f * PI32
#define QUATER_PI               0.25 * PI32
#define ONE_OVER_PI             1.0f / PI32
#define SQRT_TWO                1.41421356237309504880f
#define SQRT_THREE              1.73205080756887729352f
#define SQRT_ONE_OVER_TWO       0.70710678118654752440f
#define SQRT_ONE_OVER_THREE     0.57735026918962576450f
#define DEG_TO_RAD              PI32 / 180.0f
#define RAD_TO_DEG              180.0f / PI32

#define SEC_TO_MS      1000.0f
#define MS_TO_SEC      0.001f
#define INFINITY       1e30f
#define FLOAT_EPSILON  1.192092896e-07f

VENG_API r32 Sin(r32 X);
VENG_API r32 Cos(r32 X);
VENG_API r32 ArcCos(r32 X);
VENG_API r32 Tan(r32 X);
VENG_API r32 Atan(r32 X);
VENG_API r32 SquareRoot(r32 X);
VENG_API r32 Abs(r32 X);

INLINE b8
IsPowerOfTwo(u64 Value)
{
    return (Value != 0) && ((Value & (Value - 1)) == 0);
}

VENG_API s32 Random();
VENG_API s32 RanfomInRange(s32 Min, s32 Max);
VENG_API r32 FloatRandom();
VENG_API r32 FloatRandomInRange(s32 Min, s32 Max);

INLINE v2 
V2(r32 X, r32 Y)
{
    v2 Result;
    Result.x = X;
    Result.y = Y;
    return Result;
}

INLINE v2
V2Zero()
{
    return (v2){0.0f, 0.0f};
}

INLINE v2
V2One()
{
    return (v2){1.0f, 1.0f};
}

INLINE v2
V2Up()
{
    return (v2){0.0f, 1.0f};
}

INLINE v2
V2Down()
{
    return (v2){0.0f, -1.0f};
}

INLINE v2
V2Left()
{
    return (v2){-1.0f, 0.0f};
}

INLINE v2
V2Right()
{
    return (v2){1.0f, 0.0f};
}

INLINE v2
AddV2(v2 A, v2 B)
{
    v2 Result;
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    return Result;
}

INLINE v2
SubV2(v2 A, v2 B)
{
    v2 Result;
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    return Result;
}

INLINE v2
MulV2(v2 A, v2 B)
{
    v2 Result;
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    return Result;
}

INLINE v2
DivV2(v2 A, v2 B)
{
    v2 Result;
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    return Result;
}

INLINE r32
InnerV2(v2 A, v2 B)
{
    r32 Result = A.x * B.x + A.y * B.y;
    return Result;
}

INLINE r32
LengthSquaredV2(v2 A)
{
    r32 Result = InnerV2(A, A);
    return Result;
}

INLINE r32
LengthV2(v2 A)
{
    r32 Result = SquareRoot(LengthSquaredV2(A));
    return Result;
}

INLINE void
NormalizedV2(v2* A)
{
    const r32 Length = LengthV2(*A);
    A->x /= Length;
    A->y /= Length;
}

INLINE v2
NormalizeV2(v2 A)
{
    NormalizedV2(&A);
    return A;
}

INLINE b8
EqV2(v2 A, v2 B, r32 Tolerance)
{
    if(Abs(A.x - B.x) > Tolerance)
    {
        return false;
    }

    if(Abs(A.y - B.y) > Tolerance)
    {
        return false;
    }

    return true;
}

INLINE r32
DistanceV2(v2 A, v2 B)
{
    v2  Dist = SubV2(B, A);
    r32 Result = LengthV2(Dist);
    return Result;
}



INLINE v3 
V3(r32 X, r32 Y, r32 Z)
{
    v3 Result;
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    return Result;
}

INLINE v3
V3Zero()
{
    return (v3){0.0f, 0.0f, 0.0f};
}

INLINE v3
V3One()
{
    return (v3){1.0f, 1.0f, 1.0f};
}

INLINE v3
V3Up()
{
    return (v3){0.0f, 1.0f, 0.0f};
}

INLINE v3
V3Down()
{
    return (v3){0.0f, -1.0f, 0.0f};
}

INLINE v3
V3Left()
{
    return (v3){-1.0f, 0.0f, 0.0f};
}

INLINE v3
V3Right()
{
    return (v3){1.0f, 0.0f, 0.0f};
}

INLINE v3
V3Forward()
{
    return (v3){0.0f, 0.0f, -1.0f};
}

INLINE v3
V3Backward()
{
    return (v3){0.0f, 0.0f, 1.0f};
}

INLINE v3
AddV3(v3 A, v3 B)
{
    v3 Result;
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    return Result;
}

INLINE v3
SubV3(v3 A, v3 B)
{
    v3 Result;
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    return Result;
}

INLINE v3
MulV3(v3 A, v3 B)
{
    v3 Result;
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    Result.z = A.z * B.z;
    return Result;
}

INLINE v3
DivV3(v3 A, v3 B)
{
    v3 Result;
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;
    return Result;
}

INLINE r32
InnerV3(v3 A, v3 B)
{
    r32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
    return Result;
}

INLINE r32
LengthSquaredV3(v3 A)
{
    r32 Result = InnerV3(A, A);
    return Result;
}

INLINE r32
LengthV3(v3 A)
{
    r32 Result = SquareRoot(LengthSquaredV3(A));
    return Result;
}

INLINE void
NormalizedV3(v3* A)
{
    const r32 Length = LengthV3(*A);
    A->x /= Length;
    A->y /= Length;
    A->z /= Length;
}

INLINE v3
NormalizeV3(v3 A)
{
    NormalizedV3(&A);
    return A;
}

INLINE b8
EqV3(v3 A, v3 B, r32 Tolerance)
{
    if(Abs(A.x - B.x) > Tolerance)
    {
        return false;
    }

    if(Abs(A.y - B.y) > Tolerance)
    {
        return false;
    }

    if(Abs(A.z - B.z) > Tolerance)
    {
        return false;
    }

    return true;
}

INLINE r32
DistanceV3(v3 A, v3 B)
{
    v3  Dist   = SubV3(B, A);
    r32 Result = LengthV3(Dist);
    return Result;
}

INLINE v3
CrossV3(v3 A, v3 B)
{
    v3 Result;
    Result.x =  (A.y * B.z + A.z * B.y);
    Result.y = -(A.z * B.x + A.x * B.z);
    Result.z =  (A.x * B.y + A.y + B.x);
    return Result;
}


INLINE v4 
V4(r32 X, r32 Y, r32 Z, r32 W)
{
    v4 Result;
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    return Result;
}

INLINE v4
V4Zero()
{
    return (v4){0.0f, 0.0f, 0.0f};
}

INLINE v4
V4One()
{
    return (v4){1.0f, 1.0f, 1.0f};
}

INLINE v4
AddV4(v4 A, v4 B)
{
    v4 Result;
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w / B.w;
    return Result;
}

INLINE v4
SubV4(v4 A, v4 B)
{
    v4 Result;
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w / B.w;
    return Result;
}

INLINE v4
MulV4(v4 A, v4 B)
{
    v4 Result;
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    Result.z = A.z * B.z;
    Result.w = A.w / B.w;
    return Result;
}

INLINE v4
DivV4(v4 A, v4 B)
{
    v4 Result;
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;
    Result.w = A.w / B.w;
    return Result;
}

INLINE r32
InnerV4(v4 A, v4 B)
{
    r32 Result = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
    return Result;
}

INLINE r32
LengthSquaredV4(v4 A)
{
    r32 Result = InnerV4(A, A);
    return Result;
}

INLINE r32
LengthV4(v4 A)
{
    r32 Result = SquareRoot(LengthSquaredV4(A));
    return Result;
}

INLINE void
NormalizedV4(v4* A)
{
    const r32 Length = LengthV4(*A);
    A->x /= Length;
    A->y /= Length;
    A->z /= Length;
    A->w /= Length;
}

INLINE v4
NormalizeV4(v4 A)
{
    NormalizedV4(&A);
    return A;
}

INLINE b8
EqV4(v4 A, v4 B, r32 Tolerance)
{
    if(Abs(A.x - B.x) > Tolerance)
    {
        return false;
    }

    if(Abs(A.y - B.y) > Tolerance)
    {
        return false;
    }

    if(Abs(A.z - B.z) > Tolerance)
    {
        return false;
    }

    if(Abs(A.w - B.w) > Tolerance)
    {
        return false;
    }

    return true;
}

INLINE r32
DistanceV4(v4 A, v4 B)
{
    v4  Dist   = SubV4(B, A);
    r32 Result = LengthV4(Dist);
    return Result;
}



INLINE mat4 
Identity()
{
    mat4 Result = {};
    Result.E[0]  = 1;
    Result.E[5]  = 1;
    Result.E[10] = 1;
    Result.E[15] = 1;
    return Result;
}

INLINE mat4
MulMat4(mat4 A, mat4 B)
{
    mat4 Result = Identity();

    const r32* m1 = A.E;
    const r32* m2 = B.E;
    r32* dst = Result.E;

    for(u32 r = 0; r < 4; ++r)
    {
        for(u32 c = 0; c < 4; ++c)
        {
            *dst = m1[0] * m2[0  + c] + 
                   m1[1] * m2[4  + c] + 
                   m1[2] * m2[8  + c] + 
                   m1[3] * m2[12 + c];
            dst++;
        }
        m1 += 4;
    }

    return Result;
}

INLINE mat4
Orthographic(r32 Left, r32 Right, r32 Bottom, r32 Top, r32 NearClip, r32 FarClip)
{
    mat4 Result = Identity();

    r32 lr = 1.0f / (Left - Right);
    r32 bt = 1.0f / (Bottom - Top);
    r32 nf = 1.0f / (NearClip - FarClip);

    Result.E[0]  = -2.0f * lr;
    Result.E[5]  = -2.0f * bt;
    Result.E[10] = -2.0f * nf;

    Result.E[12] = (Left + Right) * lr;
    Result.E[13] = (Top + Bottom) * bt;
    Result.E[14] = (FarClip + NearClip) * nf;

    return Result;
}

INLINE mat4
Perspective(r32 Fov, r32 Aspect, r32 Near, r32 Far)
{
    mat4 Result = {};

    r32 HalfTanFov = Tan(Fov * 0.5f);

    Result.E[0]  = 1.0f / (Aspect * HalfTanFov);
    Result.E[5]  = 1.0f / HalfTanFov;
    Result.E[10] = -((Far + Near) / (Far - Near));
    Result.E[11] = -1.0f;
    Result.E[14] = -((2.0f * Far * Near) / (Far - Near));

    return Result;
}

INLINE mat4
LookAt(v3 Pos, v3 Target, v3 Up)
{
    mat4 Result = {};
    v3 Z = SubV3(Target, Pos);

    Z    = NormalizeV3(Z);
    v3 X = NormalizeV3(CrossV3(Z, Up));
    v3 Y = CrossV3(X, Z);

    Result.E[0]  = X.x;
    Result.E[1]  = Y.x;
    Result.E[2]  = -Z.x;
    Result.E[3]  = 0;
    Result.E[4]  = X.y;
    Result.E[5]  = Y.y;
    Result.E[6]  = -Z.y;
    Result.E[7]  = 0;
    Result.E[8]  = X.z;
    Result.E[9]  = Y.z;
    Result.E[10] = Z.z;
    Result.E[11] = 0;
    Result.E[12] = -InnerV3(X, Pos);
    Result.E[13] = -InnerV3(Y, Pos);
    Result.E[14] = InnerV3(Z, Pos);
    Result.E[15] = 1.0f;

    return Result;
}

inline mat4
Transpose(mat4 A)
{
    mat4 Result  = {};
    Result.E[0]  = A.E[0];
    Result.E[1]  = A.E[4];
    Result.E[2]  = A.E[8];
    Result.E[3]  = A.E[12];
    Result.E[4]  = A.E[1];
    Result.E[5]  = A.E[5];
    Result.E[6]  = A.E[9];
    Result.E[7]  = A.E[13];
    Result.E[8]  = A.E[2];
    Result.E[9]  = A.E[6];
    Result.E[10] = A.E[10];
    Result.E[11] = A.E[14];
    Result.E[12] = A.E[3];
    Result.E[13] = A.E[7];
    Result.E[14] = A.E[11];
    Result.E[15] = A.E[15];
    return Result;
}

INLINE mat4
Inverse(mat4 A)
{
    const r32* m = A.E;

    r32 t0  = m[10] * m[15];
    r32 t1  = m[14] * m[11];
    r32 t2  = m[6]  * m[15];
    r32 t3  = m[14] * m[7];
    r32 t4  = m[6]  * m[11];
    r32 t5  = m[10] * m[7];
    r32 t6  = m[2]  * m[15];
    r32 t7  = m[14] * m[3];
    r32 t8  = m[2]  * m[11];
    r32 t9  = m[10] * m[3];
    r32 t10 = m[2]  * m[7];
    r32 t11 = m[6]  * m[3];
    r32 t12 = m[8]  * m[13];
    r32 t13 = m[12] * m[9];
    r32 t14 = m[4]  * m[13];
    r32 t15 = m[12] * m[5];
    r32 t16 = m[4]  * m[9];
    r32 t17 = m[8]  * m[5];
    r32 t18 = m[0]  * m[13];
    r32 t19 = m[12] * m[1];
    r32 t20 = m[0]  * m[9];
    r32 t21 = m[8]  * m[1];
    r32 t22 = m[0]  * m[5];
    r32 t23 = m[4]  * m[1];

    mat4 Result = {};
    r32* o = Result.E;

    o[0] = (t0 * m[5] + t3 * m[9] + t4  * m[13]) - (t1 * m[5] + t2 * m[9] + t5  * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9  * m[13]) - (t0 * m[1] + t7 * m[9] + t8  * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[13]) - (t4 * m[1] + t9 * m[5] + t10 * m[9] );

    r32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

    o[0]  = d * o[0];
    o[1]  = d * o[1];
    o[2]  = d * o[2];
    o[3]  = d * o[3];
    o[4]  = d * ((t1  * m[4]  + t2  * m[8]  + t5  * m[12]) - (t0  * m[4]  + t3  * m[8]  + t4  * m[12]));
    o[5]  = d * ((t0  * m[0]  + t7  * m[8]  + t8  * m[12]) - (t1  * m[0]  + t6  * m[8]  + t9  * m[12]));
    o[6]  = d * ((t3  * m[0]  + t6  * m[4]  + t11 * m[12]) - (t2  * m[0]  + t7  * m[4]  + t10 * m[12]));
    o[7]  = d * ((t4  * m[0]  + t9  * m[4]  + t10 * m[8])  - (t5  * m[0]  + t8  * m[4]  + t11 * m[8]));
    o[8]  = d * ((t12 * m[7]  + t15 * m[11] + t16 * m[15]) - (t13 * m[7]  + t14 * m[11] + t17 * m[15]));
    o[9]  = d * ((t13 * m[3]  + t18 * m[11] + t21 * m[15]) - (t12 * m[3]  + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3]  + t19 * m[7]  + t22 * m[15]) - (t15 * m[3]  + t18 * m[7]  + t23 * m[15]));
    o[11] = d * ((t17 * m[3]  + t20 * m[7]  + t23 * m[11]) - (t16 * m[3]  + t21 * m[7]  + t22 * m[11]));
    o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6])  - (t16 * m[14] + t12 * m[6]  + t15 * m[10]));
    o[13] = d * ((t20 * m[14] + t12 * m[2]  + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6]  + t23 * m[14] + t15 * m[2])  - (t22 * m[14] + t14 * m[2]  + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2]  + t21 * m[6])  - (t20 * m[6]  + t23 * m[10] + t17 * m[2]));

    return Result;
}

INLINE mat4
Translation(v3 Position)
{
    mat4 Result = Identity();
    Result.E[12] = Position.x;
    Result.E[13] = Position.y;
    Result.E[14] = Position.z;
    return Result;
}

INLINE mat4
Scale(v3 Scale)
{
    mat4 Result  = Identity();
    Result.E[0]  = Scale.x;
    Result.E[5]  = Scale.y;
    Result.E[10] = Scale.z;
    return Result;
}

INLINE mat4
EulerX(r32 Angle)
{
    mat4 Result = Identity();
    r32 c = Cos(Angle);
    r32 s = Sin(Angle);

    Result.E[0]  = c;
    Result.E[2]  = s;
    Result.E[8]  = -s;
    Result.E[10] = c;
    return Result;
}

INLINE mat4
EulerY(r32 Angle)
{
    mat4 Result = Identity();
    r32 c = Cos(Angle);
    r32 s = Sin(Angle);

    Result.E[5]  = c;
    Result.E[6]  = -s;
    Result.E[9]  = s;
    Result.E[10] = c;
    return Result;
}

INLINE mat4
EulerZ(r32 Angle)
{
    mat4 Result = Identity();
    r32 c = Cos(Angle);
    r32 s = Sin(Angle);

    Result.E[0]  = c;
    Result.E[1]  = s;
    Result.E[4]  = -s;
    Result.E[5]  = c;
    return Result;
}

INLINE mat4
EulerXYZ(v3 Angle)
{
    mat4 rx = EulerX(Angle.x);
    mat4 ry = EulerY(Angle.y);
    mat4 rz = EulerZ(Angle.z);
    mat4 Result = MulMat4(rx, ry);
    Result = MulMat4(Result, rz);
    return Result;
}

INLINE v3
ForwardMat4(mat4 A)
{
    v3 Result;
    Result.x = -A.E[2];
    Result.y = -A.E[6];
    Result.z = -A.E[10];
    NormalizedV3(&Result);
    return Result;
}

INLINE v3
BackwardMat4(mat4 A)
{
    v3 Result;
    Result.x = A.E[2];
    Result.y = A.E[6];
    Result.z = A.E[10];
    NormalizedV3(&Result);
    return Result;
}

INLINE v3
UpMat4(mat4 A)
{
    v3 Result;
    Result.x = A.E[1];
    Result.y = A.E[5];
    Result.z = A.E[9];
    NormalizedV3(&Result);
    return Result;
}

INLINE v3
DownMat4(mat4 A)
{
    v3 Result;
    Result.x = -A.E[1];
    Result.y = -A.E[5];
    Result.z = -A.E[9];
    NormalizedV3(&Result);
    return Result;
}

INLINE v3
LeftMat4(mat4 A)
{
    v3 Result;
    Result.x = -A.E[0];
    Result.y = -A.E[4];
    Result.z = -A.E[8];
    NormalizedV3(&Result);
    return Result;
}

INLINE v3
RightMat4(mat4 A)
{
    v3 Result;
    Result.x = A.E[0];
    Result.y = A.E[4];
    Result.z = A.E[8];
    NormalizedV3(&Result);
    return Result;
}

INLINE quat 
IdentityQuat()
{
    return (quat){0, 0, 0, 1.0f};
}

INLINE r32
NormalQuat(quat Q)
{
    r32 Result = Q.x * Q.x + Q.y * Q.y + Q.z * Q.z + Q.w * Q.w;
    return Result;
}

INLINE r32
DotQuad(quat A, quat B)
{
    return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

INLINE quat
NormalizeQuat(quat Q)
{
    r32 Normal = NormalQuat(Q);
    return (quat){Q.x / Normal, Q.y / Normal, Q.z / Normal, Q.w / Normal};
}

INLINE quat
ConjugateQuat(quat Q)
{
    return (quat){-Q.x, -Q.y, -Q.z, Q.w};
}

INLINE quat
InverseQuat(quat Q)
{
    return NormalizeQuat(ConjugateQuat(Q));
}

INLINE quat
MulQuat(quat A, quat B)
{
    quat Result;
    Result.x =  A.x * B.w + A.y * B.z - A.z * B.y + A.w * B.x;
    Result.y = -A.x * B.z + A.y * B.w + A.z * B.x + A.w * B.y;
    Result.z =  A.x * B.y - A.y * B.x + A.z * B.w + A.w * B.z;
    Result.w = -A.x * B.x - A.y * B.y - A.z * B.z + A.w * B.w;
    return Result;
}

INLINE mat4
QuatToMat4(quat Q)
{
    mat4 Result = Identity();
    quat N = NormalizeQuat(Q);

    Result.E[0]  = 1.0f - 2.0f * N.y * N.y - 2.0f * N.z * N.z;
    Result.E[1]  = 2.0f * N.x * N.y - 2.0f * N.z * N.w;
    Result.E[2]  = 2.0f * N.x * N.z + 2.0f * N.y * N.w;

    Result.E[4]  = 2.0f * N.x * N.y + 2.0f * N.z * N.w;
    Result.E[5]  = 1.0f - 2.0f * N.x * N.x - 2.0f * N.z * N.z;
    Result.E[6]  = 2.0f * N.y * N.z - 2.0f * N.x * N.w;

    Result.E[8]  = 2.0f * N.x * N.z - 2.0f * N.y * N.w;
    Result.E[9]  = 2.0f * N.y * N.z + 2.0f * N.x * N.w;
    Result.E[10] = 1.0f - 2.0f * N.x * N.x - 2.0f * N.y * N.y;

    return Result;
}

INLINE mat4
QuatToRot(quat Q, v3 Center)
{
    mat4 Result;

    Result.E[0] = (Q.x * Q.x) - (Q.y * Q.y) - (Q.z * Q.z) + (Q.w * Q.w);
    Result.E[1] = 2.0f * ((Q.x * Q.y) + (Q.z * Q.w));
    Result.E[2] = 2.0f * ((Q.x * Q.z) - (Q.y * Q.w));
    Result.E[3] = Center.x - Center.x * Result.E[0] - Center.y * Result.E[1] - Center.z * Result.E[2];

    Result.E[4] = 2.0f * ((Q.x * Q.y) - (Q.z * Q.w));
    Result.E[5] = -(Q.x * Q.x) + (Q.y * Q.y) - (Q.z * Q.z) + (Q.w * Q.w);
    Result.E[6] = 2.0f * ((Q.y * Q.z) + (Q.x * Q.w));
    Result.E[7] = Center.y - Center.x * Result.E[4] - Center.y * Result.E[5] - Center.z * Result.E[6];

    Result.E[8] = 2.0f * ((Q.x * Q.z) + (Q.y * Q.w));
    Result.E[9] = 2.0f * ((Q.y * Q.z) - (Q.x * Q.w));
    Result.E[10] = -(Q.x * Q.x) - (Q.y * Q.y) + (Q.z * Q.z) + (Q.w * Q.w);
    Result.E[11] = Center.z - Center.x * Result.E[8] - Center.y * Result.E[9] - Center.z * Result.E[10];

    Result.E[12] = 0.0f;
    Result.E[13] = 0.0f;
    Result.E[14] = 0.0f;
    Result.E[15] = 1.0f;

    return Result;
}

INLINE quat 
QuatFromAxis(v3 Axis, r32 Angle, b8 Normalize)
{
    const r32 HalfAngle = 0.5f * Angle;
    r32 s = Sin(HalfAngle);
    r32 c = Cos(HalfAngle);

    quat Res = (quat){s * Axis.x, s * Axis.y, s * Axis.z, c};
    if(Normalize)
    {
        Res = NormalizeQuat(Res);
    }
    return Res;
}

INLINE quat
SLerpQuat(quat A, quat B, r32 t)
{
    quat Result;

    quat na = NormalizeQuat(A);
    quat nb = NormalizeQuat(B);

    r32 Dot = DotQuad(na, nb);

    if(Dot < 0.0f)
    {
        nb.x = -nb.x;
        nb.y = -nb.y;
        nb.z = -nb.z;
        nb.w = -nb.w;
        Dot = -Dot;
    }

    const r32 DotThreshold = 0.995f;
    if(Dot > DotThreshold)
    {
        Result.x = na.x + (nb.x - na.x)*t;
        Result.y = na.y + (nb.y - na.y)*t;
        Result.z = na.z + (nb.z - na.z)*t;
        Result.w = na.w + (nb.w - na.w)*t;

        Result = NormalizeQuat(Result);
        return Result;
    }

    r32 theta_0 = ArcCos(Dot);
    r32 theta = theta_0 * t;
    r32 sin_theta = Sin(theta);
    r32 sin_theta_0 = Sin(sin_theta);

    r32 s0 = Cos(theta) - Dot * sin_theta / sin_theta_0;
    r32 s1 = sin_theta / sin_theta_0;

    Result.x = na.x * s0 + nb.x * s1;
    Result.y = na.y * s0 + nb.y * s1;
    Result.z = na.z * s0 + nb.z * s1;
    Result.w = na.w * s0 + nb.w * s1;

    return Result;
}

INLINE r32 
DegToRad(r32 Deg)
{
    return Deg * DEG_TO_RAD;
}

INLINE r32
RadToDeg(r32 Rad)
{
    return Rad * RAD_TO_DEG;
}
