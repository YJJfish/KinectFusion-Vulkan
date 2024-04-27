/** @brief	Helper function to pack float TSDF and integer weight into two shorts.
  */
void packVoxel(in float tsdf, in int weight, out int packedVoxel) {
	packedVoxel = (int(tsdf * 32767.0) << 16) | weight;
}

/** @brief	Helper function to unpack two shorts to float TSDF and integer weight.
  */
void unpackVoxel(in int packedVoxel, out float tsdf, out int weight) {
	tsdf = float(packedVoxel >> 16) * (1.0 / 32767.0);
	weight = packedVoxel & 0x0000FFFF;
}

/** @brief	Helper function to pack 4 channel color into an int.
  */
void packColor(in vec4 color, out int packedColor) {
	packedColor = int(packUnorm4x8(color));
}

/** @brief	Helper function to unpack an int to 4 channel color.
  */
void unpackColor(in int packedColor, out vec4 color) {
	color = unpackUnorm4x8(uint(packedColor));
}

/** @brief	Helper function to read a voxel.
  * @note	It's the caller's reponsibility to make sure `index` is within valid range.
  */
ivec2 readVoxelData(uvec3 index) {
	return tsdfVolume.data[(index.x *  tsdfVolume.resolution.y + index.y) * tsdfVolume.resolution.z + index.z];
}