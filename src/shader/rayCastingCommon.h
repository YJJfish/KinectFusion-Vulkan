/** @brief	Helper function that compute the min intersection distance.
  */
float getMinT(vec3 rayOrigin, vec3 rayDir, vec3 minCorner, vec3 maxCorner) {
	float xMin = ((rayDir.x > 0.0 ? minCorner.x : maxCorner.x) - rayOrigin.x) / rayDir.x;
	float yMin = ((rayDir.y > 0.0 ? minCorner.y : maxCorner.y) - rayOrigin.y) / rayDir.y;
	float zMin = ((rayDir.z > 0.0 ? minCorner.z : maxCorner.z) - rayOrigin.z) / rayDir.z;
	return max(max(xMin, yMin), zMin);
}

/** @brief	Helper function that compute the max intersection distance.
  */
float getMaxT(vec3 rayOrigin, vec3 rayDir, vec3 minCorner, vec3 maxCorner) {
	float xMax = ((rayDir.x > 0.0 ? maxCorner.x : minCorner.x) - rayOrigin.x) / rayDir.x;
	float yMax = ((rayDir.y > 0.0 ? maxCorner.y : minCorner.y) - rayOrigin.y) / rayDir.y;
	float zMax = ((rayDir.z > 0.0 ? maxCorner.z : minCorner.z) - rayOrigin.z) / rayDir.z;
	return min(min(xMax, yMax), zMax);
}

/** @brief	Compute the coefficients of trilinear interpolation.
  * @sa		https://en.wikipedia.org/wiki/Trilinear_interpolation#Alternative_algorithm
  *	@param	tsdf			The 8 nearest voxels' TSDF value
  *	@param	coeff			Output coefficients.
  *
  *			Since we normalized the position to [0, 1]^3, the formula can be simplified
  *			a lot. (Plugging x0=y0=z0=0 and x1=y1=z1=1 into the original formula.)
  */
void getCoefficients(in float tsdf[2][2][2], out float coeff[8]) {
	coeff[0] = tsdf[0][0][0];
	coeff[1] = -tsdf[0][0][0] + tsdf[1][0][0];
	coeff[2] = -tsdf[0][0][0] + tsdf[0][1][0];
	coeff[3] = -tsdf[0][0][0] + tsdf[0][0][1];
	coeff[4] = tsdf[0][0][0] - tsdf[0][1][0] - tsdf[1][0][0] + tsdf[1][1][0];
	coeff[5] = tsdf[0][0][0] - tsdf[0][0][1] - tsdf[1][0][0] + tsdf[1][0][1];
	coeff[6] = tsdf[0][0][0] - tsdf[0][0][1] - tsdf[0][1][0] + tsdf[0][1][1];
	coeff[7] = -tsdf[0][0][0] + tsdf[0][0][1] + tsdf[0][1][0] - tsdf[0][1][1] + tsdf[1][0][0] - tsdf[1][0][1] - tsdf[1][1][0] + tsdf[1][1][1];
}

/** @brief	Compute the coefficients of trilinear interpolation.
  * @sa		https://en.wikipedia.org/wiki/Trilinear_interpolation#Alternative_algorithm
  *	@param	color			The 8 nearest voxels' color value
  *	@param	coeff			Output coefficients.
  */
void getCoefficients(in vec4 color[2][2][2], out vec4 coeff[8]) {
	coeff[0] = color[0][0][0];
	coeff[1] = -color[0][0][0] + color[1][0][0];
	coeff[2] = -color[0][0][0] + color[0][1][0];
	coeff[3] = -color[0][0][0] + color[0][0][1];
	coeff[4] = color[0][0][0] - color[0][1][0] - color[1][0][0] + color[1][1][0];
	coeff[5] = color[0][0][0] - color[0][0][1] - color[1][0][0] + color[1][0][1];
	coeff[6] = color[0][0][0] - color[0][0][1] - color[0][1][0] + color[0][1][1];
	coeff[7] = -color[0][0][0] + color[0][0][1] + color[0][1][0] - color[0][1][1] + color[1][0][0] - color[1][0][1] - color[1][1][0] + color[1][1][1];
}

/** @brief	Helper function to get the base index of the voxel for a world space position.
  */
uvec3 getBaseIndex(vec3 pos) {
	uvec3 baseIndex = uvec3((pos - tsdfVolume.corner) / tsdfVolume.size);
	baseIndex[0] = min(max(baseIndex[0], 0), tsdfVolume.resolution[0] - 1);
	baseIndex[1] = min(max(baseIndex[1], 0), tsdfVolume.resolution[1] - 1);
	baseIndex[2] = min(max(baseIndex[2], 0), tsdfVolume.resolution[2] - 1);
	return baseIndex;
}

/** @brief	Helper function to interpolate the TSDF value.
  * @note	It's the caller's reponsibility to make sure `pos` is within valid range.
  * @param	pos		The position in world space.
  * @param	valid	Indicate whether the 8 corners contain zero-weighted voxels.
  * @return			The interpolated TSDF value.
  */
float interpolateTSDF(in vec3 pos, out bool valid) {
	valid = true;
	uvec3 baseIndex = getBaseIndex(pos);
	// Normalize pos to [0, 1]^3
	vec3 normalizedPos = (pos - tsdfVolume.corner) / tsdfVolume.size - vec3(baseIndex);
	// Read 8 nearest voxels.
	float tsdf[2][2][2];
	for (uint dx = 0; dx < 2; ++dx)
		for (uint dy = 0; dy < 2; ++dy)
			for (uint dz = 0; dz < 2; ++dz) {
				int weight;
				unpackVoxel(readVoxelData(baseIndex + uvec3(dx, dy, dz)).x, tsdf[dx][dy][dz], weight);
				if (weight == 0) valid = false;
			}
	// Interpolate
	float coeff[8];
	getCoefficients(tsdf, coeff);
	float interpolated = \
		coeff[0] +
		coeff[1] * normalizedPos.x + \
		coeff[2] * normalizedPos.y + \
		coeff[3] * normalizedPos.z + \
		coeff[4] * normalizedPos.x * normalizedPos.y + \
		coeff[5] * normalizedPos.x * normalizedPos.z + \
		coeff[6] * normalizedPos.y * normalizedPos.z + \
		coeff[7] * normalizedPos.x * normalizedPos.y * normalizedPos.z;
	return interpolated;
}

/** @brief	Helper function to interpolate the color value.
  * @note	It's the caller's reponsibility to make sure `pos` is within valid range.
  * @param	pos		The position in world space.
  * @return			The interpolated color value.
  */
vec4 interpolateColor(vec3 pos) {
	uvec3 baseIndex = getBaseIndex(pos);
	// Normalize pos to [0, 1]^3
	vec3 normalizedPos = (pos - tsdfVolume.corner) / tsdfVolume.size - vec3(baseIndex);
	// Read 8 nearest voxels.
	vec4 color[2][2][2];
	for (uint dx = 0; dx < 2; ++dx)
		for (uint dy = 0; dy < 2; ++dy)
			for (uint dz = 0; dz < 2; ++dz) {
				unpackColor(readVoxelData(baseIndex + uvec3(dx, dy, dz)).y, color[dx][dy][dz]);
			}
	// Interpolate
	vec4 coeff[8];
	getCoefficients(color, coeff);
	vec4 interpolated = \
		coeff[0] +
		coeff[1] * normalizedPos.x + \
		coeff[2] * normalizedPos.y + \
		coeff[3] * normalizedPos.z + \
		coeff[4] * normalizedPos.x * normalizedPos.y + \
		coeff[5] * normalizedPos.x * normalizedPos.z + \
		coeff[6] * normalizedPos.y * normalizedPos.z + \
		coeff[7] * normalizedPos.x * normalizedPos.y * normalizedPos.z;
	return interpolated;
}

/** @brief	Helper function to compute the normal at a point on the zero-surface.
  * @note	It's the caller's reponsibility to make sure `pos` is within valid range.
  * @param	pos		The position in world space. It should be on the zero-surface.
  * @return			The interpolated TSDF value.
  */
vec3 computeNormal(vec3 pos) {
	uvec3 baseIndex = getBaseIndex(pos);
	// Normalize pos to [0, 1]^3
	vec3 normalizedPos = (pos - tsdfVolume.corner) / tsdfVolume.size - vec3(baseIndex);
	// Read 8 nearest voxels.
	float tsdf[2][2][2];
	for (uint dx = 0; dx < 2; ++dx)
		for (uint dy = 0; dy < 2; ++dy)
			for (uint dz = 0; dz < 2; ++dz) {
				int weight;
				unpackVoxel(readVoxelData(baseIndex + uvec3(dx, dy, dz)).x, tsdf[dx][dy][dz], weight);
			}
	// Get the coefficients of trilinear interpolation.
	float coeff[8];
	getCoefficients(tsdf, coeff);
	// The normal should be the gradient of the interpolated TSDF.
	vec3 normal = normalize(vec3(
		coeff[1] + coeff[4] * normalizedPos.y + coeff[5] * normalizedPos.z + coeff[7] * normalizedPos.y * normalizedPos.z,
		coeff[2] + coeff[4] * normalizedPos.x + coeff[6] * normalizedPos.z + coeff[7] * normalizedPos.x * normalizedPos.z,
		coeff[3] + coeff[5] * normalizedPos.x + coeff[6] * normalizedPos.y + coeff[7] * normalizedPos.x * normalizedPos.y
	));
	return normal;
}

/** @brief	Function that actually does ray casting.
  *	@return	Ray marching length, in meter. Note that this is not the depth value.
  *			If the ray does not intersect with a zero surface, +infinity will be returned.
  */
float rayCasting(vec3 rayOrigin, vec3 rayDir, float minLength, float maxLength) {
	// Compute t range
	rayDir.x = (rayDir.x == 0.0) ? 1e-5 : rayDir.x;
	rayDir.y = (rayDir.y == 0.0) ? 1e-5 : rayDir.y;
	rayDir.z = (rayDir.z == 0.0) ? 1e-5 : rayDir.z;
	vec3 minCorner = tsdfVolume.corner;
	vec3 maxCorner = tsdfVolume.corner + vec3(tsdfVolume.resolution - 1) * tsdfVolume.size;
	float minT = getMinT(rayOrigin, rayDir, minCorner, maxCorner);
	minT = max(minT, minLength);
	float maxT = getMaxT(rayOrigin, rayDir, minCorner, maxCorner);
	maxT = min(maxT, maxLength);
	if (minT >= maxT)
		return (1.0 / 0.0);
	// Start ray casting
	float currT = minT + 1e-5;
	float lastT = -1.0;
	float currTSDF = -1.0;
	float lastTSDF = -1.0;
	while (currT < maxT) {
		bool valid = true;
		currTSDF = interpolateTSDF(rayOrigin + rayDir * currT, valid);
		if (!valid) {
			// One or more surrounding voxels are zero-weighted. Continue marching.
			lastT = currT;
			currT += tsdfVolume.truncationDistance * 0.95;
			lastTSDF = -1.0;
			continue;
		}
		else if (currTSDF > 1e-5) {
			// Still outside of the zero-surface. Continue marching.
			lastT = currT;
			currT += max(rayCastingParameters.marchingStep, tsdfVolume.truncationDistance * currTSDF * 0.95);
			lastTSDF = currTSDF;
			continue;
		}
		else if (currTSDF < -1e-5) {
			// Inside of the zero-surface.
			if (lastTSDF > 0.0) {
				// Find zero-surface.
				float t = lastT + (currT - lastT) * lastTSDF / (lastTSDF - currTSDF);
				return t;
			}
			else {
				// Failed.
				return (1.0f / 0.0f);
			}
		}
		else {
			// Find zero-surface.
			float t = currT;
			return t;
		}
	}
	return (1.0f / 0.0f);
}