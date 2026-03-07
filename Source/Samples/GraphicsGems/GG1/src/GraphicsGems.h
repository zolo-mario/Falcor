/*
 * GraphicsGems.h
 * Version 1.0 - Andrew Glassner
 * from "Graphics Gems", Academic Press, 1990
 *
 * The authors and the publisher hold no copyright restrictions on this file;
 * this source code is public domain, and is freely available to the entire
 * computer graphics community for study, use, and modification.
 */

#ifndef GG_H
#define GG_H 1

/*********************/
/* 2d geometry types */
/*********************/

typedef struct Point2Struct { /* 2d point */
    double x, y;
} Point2;
typedef Point2 Vector2;

typedef struct IntPoint2Struct { /* 2d integer point */
    int x, y;
} IntPoint2;

typedef struct Matrix3Struct { /* 3-by-3 matrix */
    double element[3][3];
} Matrix3;

typedef struct Box2dStruct { /* 2d box */
    Point2 min, max;
} Box2;

/*********************/
/* 3d geometry types */
/*********************/

typedef struct Point3Struct { /* 3d point */
    double x, y, z;
} Point3;
typedef Point3 Vector3;

typedef struct IntPoint3Struct { /* 3d integer point */
    int x, y, z;
} IntPoint3;

typedef struct Matrix4Struct { /* 4-by-4 matrix */
    double element[4][4];
} Matrix4;

typedef struct Box3dStruct { /* 3d box */
    Point3 min, max;
} Box3;

/***********************/
/* one-argument macros */
/***********************/

#define ABS(a) (((a) < 0) ? -(a) : (a))
#define ROUND(a) ((a) > 0 ? (int)((a) + 0.5) : -(int)(0.5 - (a)))
#define ZSGN(a) (((a) < 0) ? -1 : (a) > 0 ? 1 : 0)
#define SGN(a) (((a) < 0) ? -1 : 1)
#define SQR(a) ((a) * (a))

/***********************/
/* two-argument macros */
/***********************/

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define LERP(a, l, h) ((l) + (((h) - (l)) * (a)))
#define CLAMP(v, l, h) ((v) < (l) ? (l) : (v) > (h) ? (h) : (v))

/****************************/
/* memory allocation macros */
/****************************/

#include <stdlib.h>
#define NEWSTRUCT(x) (struct x*)(malloc((unsigned)sizeof(struct x)))
#define NEWTYPE(x) (x*)(malloc((unsigned)sizeof(x)))

/********************/
/* useful constants */
/********************/

#define GG_PI       3.141592
#define PITIMES2    6.283185
#define PIOVER2     1.570796
#define GG_E        2.718282
#define SQRT2       1.414214
#define SQRT3       1.732051
#define GOLDEN      1.618034
#define DTOR        0.017453
#define RTOD        57.29578

#endif /* GG_H */
