# Falcor Scene → Niagara Scene 映射表

适配器 `convertFalcorSceneToNiagaraScene` 将 Falcor `Scene` 转换为 `NiagaraFormat::NiagaraSceneFormat`。下表描述各概念与字段的对应关系。

## 顶层

| Falcor | Niagara |
|--------|---------|
| `Scene` | `NiagaraSceneFormat` |
| 几何 + 材质 + 实例 + 相机 + 灯光 | `geometry` + `materials` + `draws` + `texturePaths` + `camera` + `sunDirection` |

---

## Mesh / Geometry

| Falcor | Niagara | 说明 |
|--------|---------|------|
| `getMeshCount()` / `getMesh(MeshID)` | `Geometry::meshes` | 每个 Falcor mesh → 一个 `Mesh` |
| `getMeshVerticesAndIndices(meshID, buffers)` | `Geometry::vertices`, `Geometry::indices` | 顶点经去重、量化；三角形索引展平为 uint32 列表 |
| — | `Geometry::meshlets`, `meshletdata`, `meshletvtx0` | 由适配器内用 meshoptimizer 从顶点/索引**重新生成**，不读 Falcor meshlet |
| `MeshDesc` (vbOffset, vertexCount, indexCount, …) | `Mesh` (vertexOffset, vertexCount, lods[].indexOffset/indexCount, center, radius) | 仅用 vertexCount 与三角形数；顶点/索引来自 getMeshVerticesAndIndices |
| — | `MeshLod` | 单 LOD：indexOffset, indexCount, meshletOffset, meshletCount, error=0 |
| 顶点 position (float3) | `Vertex::vx, vy, vz` (half) | meshopt_quantizeHalf |
| 顶点 normal/tangent/uv | `Vertex::tp, np, tu, tv` | 当前适配器未从 Falcor 读取时填默认 (np 默认法线等) |

---

## Material

| Falcor | Niagara | 说明 |
|--------|---------|------|
| 无 (dummy) | `materials[0]` | 占位材质，纹理索引 0，因子默认 |
| `getMaterialCount()` / `getMaterial(MaterialID)` | `materials[1..N]` | 仅支持 `toBasicMaterial()` |
| `BasicMaterial::getBaseColor()` | `Material::diffuseFactor` (float4) | |
| `BasicMaterial::getSpecularParams()` | `Material::specularFactor` (float4) | |
| `BasicMaterial::getData().emissive * emissiveFactor` | `Material::emissiveFactor` (float3) | |
| `getBaseColorTexture()->getSourcePath()` | `Material::albedoTexture` (int) | 索引为 texturePaths 中的 1-based 下标 |
| `getNormalMap()->getSourcePath()` | `Material::normalTexture` | 同上 |
| `getSpecularTexture()->getSourcePath()` | `Material::specularTexture` | 同上 |
| `getEmissiveTexture()->getSourcePath()` | `Material::emissiveTexture` | 同上 |
| 纹理路径去重列表 | `NiagaraSceneFormat::texturePaths` | 由各材质纹理路径汇总，addTexturePath 去重 |

---

## Draw / Instance

| Falcor | Niagara | 说明 |
|--------|---------|------|
| `getGeometryInstanceCount()` / `getGeometryInstance(instanceID)` | `NiagaraSceneFormat::draws` | 仅 `TriangleMesh` / `DisplacedTriangleMesh` |
| `getAnimationController()->getGlobalMatrices()[instance.globalMatrixID]` | `MeshDraw::position`, `scale`, `orientation` | math::decompose 得到 translation、uniform scale、quat |
| `instance.geometryID` | `MeshDraw::meshIndex` | 对应 `Geometry::meshes` 下标 |
| `instance.materialID + 1` | `MeshDraw::materialIndex` | +1 因 materials[0] 为 dummy |
| — | `MeshDraw::meshletVisibilityOffset` | 填 0 |
| — | `MeshDraw::postPass` | 填 0 |

---

## Camera

| Falcor | Niagara | 说明 |
|--------|---------|------|
| `getCameras().empty()` 为 false 时 `getCamera()` | `NiagaraSceneFormat::camera` | 取当前选中相机 |
| `Camera::getPosition()` | `Camera::position` (float3) | |
| `math::inverse(getViewMatrix())` 分解 | `Camera::orientation` (quatf) | 从逆 view 矩阵分解旋转 |
| `focalLengthToFovY(getFocalLength(), getFrameHeight())` | `Camera::fovY` | |
| `getNearPlane()` | `Camera::znear` | |
| `getViewMatrix()` | `Camera::viewMatrix` (float4x4) | 缓存 |
| 无相机 | 默认 camera | position=0, orientation=identity, fovY≈70°, znear=0.1, viewMatrix=identity |

---

## Light

| Falcor | Niagara | 说明 |
|--------|---------|------|
| `getLights()` 中 `LightType::Distant` | `NiagaraSceneFormat::sunDirection` (float3) | 取首个 DistantLight，`DistantLight::getWorldDirection()` 后 normalize |
| 无方向光 | `sunDirection = normalize(float3(1,1,1))` | 默认 |

---

## 类型速查 (NiagaraFormat)

| 类型 | 用途 |
|------|------|
| `Vertex` | 顶点：vx,vy,vz (half), tp (tangent packed), np (normal packed), tu,tv (uv) |
| `Meshlet` | Meshlet 元数据：center, radius, cone_axis/cutoff, dataOffset, baseVertex, vertexCount, triangleCount, shortRefs |
| `Material` | 材质：albedo/normal/specular/emissive 纹理索引 (int)，diffuse/specular/emissive 因子 |
| `MeshDraw` | 实例：position, scale, orientation, meshIndex, materialIndex, meshletVisibilityOffset, postPass |
| `MeshLod` | LOD：indexOffset, indexCount, meshletOffset, meshletCount, error |
| `Mesh` | 网格：center, radius, vertexOffset, vertexCount, lodCount, lods[8] |
| `Geometry` | 几何集合：vertices, indices, meshlets, meshletdata, meshletvtx0, meshes |
| `Camera` | 相机：position, orientation, fovY, znear, viewMatrix |
| `NiagaraSceneFormat` | 场景根：geometry, materials, draws, texturePaths, camera, sunDirection |
