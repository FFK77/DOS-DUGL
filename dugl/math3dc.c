#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "DUGL.h"

#define MDEG_TO_RAD_STEP     3.14159265358979323846 / 180.0
#define THETA_SMALL          0.0000001f
#ifndef sincosf
#define sincosf(x, ps, pc) { double ressind, rescosd; sincos(x, &ressind, &rescosd); *ps = (float)ressind; *pc=(float)rescosd; }
#endif // sincosf

DVEC4 zeroDVEC4 __attribute__ ((aligned (16))) = { {0.0f, 0.0f, 0.0f, 0.0f } };

void *CreateDVEC4() {
    return malloc(sizeof(DVEC4));
}

DVEC4 *CreateInitDVEC4(float x, float y, float z, float d) {
    DVEC4 *vec4 = (DVEC4 *)malloc(sizeof(DVEC4));
    if (vec4 != NULL) {
        vec4->x = x; vec4->y = y; vec4->z = z; vec4->d = d;
    }
    return vec4;
}

void *CreateDVEC4Array(int count) {
    void *resDVEC4Array = malloc(sizeof(DVEC4)*(size_t)(count));
    if (resDVEC4Array != NULL)
        StoreDVEC4(resDVEC4Array, &zeroDVEC4, count);
    return resDVEC4Array;
}

DVEC4 *CreateInitDVEC4Array(DVEC4 *vec4init, int count) {
    DVEC4 *vec4array = (DVEC4 *)malloc(sizeof(DVEC4)*count);
    return vec4array;
}

DVEC4i *CreateInitDVEC4i(int x, int y, int z, int d) {
    DVEC4i *vec4 = (DVEC4i *)malloc(sizeof(DVEC4i));
    if (vec4 != NULL) {
        vec4->x = x; vec4->y = y; vec4->z = z; vec4->d = d;
    }
    return vec4;
}

DVEC4i *CreateInitDVEC4iArray(DVEC4i *vec4init, int count) {
    DVEC4i *vec4array = (DVEC4i *)malloc(sizeof(DVEC4i)*count);
    if (vec4array != NULL) {
        StoreDVEC4(vec4array, vec4init, count);
    }
    return vec4array;
}

void DestroyDVEC4(void *vec4) {
    free(vec4);
}

void *CreateDVEC2() {
    return malloc(sizeof(DVEC2));
}

void *CreateDVEC2Array(int count) {
    return malloc(sizeof(DVEC2)*(size_t)(count));
}

void DestroyDVEC2(void *vec2) {
    free(vec2);
}

// DMatrix4 ==========
DMatrix4 identityDMatrix4 __attribute__ ((aligned (16))) = {
    .raw = {
         1.0f, 0.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f, 0.0f,
         0.0f, 0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 0.0f, 1.0f}
};

DMatrix4 *CreateDMatrix4() {
    DMatrix4 *resMat4 = (DMatrix4 *)malloc(sizeof(DMatrix4));
    if (resMat4 != NULL)
        StoreDVEC4(resMat4, &zeroDVEC4, 4);
    return resMat4;
}

DMatrix4 *CreateDMatrix4Array(size_t count) {
    DMatrix4 *resArrayMat4 = (DMatrix4 *)malloc(sizeof(DMatrix4));
    if (resArrayMat4 != NULL)
        StoreDVEC4(resArrayMat4, &zeroDVEC4, count*4);
    return (DMatrix4 *)malloc(sizeof(DMatrix4)*count);
}

DMatrix4 *GetIdentityDMatrix4(DMatrix4 *mat4x4Dst) {
    CopyDVEC4(mat4x4Dst, &identityDMatrix4, 4);
    return mat4x4Dst;
}

DMatrix4 *GetLookAtDMatrix4Val(DMatrix4 *mat4x4, float eye_x, float eye_y, float eye_z, float center_x, float center_y, float center_z, float up_x, float up_y, float up_z) {
    DVEC4 *varray = (DVEC4 *)CreateDVEC4Array(3);
    if (varray == NULL)
        return mat4x4;
    DVEC4 *eye = &varray[0];
    DVEC4 *center = &varray[1];
    DVEC4 *up = &varray[2];
    eye->x = eye_x; eye->y = eye_y; eye->z = eye_z; eye->d = 0.0f;
    center->x = center_x; center->y = center_y; center->z = center_z; center->d = 0.0f;
    up->x = up_x; up->y = up_y; up->z = up_z; up->d = 0.0f;

    GetLookAtDMatrix4(mat4x4, eye, center, up);

    DestroyDVEC4(varray);
    return mat4x4;
}

DMatrix4 *GetLookAtDMatrix4(DMatrix4 *mat4x4, DVEC4 *eye, DVEC4 *center, DVEC4 *up) {
    DVEC4 *varray = (DVEC4*)CreateDVEC4Array(4);
    if (varray == NULL)
        return mat4x4;
    DMatrix4 *vTransMat = CreateDMatrix4();
    if (vTransMat == NULL) {
        DestroyDVEC4(varray);
        return mat4x4;
    }

    DVEC4 *negEye = &varray[0];
    DVEC4 *n = &varray[1];
    DVEC4 *u = &varray[2];
    DVEC4 *s = &varray[3];

    GetTranslateDMatrix4(vTransMat, MulValDVEC4Res(eye, -1.0f, negEye));

    *n = *eye;
    SubNormalizeDVEC4(n, center);

    CrossNormalizeDVEC4(up, n, s);

    CrossNormalizeDVEC4(n, s, u);

    mat4x4->raw[0] = s->x;  mat4x4->raw[4] = s->y;  mat4x4->raw[8]  = s->z;  mat4x4->raw[12] = 0.0f;
    mat4x4->raw[1] = u->x;  mat4x4->raw[5] = u->y;  mat4x4->raw[9]  = u->z;  mat4x4->raw[13] = 0.0f;
    mat4x4->raw[2] = n->x;  mat4x4->raw[6] = n->y;  mat4x4->raw[10] = n->z;  mat4x4->raw[14] = 0.0f;
    mat4x4->raw[3] = 0.0f;  mat4x4->raw[7] = 0.0f;  mat4x4->raw[11] = 0.0f;  mat4x4->raw[15] = 1.0f;

    DMatrix4MulDMatrix4(mat4x4, vTransMat);

    DestroyDVEC4(varray);
    DestroyDMatrix4(vTransMat);

    return mat4x4;
}

DMatrix4 *GetPerspectiveDMatrix4(DMatrix4 *mat4x4, float fov, float aspect, float znear, float zfar) {

    float y = tanf(fov * MDEG_TO_RAD_STEP / 2.0f);
    float x = y * aspect;
    float deltaFN = (zfar - znear);
    float zFNRat = -(zfar + znear) / deltaFN;
    float zFNVol = -(2.0f * zfar * znear) / deltaFN;


    mat4x4->raw[0] = 1.0f/x; mat4x4->raw[4] = 0.0f;   mat4x4->raw[8]  = 0.0f;   mat4x4->raw[12] = 0.0f;
    mat4x4->raw[1] = 0.0f;   mat4x4->raw[5] = 1.0f/y; mat4x4->raw[9]  = 0.0f;   mat4x4->raw[13] = 0.0f;
    mat4x4->raw[2] = 0.0f;   mat4x4->raw[6] = 0.0f;   mat4x4->raw[10] = zFNRat; mat4x4->raw[14] = zFNVol;
    mat4x4->raw[3] = 0.0f;   mat4x4->raw[7] = 0.0f;   mat4x4->raw[11] = -1.0f;  mat4x4->raw[15] = 0.0f;

    return mat4x4;
}

DMatrix4 *GetOrthoDMatrix4(DMatrix4 *mat4x4, float left, float right, float bottom, float top, float znear, float zfar) {
    float x = 2.0f / (right - left);
    float y = 2.0f / (top - bottom);
    float z = -2.0f / (zfar - znear);
    float tx = - ((right + left) / (right - left));
    float ty = - ((top + bottom) / (top - bottom));
    float tz = - ((zfar + znear) / (zfar - znear));

    mat4x4->raw[0] = x;      mat4x4->raw[4] = 0.0f;   mat4x4->raw[8]  = 0.0f;   mat4x4->raw[12] = -tx;
    mat4x4->raw[1] = 0.0f;   mat4x4->raw[5] = y;      mat4x4->raw[9]  = 0.0f;   mat4x4->raw[13] = -ty;
    mat4x4->raw[2] = 0.0f;   mat4x4->raw[6] = 0.0f;   mat4x4->raw[10] = z;      mat4x4->raw[14] = -tz;
    mat4x4->raw[3] = 0.0f;   mat4x4->raw[7] = 0.0f;   mat4x4->raw[11] = 0.0f;   mat4x4->raw[15] = 0.0f;

    return mat4x4;
}

DMatrix4 *GetViewDMatrix4(DMatrix4 *mat4x4, DgView *view, float startX, float endX, float startY, float endY)
{
    float propWidth = endX - startX;
    float propHeight = endY - startY;

    if (propWidth == 0.0f || propHeight == 0.0f)
        return mat4x4;

    int viewWidth = view->MaxX - view->MinX;
    int viewHeight = view->MaxY - view->MinY;

    DMatrix4 *vmatrix = CreateDMatrix4();

    DMatrix4MulDMatrix4(GetTranslateDMatrix4Val(mat4x4, (float)(view->MinX), (float)(view->MinY), 0.0f),
                       GetScaleDMatrix4Val(vmatrix, (float)(viewWidth), (float)(viewHeight), 1.0f));

    if (startX != 0.0f || startY != 0.0f) {
        DMatrix4MulDMatrix4(mat4x4, GetTranslateDMatrix4Val(vmatrix, -startX/propWidth, -startY/propHeight, 0.0f));
    }

    DMatrix4MulDMatrix4(mat4x4, GetScaleDMatrix4Val(vmatrix, 0.5f / propWidth, 0.5f / propHeight, 1.0f));

    DMatrix4MulDMatrix4(mat4x4, GetTranslateDMatrix4Val(vmatrix, 1.0f, 1.0f, 0.0f));

    DestroyDMatrix4(vmatrix);

    return mat4x4;
}


DMatrix4 *GetRotDMatrix4(DMatrix4 *FMG, float Rx, float Ry, float Rz) {
    float rx=MDEG_TO_RAD_STEP*Rx;
    float ry=MDEG_TO_RAD_STEP*Ry;
    float rz=MDEG_TO_RAD_STEP*Rz;
    float cx,sx,cy,sy,cz,sz,sx_sy,cx_sy;
    sincosf(rx, &sx, &cx);
    sincosf(ry, &sy, &cy);
    sincosf(rz, &sz, &cz);
    sx_sy=sx*sy;
    cx_sy=cx*sy;

    GetIdentityDMatrix4(FMG);
    FMG->rows[0].v[0]=cy*cz;
    FMG->rows[1].v[0]=cy*sz;
    FMG->rows[2].v[0]=-sy;
    FMG->rows[0].v[1]=(sx_sy*cz)-(cx*sz);
    FMG->rows[1].v[1]=(sx_sy*sz)+(cx*cz);
    FMG->rows[2].v[1]=sx*cy;
    FMG->rows[0].v[2]=(cx_sy*cz)+(sx*sz);
    FMG->rows[1].v[2]=(cx_sy*sz)-(sx*cz);
    FMG->rows[2].v[2]=cx*cy;
    return FMG;
}


DMatrix4 *GetXRotDMatrix4(DMatrix4 *FMX, float Rx) {
   float rx=MDEG_TO_RAD_STEP*Rx;
   float cosr;
   float sinr;
   sincosf(Rx, &sinr, &cosr);
   GetIdentityDMatrix4(FMX);
   FMX->rows[1].v[1]=cosr; FMX->rows[1].v[2]=-sinr;
   FMX->rows[2].v[1]=sinr; FMX->rows[2].v[2]=cosr;
   return FMX;
}

DMatrix4 *GetYRotDMatrix4(DMatrix4 *FMY, float Ry) {
   float ry=MDEG_TO_RAD_STEP*Ry;
   float cosr,sinr;
   sincosf(Ry,&sinr,&cosr);
   GetIdentityDMatrix4(FMY);
   FMY->rows[0].v[0]=cosr; FMY->rows[0].v[2]=sinr;
   FMY->rows[2].v[0]=-sinr; FMY->rows[2].v[2]=cosr;
   return FMY;
}

DMatrix4 *GetZRotDMatrix4(DMatrix4 *FMZ, float Rz) {
   float rz=MDEG_TO_RAD_STEP*Rz;
   float cosr,sinr;
   sincosf(Rz,&sinr,&cosr);
   GetIdentityDMatrix4(FMZ);
   FMZ->rows[0].v[0]=cosr; FMZ->rows[0].v[1]=-sinr;
   FMZ->rows[1].v[0]=sinr; FMZ->rows[1].v[1]=cosr;
   return FMZ;
}

DMatrix4 *GetTranslateDMatrix4(DMatrix4 *mat4x4Trans, DVEC4 *vecTrans) {
   GetIdentityDMatrix4(mat4x4Trans);
   mat4x4Trans->rows[3] = *vecTrans;
   return mat4x4Trans;
}

DMatrix4 *GetScaleDMatrix4(DMatrix4 *mat4x4, DVEC4 *vecScale) {
   GetIdentityDMatrix4(mat4x4);
   mat4x4->rows[0].x = vecScale->x;
   mat4x4->rows[1].y = vecScale->y;
   mat4x4->rows[2].z = vecScale->z;
   return mat4x4;
}

DMatrix4 *GetTranslateDMatrix4Val(DMatrix4 *mat4x4Trans, float tx, float ty, float tz) {
   GetIdentityDMatrix4(mat4x4Trans);
   mat4x4Trans->rows[3].x = tx;
   mat4x4Trans->rows[3].y = ty;
   mat4x4Trans->rows[3].z = tz;
   return mat4x4Trans;
}

DMatrix4 *GetScaleDMatrix4Val(DMatrix4 *mat4x4Trans, float sx, float sy, float sz) {
   GetIdentityDMatrix4(mat4x4Trans);
   mat4x4Trans->rows[0].x = sx;
   mat4x4Trans->rows[1].y = sy;
   mat4x4Trans->rows[2].z = sz;
   return mat4x4Trans;
}

void DestroyDMatrix4(DMatrix4 *matrix4) {
    free(matrix4);
}

bool IntersectRayPlane(DVEC4 *plane, DVEC4 *raypos, DVEC4 *raydir) {
    float ddir = 0.0f;
    float dpos = 0.0f;
    DotDVEC4(plane, raydir, &ddir);
    // plane normal and ray dir should in reverse direction or no interesection occur
    if (ddir <= 0.00005f ) {
        return false;
    }

    float t = -(*DotDVEC4(plane, raypos, &dpos)+plane->d) / ddir;

    if (t < 0.0f)
        return false;

    return true;
}

bool IntersectRayPlaneRes(DVEC4 *plane, DVEC4 *raypos, DVEC4 *raydir, DVEC4 *intrscPos) {
    float ddir = 0.0f;
    float dpos = 0.0f;
    DotDVEC4(plane, raydir, &ddir);

    if (ddir <= 0.00005f ) {
        return false;
    }

    float t = -(*DotDVEC4(plane, raypos, &dpos)+plane->d) / ddir;

    if (t < 0.0f)
        return false;

    RayProjectDVEC4Res(t, raypos, raydir, intrscPos);

    return true;
}

float *DistanceDVEC4(DVEC4 *v1, DVEC4 *v2, float *distanceRes) {
    *distanceRes = sqrtf(((v1->x-v2->x)*(v1->x-v2->x))+((v1->y-v2->y)*(v1->y-v2->y))+((v1->z-v2->z)*(v1->z-v2->z)));
    return distanceRes;
}


float *DistancePow2DVEC4(DVEC4 *v1, DVEC4 *v2, float *distancePow2Res) {
    *distancePow2Res = ((v1->x-v2->x)*(v1->x-v2->x))+((v1->y-v2->y)*(v1->y-v2->y))+((v1->z-v2->z)*(v1->z-v2->z));
    return distancePow2Res;
}

// v1.x*v2.x + v1.y*v2.y + v1.z*v2.z
float *DotDVEC4(DVEC4 *v1, DVEC4 *v2, float *dotRes) {
    *dotRes = (v1->x*v2->x) + (v1->y*v2->y) + (v1->z*v2->z);
    return dotRes;
}

float *DotNormalizeDVEC4(DVEC4 *v1, DVEC4 *v2, float *dotRes) {
    DVEC4 lv1, lv2;
    return DotDVEC4(NormalizeDVEC4Res(v1, &lv1), NormalizeDVEC4Res(v2, &lv2), dotRes);
}

float *LengthDVEC4(DVEC4 *vec4, float *lengthRes) {
    *lengthRes = sqrtf((vec4->x*vec4->x) + (vec4->y*vec4->y) + (vec4->z*vec4->z));
    return lengthRes;
}

DVEC4 *NormalizeDVEC4(DVEC4 *vec4) {
    float lengthPow2Vec4 = (vec4->x*vec4->x) + (vec4->y*vec4->y) + (vec4->z*vec4->z);
    if (lengthPow2Vec4 > 0.0f) {
        float InvLengthVec4 = 1.0f / sqrtf(lengthPow2Vec4);
        vec4->x *= InvLengthVec4;
        vec4->y *= InvLengthVec4;
        vec4->z *= InvLengthVec4;
    }
    return vec4;
}


DVEC4 *NormalizeDVEC4Res(DVEC4 *vec4, DVEC4 *nvres) {
    float lengthPow2Vec4 = (vec4->x*vec4->x) + (vec4->y*vec4->y) + (vec4->z*vec4->z);
    if (lengthPow2Vec4 > 0.0f) {
        float InvLengthVec4 = 1.0f / sqrtf(lengthPow2Vec4);
        nvres->x = vec4->x * InvLengthVec4;
        nvres->y = vec4->y * InvLengthVec4;
        nvres->z = vec4->z * InvLengthVec4;
    } else {
        nvres->x = 0.0f;
        nvres->y = 0.0f;
        nvres->z = 0.0f;
        nvres->d = 0.0f;
    }
    return nvres;
}

// v1 = Normalize(v1 - v2)
DVEC4 *SubNormalizeDVEC4(DVEC4 *v1, DVEC4 *v2) {
    v1->x -= v2->x;
    v1->y -= v2->y;
    v1->z -= v2->z;
    return NormalizeDVEC4(v1);
}

// vresSubNormalize = Normalize(v1 - v2)
DVEC4 *SubNormalizeDVEC4Res(DVEC4 *v1, DVEC4 *v2, DVEC4 *vresSubNormalize) {
    vresSubNormalize->x = v1->x - v2->x;
    vresSubNormalize->y = v1->y - v2->y;
    vresSubNormalize->z = v1->z - v2->z;
    //vresSubNormalize->d = v1->d - v2->d;
    return NormalizeDVEC4(vresSubNormalize);
}


// ([v1.y * v2.z - v1.z * v2.y],  [v1.z * v2.x - v1.x * v2.z],  [v1.x * v2.y - v1.y * v2.x])
DVEC4 *CrossDVEC4(DVEC4 *v1, DVEC4 *v2, DVEC4 *vcrossRes) {
    vcrossRes->x = v1->y * v2->z - v1->z * v2->y;
    vcrossRes->y = v1->z * v2->x - v1->x * v2->z;
    vcrossRes->z = v1->x * v2->y - v1->y * v2->x;
    vcrossRes->d = 0.0f;
    return vcrossRes;
}

// Normalize([v1.y * v2.z - v1.z * v2.y],  [v1.z * v2.x - v1.x * v2.z],  [v1.x * v2.y - v1.y * v2.x])
DVEC4 *CrossNormalizeDVEC4(DVEC4 *v1, DVEC4 *v2, DVEC4 *vcrossRes) {
    vcrossRes->x = v1->y * v2->z - v1->z * v2->y;
    vcrossRes->y = v1->z * v2->x - v1->x * v2->z;
    vcrossRes->z = v1->x * v2->y - v1->y * v2->x;
    vcrossRes->d = 0.0f;
    return NormalizeDVEC4(vcrossRes);
}

// build plane equation from (v1, v2, v3) giving the equation (a*x)+(b*y)+(c*z)+d = 0 in vPlaneRes
// CrossNormalize(v3-v2, v2-v1, vPlaneNorm) then compute  d = - ((vPlaneNorm.x*v1.x) + (vPlaneNorm.y*v1.y) + (vPlaneNorm.z*v1.z))
DVEC4 *GetPlaneDVEC4(DVEC4 *v1, DVEC4 *v2, DVEC4 *v3, DVEC4 *vPlaneRes) {
    DVEC4 v3subv2;
    DVEC4 v2subv1;
    CrossNormalizeDVEC4(SubDVEC4Res(v3, v2, &v3subv2), SubDVEC4Res(v2, v1, &v2subv1), vPlaneRes);
    vPlaneRes->d = - ((vPlaneRes->x*v1->x) + (vPlaneRes->y*v1->y) + (vPlaneRes->z*v1->z));
    return vPlaneRes;
}

// vRes = v1 + ((v2 - v1) * alpha)
DVEC4 *LerpDVEC4Res(float alpha, DVEC4 *v1, DVEC4 *v2, DVEC4 *vlerpRes) {
    DVEC4 v2subv1;
    return AddDVEC4Res(v1, MulValDVEC4(SubDVEC4Res(v2, v1, &v2subv1), alpha), vlerpRes);
}

// project Ray ([rpos], [rdir]) vResProj = rdir * t + rpos
DVEC4 *RayProjectDVEC4Res(float t, DVEC4 *rpos, DVEC4 *rdir, DVEC4 *vResProj) {
    DVEC4 vdirMulT;
    MulValDVEC4Res(rdir, t, &vdirMulT);
    return AddDVEC4Res(rpos, &vdirMulT, vResProj);
}

DVEC4 *MulDVEC4(DVEC4 *v1, DVEC4 *v2) {
    v1->x*=v2->x;
    v1->y*=v2->y;
    v1->z*=v2->z;
    //v1->d*=v2->d;
    return v1;
}


DVEC4 *MulDVEC4Res(DVEC4 *v1, DVEC4 *v2, DVEC4 *vresMul) {
    vresMul->x=v1->x*v2->x;
    vresMul->y=v1->y*v2->y;
    vresMul->z=v1->z*v2->z;
    //vresMul->d=v1->d*v2->d;
    return vresMul;
}

DVEC4 *MulValDVEC4(DVEC4 *vec4, float val) {
    vec4->x *= val;
    vec4->y *= val;
    vec4->z *= val;
    //vec4->d *=  val;
    return vec4;
}

DVEC4 *MulValDVEC4Res(DVEC4 *vec4, float val, DVEC4 *vres) {
    vres->x = vec4->x * val;
    vres->y = vec4->y * val;
    vres->z = vec4->z * val;
    //vres->d = vec4->d *  val;
    return vres;
}

void MulDVEC4Array(DVEC4 *vec4array, int count, DVEC4 *vmul) {
    for (int i=0; i < count; i++) {
        vec4array[i].x *= vmul->x;
        vec4array[i].y *= vmul->y;
        vec4array[i].z *= vmul->z;
        //vec4array[i].d *= vmul->d;
    }
}

void MulValDVEC4Array(DVEC4 *vec4array, int count, float val) {
    for (int i=0; i < count; i++) {
        vec4array[i].x *= val;
        vec4array[i].y *= val;
        vec4array[i].z *= val;
        //vec4array[i].d *= val;
    }
}

// ([v1.x + v2.x],  [v1.y + v2.y],  [v1.z + v2.z])
DVEC4 *AddDVEC4(DVEC4 *v1, DVEC4 *v2) {
    v1->x += v2->x;
    v1->y += v2->y;
    v1->z += v2->z;
    return v1;
}

DVEC4 *AddDVEC4Res(DVEC4 *v1, DVEC4 *v2, DVEC4 *vresAdd) {
    vresAdd->x = v1->x + v2->x;
    vresAdd->y = v1->y + v2->y;
    vresAdd->z = v1->z + v2->z;
    return vresAdd;
}

void AddDVEC4Array(DVEC4 *vec4array, int count, DVEC4 *vplus) {
    for (int i=0; i < count; i++) {
        vec4array[i].x += vplus->x;
        vec4array[i].y += vplus->y;
        vec4array[i].z += vplus->z;
        //vec4array[i].d *= vplus->d;
    }
}

// v1 = v1 - v2
// v1([v1.x - v2.x],  [v1.y - v2.y],  [v1.z - v2.z])
DVEC4 *SubDVEC4(DVEC4 *v1, DVEC4 *v2) {
    v1->x -= v2->x;
    v1->y -= v2->y;
    v1->z -= v2->z;
    //v1->d -= v2->d;
    return v1;
}

// vresSub = v1 - v2
DVEC4 *SubDVEC4Res(DVEC4 *v1, DVEC4 *v2, DVEC4 *vresSub) {
    vresSub->x = v1->x - v2->x;
    vresSub->y = v1->y - v2->y;
    vresSub->z = v1->z - v2->z;
    //vresSub->d = v1->d - v2->d;
    return vresSub;
}

// Conversion / Clip ================

void DVEC4Array2DVec4i(DVEC4i *vec4iArrayDst, DVEC4 *vec4ArraySrc, int count) {
    for (int i=0; i < count; i++) {
        vec4iArrayDst[i].x = (int)vec4ArraySrc[i].x;
        vec4iArrayDst[i].y = (int)vec4ArraySrc[i].y;
        vec4iArrayDst[i].z = (int)vec4ArraySrc[i].z;
        //vec4iArrayDst[i].d = (int)vec4ArraySrc[i].d;
    }
}

void DVEC4iArray2DVec4(DVEC4 *vec4ArrayDst, DVEC4i *vec4iArraySrc, int count) {
    for (int i=0; i < count; i++) {
        vec4ArrayDst[i].x = (float)vec4iArraySrc[i].x;
        vec4ArrayDst[i].y = (float)vec4iArraySrc[i].y;
        vec4ArrayDst[i].z = (float)vec4iArraySrc[i].z;
        //vec4ArrayDst[i].d = (float)vec4iArraySrc[i].d;
    }
}

void ClipDVEC4Array(DVEC4 *vec4Array, int count, DVEC4 *vec4_min, DVEC4 *vec4_max) {
    for (int i=0; i < count; i++) {
        if (vec4Array[i].x < vec4_min->x) vec4Array[i].x = vec4_min->x;
        if (vec4Array[i].y < vec4_min->y) vec4Array[i].y = vec4_min->y;
        if (vec4Array[i].z < vec4_min->z) vec4Array[i].z = vec4_min->z;

        if (vec4Array[i].x > vec4_max->x) vec4Array[i].x = vec4_max->x;
        if (vec4Array[i].y > vec4_max->y) vec4Array[i].y = vec4_max->y;
        if (vec4Array[i].z > vec4_max->z) vec4Array[i].z = vec4_max->z;
    }
}

void CopyDVEC4(void *vec4ArrayDst, void *vec4ArraySrc, int count) {
    memcpy(vec4ArrayDst, vec4ArraySrc, sizeof(DVEC4)*count);
}

void StoreDVEC4(void *vec4ArrayDst, void *vec4ElemSrc, int count) {
    for (int i=0; i < count; i++) {
        memcpy(&((DVEC4*)vec4ArrayDst)[i], vec4ElemSrc, sizeof(DVEC4));
    }
}

//; search/AABBox/Filtering/intersection
//
void FetchDAAMinBBoxDVEC4Array(DVEC4 *vec4Array, int count, DAAMinBBox *aaMinBboxRes) {
    DVEC4 vec4_min, vec4_max;
    if (count > 0) {
        vec4_min = vec4Array[0];
        vec4_max = vec4Array[0];
    }
    if (count > 1) {
        for (int i=1; i < count; i++) {
            if (vec4Array[i].x < vec4_min.x) vec4_min.x = vec4Array[i].x ;
            if (vec4Array[i].y < vec4_min.y) vec4_min.y = vec4Array[i].y ;
            if (vec4Array[i].z < vec4_min.z) vec4_min.z = vec4Array[i].z ;

            if (vec4Array[i].x > vec4_max.x) vec4_max.x = vec4Array[i].x ;
            if (vec4Array[i].y > vec4_max.y) vec4_max.y = vec4Array[i].y ;
            if (vec4Array[i].z > vec4_max.z) vec4_max.z = vec4Array[i].z ;
        }
    }
    aaMinBboxRes->min = vec4_min;
    aaMinBboxRes->max = vec4_max;
}

//
void FetchDAABBoxDVEC4Array(DVEC4 *vec4Array, int count, DAABBox *aaBboxRes) {
    DVEC4 vec4_min, vec4_max;
    if (count > 0) {
        vec4_min = vec4Array[0];
        vec4_max = vec4Array[0];
    }
    if (count > 1) {
        for (int i=1; i < count; i++) {
            if (vec4Array[i].x < vec4_min.x) vec4_min.x = vec4Array[i].x ;
            if (vec4Array[i].y < vec4_min.y) vec4_min.y = vec4Array[i].y ;
            if (vec4Array[i].z < vec4_min.z) vec4_min.z = vec4Array[i].z ;

            if (vec4Array[i].x > vec4_max.x) vec4_max.x = vec4Array[i].x ;
            if (vec4Array[i].y > vec4_max.y) vec4_max.y = vec4Array[i].y ;
            if (vec4Array[i].z > vec4_max.z) vec4_max.z = vec4Array[i].z ;
        }
    }
    aaBboxRes->v[0] = vec4_min;                                                                     //  minX, minY, minZ
    aaBboxRes->v[1].x = vec4_max.x; aaBboxRes->v[1].y = vec4_min.y; aaBboxRes->v[1].z = vec4_min.z; //  maxX, minY, minZ
    aaBboxRes->v[2].x = vec4_max.x; aaBboxRes->v[2].y = vec4_max.y; aaBboxRes->v[2].z = vec4_min.z; //  maxX, maxY, minZ
    aaBboxRes->v[3].x = vec4_min.x; aaBboxRes->v[3].y = vec4_max.y; aaBboxRes->v[3].z = vec4_min.z; //  minX, maxY, minZ
    aaBboxRes->v[4].x = vec4_min.x; aaBboxRes->v[4].y = vec4_min.y; aaBboxRes->v[4].z = vec4_max.z; //  minX, minY, maxZ
    aaBboxRes->v[5].x = vec4_max.x; aaBboxRes->v[5].y = vec4_min.y; aaBboxRes->v[5].z = vec4_max.z; //  maxX, minY, maxZ
    aaBboxRes->v[6] = vec4_max;                                                                     //  maxX, maxY, maxZ
    aaBboxRes->v[7].x = vec4_min.x; aaBboxRes->v[7].y = vec4_max.y; aaBboxRes->v[7].z = vec4_max.z; //  minX, maxY, maxZ
}

//
//
// culling / collision / comparison
//
//; compare equality of x,y,z and d
bool EqualDVEC4(DVEC4 *v1, DVEC4 *v2) {
    return ((fabsf((v1->x - v2->x)) <= THETA_SMALL) && (fabsf((v1->y - v2->y)) <= THETA_SMALL) && (fabsf((v1->z - v2->z)) <= THETA_SMALL));
}

// return true if vec4Pos is inside aaMinBbox boundaries
bool DVEC4InAAMinBBox(DVEC4 *vec4Pos, DAAMinBBox *aaMinBbox) {
    if (vec4Pos->x < aaMinBbox->min.x || vec4Pos->x > aaMinBbox->max.x ||
        vec4Pos->y < aaMinBbox->min.y || vec4Pos->y > aaMinBbox->max.y ||
        vec4Pos->z < aaMinBbox->min.z || vec4Pos->z > aaMinBbox->max.z)
        return false;
    return true;
}

// return count of vec4 insdie aaMinBbox
int DVEC4ArrayIdxCountInAAMinBBox(DVEC4 *vec4Array, int *idxsVec4, int countIdxs, DAAMinBBox *aaMinBbox) {
    int countIn = 0;
    for (int i = 0; i < countIdxs; i++) {
        if (DVEC4InAAMinBBox(&vec4Array[idxsVec4[i]], aaMinBbox))
            countIn++;
    }
    return countIn;
}

// return count of vec4 insdie aaMinBbox, fill InMap with 0/out 1/in
int DVEC4ArrayIdxCountInMapAAMinBBox(DVEC4 *vec4Array, int *idxsVec4, int countIdxs, DAAMinBBox *aaMinBbox, char *InMap) {
    int countIn = 0;
    for (int i = 0; i < countIdxs; i++) {
        if (DVEC4InAAMinBBox(&vec4Array[idxsVec4[i]], aaMinBbox)) {
            countIn ++;
            InMap[i] = 1;
        } else {
            InMap[i] = 0;
        }
    }
    return countIn;
}

// return true vec4Pos is inside aaMinBBox
// Mask: 0x1111 => (d,z,y,x) ex: 0x110 => test in for (z,y)
bool DVEC4MaskInAAMinBBox(DVEC4 *vec4Pos, DAAMinBBox *aaMinBbox, int Mask) {
    if ((Mask & 0x1000) > 0) { // d
        if (vec4Pos->d > aaMinBbox->max.d || vec4Pos->d < aaMinBbox->min.d)
            return false;
    }
    if ((Mask & 0x0100) > 0) { // z
        if (vec4Pos->z > aaMinBbox->max.z || vec4Pos->z < aaMinBbox->min.z)
            return false;
    }
    if ((Mask & 0x0010) > 0) { // y
        if (vec4Pos->y > aaMinBbox->max.y || vec4Pos->y < aaMinBbox->min.y)
            return false;
    }
    if ((Mask & 0x0001) > 0) { // x
        if (vec4Pos->x > aaMinBbox->max.x || vec4Pos->x < aaMinBbox->min.x)
            return false;
    }
    return true;
}

//
// return min of the x, y, z, d components merged in res DVEC4
void DVEC4MinRes(DVEC4 *v1, DVEC4 *v2, DVEC4 *vec4_minRes) {
    vec4_minRes->x = (v1->x <= v2->x) ? v1->x : v2->x;
    vec4_minRes->y = (v1->y <= v2->y) ? v1->y : v2->y;
    vec4_minRes->z = (v1->z <= v2->z) ? v1->z : v2->z;
    vec4_minRes->d = (v1->d <= v2->d) ? v1->d : v2->d;
}


// return max of the x, y, z, d components merged in res DVEC4
void DVEC4MaxRes(DVEC4 *v1, DVEC4 *v2, DVEC4 *vec4_maxRes) {
    vec4_maxRes->x = (v1->x >= v2->x) ? v1->x : v2->x;
    vec4_maxRes->y = (v1->y >= v2->y) ? v1->y : v2->y;
    vec4_maxRes->z = (v1->z >= v2->z) ? v1->z : v2->z;
    vec4_maxRes->d = (v1->d >= v2->d) ? v1->d : v2->d;
}

// return min value of x, y, z components in minXYZRes
void DVEC4MinXYZ(DVEC4 *v, float *minXYZRes) {
    *minXYZRes = (v->x <= v->y) ? v->x : v->y;
    if (v->z < *minXYZRes)
        *minXYZRes = v->z;
}

// return max value of x, y, z components in maxXYZRes
void DVEC4MaxXYZ(DVEC4 *v, float *maxXYZRes) {
    *maxXYZRes = (v->x >= v->y) ? v->x : v->y;
    if (v->z > *maxXYZRes)
        *maxXYZRes = v->z;
}


// DMatrix4 ===================

// multiply DVEC4 array by mat4x4 and store result on the same array
void DMatrix4MulDVEC4Array(DMatrix4 *mat4x4, DVEC4 *vec4Array, int count) {
    DVEC4 inVec4;
    for (int i=0; i<count; i++) {
        inVec4 = vec4Array[i];
        vec4Array[i].x = inVec4.x * mat4x4->rows[0].x + inVec4.y * mat4x4->rows[1].x + inVec4.z * mat4x4->rows[2].x + mat4x4->rows[3].x;
        vec4Array[i].y = inVec4.x * mat4x4->rows[0].y + inVec4.y * mat4x4->rows[1].y + inVec4.z * mat4x4->rows[2].y + mat4x4->rows[3].y;
        vec4Array[i].z = inVec4.x * mat4x4->rows[0].z + inVec4.y * mat4x4->rows[1].z + inVec4.z * mat4x4->rows[2].z + mat4x4->rows[3].z;
    }
}

// multiply DVEC4 array by mat4x4 and store result on the result DVEC4 array
void DMatrix4MulDVEC4ArrayRes(DMatrix4 *mat4x4, DVEC4 *vec4ArraySrc, int count, DVEC4 *vec4ArrayDst) {
    for (int i=0; i<count; i++) {
        vec4ArrayDst[i].x = vec4ArraySrc[i].x * mat4x4->rows[0].x + vec4ArraySrc[i].y * mat4x4->rows[1].x + vec4ArraySrc[i].z * mat4x4->rows[2].x + mat4x4->rows[3].x;
        vec4ArrayDst[i].y = vec4ArraySrc[i].x * mat4x4->rows[0].y + vec4ArraySrc[i].y * mat4x4->rows[1].y + vec4ArraySrc[i].z * mat4x4->rows[2].y + mat4x4->rows[3].y;
        vec4ArrayDst[i].z = vec4ArraySrc[i].x * mat4x4->rows[0].z + vec4ArraySrc[i].y * mat4x4->rows[1].z + vec4ArraySrc[i].z * mat4x4->rows[2].z + mat4x4->rows[3].z;
    }
}

// multiply DVEC4 array by mat4x4, divide result x,y,z by max (1.0, result Z) and store result on the same array
void DMatrix4MulDVEC4ArrayPersp(DMatrix4 *mat4x4, DVEC4 *vec4Array, int count) {
     DVEC4 inVec4;
     float invDivZ = 0.0f;
     for (int i=0; i<count; i++) {
        inVec4 = vec4Array[i];
        vec4Array[i].z = inVec4.x * mat4x4->rows[0].z + inVec4.y * mat4x4->rows[1].z + inVec4.z * mat4x4->rows[2].z + mat4x4->rows[3].z;
        if (vec4Array[i].z > 1.0f) {
            invDivZ = 1.0f/vec4Array[i].z;
            vec4Array[i].z *= invDivZ;
            vec4Array[i].x = (inVec4.x * mat4x4->rows[0].x + inVec4.y * mat4x4->rows[1].x + inVec4.z * mat4x4->rows[2].x + mat4x4->rows[3].x) * invDivZ;
            vec4Array[i].y = (inVec4.x * mat4x4->rows[0].y + inVec4.y * mat4x4->rows[1].y + inVec4.z * mat4x4->rows[2].y + mat4x4->rows[3].y) * invDivZ;
        }
     }
}

// multiply DVEC4 array by mat4x4, divide result x,y,z by max (1.0, result Z) and store result on destination array
 void DMatrix4MulDVEC4ArrayPerspRes(DMatrix4 *mat4x4, DVEC4 *vec4ArraySrc, int count, DVEC4 *vec4ArrayDst) {
     float invDivZ = 0.0f;
     for (int i=0; i<count; i++) {
        vec4ArrayDst[i].z = vec4ArraySrc[i].x * mat4x4->rows[0].z + vec4ArraySrc[i].y * mat4x4->rows[1].z + vec4ArraySrc[i].z * mat4x4->rows[2].z + mat4x4->rows[3].z;
        if (vec4ArrayDst[i].z > 1.0f) {
            invDivZ = (1.0f/vec4ArrayDst[i].z);
            vec4ArrayDst[i].z *= invDivZ;
            vec4ArrayDst[i].x = (vec4ArraySrc[i].x * mat4x4->rows[0].x + vec4ArraySrc[i].y * mat4x4->rows[1].x + vec4ArraySrc[i].z * mat4x4->rows[2].x + mat4x4->rows[3].x) * invDivZ;
            vec4ArrayDst[i].y = (vec4ArraySrc[i].x * mat4x4->rows[0].y + vec4ArraySrc[i].y * mat4x4->rows[1].y + vec4ArraySrc[i].z * mat4x4->rows[2].y + mat4x4->rows[3].y) * invDivZ;
        }
     }
 }


// multiply DVEC4 array by mat4x4, and store result on destination integer DVEC4i array
void DMatrix4MulDVEC4ArrayResDVec4i(DMatrix4 *mat4x4, DVEC4 *vec4ArraySrc, int count, DVEC4i *vec4ArrayDst) {
     for (int i=0; i<count; i++) {
        vec4ArrayDst[i].x = (int)(vec4ArraySrc[i].x * mat4x4->rows[0].x + vec4ArraySrc[i].y * mat4x4->rows[1].x + vec4ArraySrc[i].z * mat4x4->rows[2].x + mat4x4->rows[3].x);
        vec4ArrayDst[i].y = (int)(vec4ArraySrc[i].x * mat4x4->rows[0].y + vec4ArraySrc[i].y * mat4x4->rows[1].y + vec4ArraySrc[i].z * mat4x4->rows[2].y + mat4x4->rows[3].y);
        vec4ArrayDst[i].z = (int)(vec4ArraySrc[i].x * mat4x4->rows[0].z + vec4ArraySrc[i].y * mat4x4->rows[1].z + vec4ArraySrc[i].z * mat4x4->rows[2].z + mat4x4->rows[3].z);
     }
}

// multiply DVEC4 array by subset 3x2 of mat4x4, then add by last mat4x4 row (x,y), and store result on destination integer DVEC2i array (only x, y)
 void DMatrix4MulDVEC4ArrayResDVec2i(DMatrix4 *mat4x4, DVEC4 *vec4ArraySrc, int count, DVEC2i *vec2iArrayDst) {
     for (int i=0; i<count; i++) {
        vec2iArrayDst[i].x = (int)((vec4ArraySrc[i].x * mat4x4->rows[0].x + vec4ArraySrc[i].y * mat4x4->rows[1].x + vec4ArraySrc[i].z * mat4x4->rows[2].x) + mat4x4->rows[3].x);
        vec2iArrayDst[i].y = (int)((vec4ArraySrc[i].x * mat4x4->rows[0].y + vec4ArraySrc[i].y * mat4x4->rows[1].y + vec4ArraySrc[i].z * mat4x4->rows[2].y) + mat4x4->rows[3].y);
     }
}

DMatrix4 *DMatrix4MulDMatrix4(DMatrix4 *mat4x4_left, DMatrix4 *mat4x4_right) {
    DMatrix4 saveLeft = *mat4x4_left;
    // Row 0
    mat4x4_left->raw[0]  = saveLeft.raw[0]*mat4x4_right->raw[0] + saveLeft.raw[1]*mat4x4_right->raw[4] + saveLeft.raw[2]*mat4x4_right->raw[8] + saveLeft.raw[3]*mat4x4_right->raw[12];
    mat4x4_left->raw[1]  = saveLeft.raw[0]*mat4x4_right->raw[1] + saveLeft.raw[1]*mat4x4_right->raw[5] + saveLeft.raw[2]*mat4x4_right->raw[9] + saveLeft.raw[3]*mat4x4_right->raw[13];
    mat4x4_left->raw[2]  = saveLeft.raw[0]*mat4x4_right->raw[2] + saveLeft.raw[1]*mat4x4_right->raw[6] + saveLeft.raw[2]*mat4x4_right->raw[10] + saveLeft.raw[3]*mat4x4_right->raw[14];
    mat4x4_left->raw[3]  = saveLeft.raw[0]*mat4x4_right->raw[3] + saveLeft.raw[1]*mat4x4_right->raw[7] + saveLeft.raw[2]*mat4x4_right->raw[11] + saveLeft.raw[3]*mat4x4_right->raw[15];
    // Row 1
    mat4x4_left->raw[4]  = saveLeft.raw[4]*mat4x4_right->raw[0] + saveLeft.raw[5]*mat4x4_right->raw[4] + saveLeft.raw[6]*mat4x4_right->raw[8] + saveLeft.raw[7]*mat4x4_right->raw[12];
    mat4x4_left->raw[5]  = saveLeft.raw[4]*mat4x4_right->raw[1] + saveLeft.raw[5]*mat4x4_right->raw[5] + saveLeft.raw[6]*mat4x4_right->raw[9] + saveLeft.raw[7]*mat4x4_right->raw[13];
    mat4x4_left->raw[6]  = saveLeft.raw[4]*mat4x4_right->raw[2] + saveLeft.raw[5]*mat4x4_right->raw[6] + saveLeft.raw[6]*mat4x4_right->raw[10] + saveLeft.raw[7]*mat4x4_right->raw[14];
    mat4x4_left->raw[7]  = saveLeft.raw[4]*mat4x4_right->raw[3] + saveLeft.raw[5]*mat4x4_right->raw[7] + saveLeft.raw[6]*mat4x4_right->raw[11] + saveLeft.raw[7]*mat4x4_right->raw[15];
    // Row 2
    mat4x4_left->raw[8]  = saveLeft.raw[8]*mat4x4_right->raw[0] + saveLeft.raw[9]*mat4x4_right->raw[4] + saveLeft.raw[10]*mat4x4_right->raw[8] + saveLeft.raw[11]*mat4x4_right->raw[12];
    mat4x4_left->raw[9]  = saveLeft.raw[8]*mat4x4_right->raw[1] + saveLeft.raw[9]*mat4x4_right->raw[5] + saveLeft.raw[10]*mat4x4_right->raw[9] + saveLeft.raw[11]*mat4x4_right->raw[13];
    mat4x4_left->raw[10] = saveLeft.raw[8]*mat4x4_right->raw[2] + saveLeft.raw[9]*mat4x4_right->raw[6] + saveLeft.raw[10]*mat4x4_right->raw[10] + saveLeft.raw[11]*mat4x4_right->raw[14];
    mat4x4_left->raw[11] = saveLeft.raw[8]*mat4x4_right->raw[3] + saveLeft.raw[9]*mat4x4_right->raw[7] + saveLeft.raw[10]*mat4x4_right->raw[11] + saveLeft.raw[11]*mat4x4_right->raw[15];
    // Row 3
    mat4x4_left->raw[12] = saveLeft.raw[12]*mat4x4_right->raw[0] + saveLeft.raw[13]*mat4x4_right->raw[4] + saveLeft.raw[14]*mat4x4_right->raw[8] + saveLeft.raw[15]*mat4x4_right->raw[12];
    mat4x4_left->raw[13] = saveLeft.raw[12]*mat4x4_right->raw[1] + saveLeft.raw[13]*mat4x4_right->raw[5] + saveLeft.raw[14]*mat4x4_right->raw[9] + saveLeft.raw[15]*mat4x4_right->raw[13];
    mat4x4_left->raw[14] = saveLeft.raw[12]*mat4x4_right->raw[2] + saveLeft.raw[13]*mat4x4_right->raw[6] + saveLeft.raw[14]*mat4x4_right->raw[10] + saveLeft.raw[15]*mat4x4_right->raw[14];
    mat4x4_left->raw[15] = saveLeft.raw[12]*mat4x4_right->raw[3] + saveLeft.raw[13]*mat4x4_right->raw[7] + saveLeft.raw[14]*mat4x4_right->raw[11] + saveLeft.raw[15]*mat4x4_right->raw[15];

    return mat4x4_left;
}


DMatrix4 *DMatrix4MulDMatrix4Res(DMatrix4 *mat4x4_left, DMatrix4 *mat4x4_right, DMatrix4 *mat4x4_res) {
    // row 0
    mat4x4_res->raw[0]  = mat4x4_left->raw[0]*mat4x4_right->raw[0] + mat4x4_left->raw[1]*mat4x4_right->raw[4] + mat4x4_left->raw[2]*mat4x4_right->raw[8] + mat4x4_left->raw[3]*mat4x4_right->raw[12];
    mat4x4_res->raw[1]  = mat4x4_left->raw[0]*mat4x4_right->raw[1] + mat4x4_left->raw[1]*mat4x4_right->raw[5] + mat4x4_left->raw[2]*mat4x4_right->raw[9] + mat4x4_left->raw[3]*mat4x4_right->raw[13];
    mat4x4_res->raw[2]  = mat4x4_left->raw[0]*mat4x4_right->raw[2] + mat4x4_left->raw[1]*mat4x4_right->raw[6] + mat4x4_left->raw[2]*mat4x4_right->raw[10] + mat4x4_left->raw[3]*mat4x4_right->raw[14];
    mat4x4_res->raw[3]  = mat4x4_left->raw[0]*mat4x4_right->raw[3] + mat4x4_left->raw[1]*mat4x4_right->raw[7] + mat4x4_left->raw[2]*mat4x4_right->raw[11] + mat4x4_left->raw[3]*mat4x4_right->raw[15];
    // Row 1
    mat4x4_res->raw[4]  = mat4x4_left->raw[4]*mat4x4_right->raw[0] + mat4x4_left->raw[5]*mat4x4_right->raw[4] + mat4x4_left->raw[6]*mat4x4_right->raw[8] + mat4x4_left->raw[7]*mat4x4_right->raw[12];
    mat4x4_res->raw[5]  = mat4x4_left->raw[4]*mat4x4_right->raw[1] + mat4x4_left->raw[5]*mat4x4_right->raw[5] + mat4x4_left->raw[6]*mat4x4_right->raw[9] + mat4x4_left->raw[7]*mat4x4_right->raw[13];
    mat4x4_res->raw[6]  = mat4x4_left->raw[4]*mat4x4_right->raw[2] + mat4x4_left->raw[5]*mat4x4_right->raw[6] + mat4x4_left->raw[6]*mat4x4_right->raw[10] + mat4x4_left->raw[7]*mat4x4_right->raw[14];
    mat4x4_res->raw[7]  = mat4x4_left->raw[4]*mat4x4_right->raw[3] + mat4x4_left->raw[5]*mat4x4_right->raw[7] + mat4x4_left->raw[6]*mat4x4_right->raw[11] + mat4x4_left->raw[7]*mat4x4_right->raw[15];
    // Row 2
    mat4x4_res->raw[8]  = mat4x4_left->raw[8]*mat4x4_right->raw[0] + mat4x4_left->raw[9]*mat4x4_right->raw[4] + mat4x4_left->raw[10]*mat4x4_right->raw[8] + mat4x4_left->raw[11]*mat4x4_right->raw[12];
    mat4x4_res->raw[9]  = mat4x4_left->raw[8]*mat4x4_right->raw[1] + mat4x4_left->raw[9]*mat4x4_right->raw[5] + mat4x4_left->raw[10]*mat4x4_right->raw[9] + mat4x4_left->raw[11]*mat4x4_right->raw[13];
    mat4x4_res->raw[10] = mat4x4_left->raw[8]*mat4x4_right->raw[2] + mat4x4_left->raw[9]*mat4x4_right->raw[6] + mat4x4_left->raw[10]*mat4x4_right->raw[10] + mat4x4_left->raw[11]*mat4x4_right->raw[14];
    mat4x4_res->raw[11] = mat4x4_left->raw[8]*mat4x4_right->raw[3] + mat4x4_left->raw[9]*mat4x4_right->raw[7] + mat4x4_left->raw[10]*mat4x4_right->raw[11] + mat4x4_left->raw[11]*mat4x4_right->raw[15];
    // Row 3
    mat4x4_res->raw[12] = mat4x4_left->raw[12]*mat4x4_right->raw[0] + mat4x4_left->raw[13]*mat4x4_right->raw[4] + mat4x4_left->raw[14]*mat4x4_right->raw[8] + mat4x4_left->raw[15]*mat4x4_right->raw[12];
    mat4x4_res->raw[13] = mat4x4_left->raw[12]*mat4x4_right->raw[1] + mat4x4_left->raw[13]*mat4x4_right->raw[5] + mat4x4_left->raw[14]*mat4x4_right->raw[9] + mat4x4_left->raw[15]*mat4x4_right->raw[13];
    mat4x4_res->raw[14] = mat4x4_left->raw[12]*mat4x4_right->raw[2] + mat4x4_left->raw[13]*mat4x4_right->raw[6] + mat4x4_left->raw[14]*mat4x4_right->raw[10] + mat4x4_left->raw[15]*mat4x4_right->raw[14];
    mat4x4_res->raw[15] = mat4x4_left->raw[12]*mat4x4_right->raw[3] + mat4x4_left->raw[13]*mat4x4_right->raw[7] + mat4x4_left->raw[14]*mat4x4_right->raw[11] + mat4x4_left->raw[15]*mat4x4_right->raw[15];

    return mat4x4_res;
}
