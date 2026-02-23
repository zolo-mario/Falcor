# Niagara AS 剔除实现说明

本文档记录 Amplification Shader (AS) 中 frustum + cone 剔除的实现细节与常见问题。

---

## View Space 约定 (Falcor)

- **右手系**：相机看向 **-Z**
- **可见深度范围**：`z ∈ [-zfar, -znear]`
  - 近平面：`z = -znear`（距相机 znear）
  - 远平面：`z = -zfar`（距相机 zfar）
- 球体 center 在 view space 中：`center = mul(view, float4(worldPos, 1)).xyz`

---

## Near/Far 平面测试

**问题现象**：早期实现使用错误符号导致黑屏。

错误逻辑（会剔除所有几何体）：
```
// 错误
visible = visible && (center.z + radius > -znear);   // 近平面
visible = visible && (center.z - radius < -zfar);    // 远平面
```

正确逻辑：
```
// 正确
visible = visible && (center.z + radius < -znear);  // 球体后部在近平面之后
visible = visible && (center.z - radius > -zfar);  // 球体前部在远平面之前
```

**解释**：
- `center.z + radius`：球体后部（最靠近相机的 z）
- `center.z - radius`：球体前部（最远离相机的 z）
- 可见要求：后部必须在近平面之后（`< -znear`），前部必须在远平面之前（`> -zfar`）

---

## Frustum 平面

- 由 projection 矩阵推导：`row3 ± row0`, `row3 ± row1`
- 左右、上下平面测试：`center.z * frustum[i] - abs(center.x/y) * frustum[j] > -radius`

---

## Cone 背面剔除

- `coneCull(center, radius, cone_axis, cone_cutoff, camera_pos)`：当 meshlet 法线锥完全背对相机时返回 true（应剔除）
- `clusterBackfaceEnabled == 0` 时禁用 cone 剔除

---

## 相关文件

| 文件 | 说明 |
|------|------|
| `shaders/NiagaraMeshlet.slang` | AS `ampMain` 剔除逻辑（约 133–139 行） |
| `Niagara.cpp` | `NiagaraGlobals` 与 frustum 平面计算 |
