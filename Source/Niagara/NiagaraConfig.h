#pragma once

// Niagara meshlet and rendering configuration (1:1 from original config.h).

// Workgroup size for task shader; each task shader thread produces up to one meshlet
#define TASK_WGSIZE 64

// Workgroup size for mesh shader; mesh shader workgroup processes the entire meshlet in parallel
#define MESH_WGSIZE 64

// Maximum number of vertices and triangles in a meshlet
#define MESH_MAXVTX 64
#define MESH_MAXTRI 96

// Meshlet build configuration for raster/RT
#define MESHLET_CONE_WEIGHT 0.25f
#define MESHLET_FILL_WEIGHT 0.5f

// Number of clusters along X dimension in a 3D tiled dispatch (must be a divisor of 256)
#define CLUSTER_TILE 16

// Maximum number of total task shader workgroups; 4M workgroups ~= 256M meshlets
#define TASK_WGLIMIT (1 << 22)

// Maximum number of total visible clusters; 16M meshlets ~= 64MB buffer with cluster indices
#define CLUSTER_LIMIT (1 << 24)
