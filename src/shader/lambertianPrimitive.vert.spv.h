constexpr char lambertianPrimitive_vert_spv[] = { '\x03', '\x02', '\x23', '\x07', '\x00', '\x00', '\x01', '\x00', '\x0b', '\x00', '\x0d', '\x00', '\x50', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x11', '\x00', '\x02', '\x00', '\x01', '\x00', '\x00', '\x00', '\x0b', '\x00', '\x06', '\x00', '\x01', '\x00', '\x00', '\x00', '\x47', '\x4c', '\x53', '\x4c', '\x2e', '\x73', '\x74', '\x64', '\x2e', '\x34', '\x35', '\x30', '\x00', '\x00', '\x00', '\x00', '\x0e', '\x00', '\x03', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x0f', '\x00', '\x0c', '\x00', '\x00', '\x00', '\x00', '\x00', '\x04', '\x00', '\x00', '\x00', '\x6d', '\x61', '\x69', '\x6e', '\x00', '\x00', '\x00', '\x00', '\x0d', '\x00', '\x00', '\x00', '\x23', '\x00', '\x00', '\x00', '\x31', '\x00', '\x00', '\x00', '\x3e', '\x00', '\x00', '\x00', '\x49', '\x00', '\x00', '\x00', '\x4c', '\x00', '\x00', '\x00', '\x4e', '\x00', '\x00', '\x00', '\x03', '\x00', '\x03', '\x00', '\x02', '\x00', '\x00', '\x00', '\xc2', '\x01', '\x00', '\x00', '\x04', '\x00', '\x0a', '\x00', '\x47', '\x4c', '\x5f', '\x47', '\x4f', '\x4f', '\x47', '\x4c', '\x45', '\x5f', '\x63', '\x70', '\x70', '\x5f', '\x73', '\x74', '\x79', '\x6c', '\x65', '\x5f', '\x6c', '\x69', '\x6e', '\x65', '\x5f', '\x64', '\x69', '\x72', '\x65', '\x63', '\x74', '\x69', '\x76', '\x65', '\x00', '\x00', '\x04', '\x00', '\x08', '\x00', '\x47', '\x4c', '\x5f', '\x47', '\x4f', '\x4f', '\x47', '\x4c', '\x45', '\x5f', '\x69', '\x6e', '\x63', '\x6c', '\x75', '\x64', '\x65', '\x5f', '\x64', '\x69', '\x72', '\x65', '\x63', '\x74', '\x69', '\x76', '\x65', '\x00', '\x05', '\x00', '\x04', '\x00', '\x04', '\x00', '\x00', '\x00', '\x6d', '\x61', '\x69', '\x6e', '\x00', '\x00', '\x00', '\x00', '\x05', '\x00', '\x06', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x67', '\x6c', '\x5f', '\x50', '\x65', '\x72', '\x56', '\x65', '\x72', '\x74', '\x65', '\x78', '\x00', '\x00', '\x00', '\x00', '\x06', '\x00', '\x06', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x67', '\x6c', '\x5f', '\x50', '\x6f', '\x73', '\x69', '\x74', '\x69', '\x6f', '\x6e', '\x00', '\x06', '\x00', '\x07', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x67', '\x6c', '\x5f', '\x50', '\x6f', '\x69', '\x6e', '\x74', '\x53', '\x69', '\x7a', '\x65', '\x00', '\x00', '\x00', '\x00', '\x06', '\x00', '\x07', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x67', '\x6c', '\x5f', '\x43', '\x6c', '\x69', '\x70', '\x44', '\x69', '\x73', '\x74', '\x61', '\x6e', '\x63', '\x65', '\x00', '\x06', '\x00', '\x07', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x67', '\x6c', '\x5f', '\x43', '\x75', '\x6c', '\x6c', '\x44', '\x69', '\x73', '\x74', '\x61', '\x6e', '\x63', '\x65', '\x00', '\x05', '\x00', '\x03', '\x00', '\x0d', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x05', '\x00', '\x07', '\x00', '\x11', '\x00', '\x00', '\x00', '\x43', '\x61', '\x6d', '\x65', '\x72', '\x61', '\x50', '\x61', '\x72', '\x61', '\x6d', '\x65', '\x74', '\x65', '\x72', '\x73', '\x00', '\x00', '\x00', '\x00', '\x06', '\x00', '\x06', '\x00', '\x11', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x70', '\x72', '\x6f', '\x6a', '\x65', '\x63', '\x74', '\x69', '\x6f', '\x6e', '\x00', '\x00', '\x06', '\x00', '\x05', '\x00', '\x11', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x76', '\x69', '\x65', '\x77', '\x00', '\x00', '\x00', '\x00', '\x06', '\x00', '\x05', '\x00', '\x11', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x76', '\x69', '\x65', '\x77', '\x50', '\x6f', '\x73', '\x00', '\x05', '\x00', '\x07', '\x00', '\x13', '\x00', '\x00', '\x00', '\x63', '\x61', '\x6d', '\x65', '\x72', '\x61', '\x50', '\x61', '\x72', '\x61', '\x6d', '\x65', '\x74', '\x65', '\x72', '\x73', '\x00', '\x00', '\x00', '\x00', '\x05', '\x00', '\x06', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x4d', '\x6f', '\x64', '\x65', '\x6c', '\x54', '\x72', '\x61', '\x6e', '\x73', '\x66', '\x6f', '\x72', '\x6d', '\x73', '\x00', '\x06', '\x00', '\x05', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x6d', '\x6f', '\x64', '\x65', '\x6c', '\x00', '\x00', '\x00', '\x06', '\x00', '\x05', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x6e', '\x6f', '\x72', '\x6d', '\x61', '\x6c', '\x00', '\x00', '\x05', '\x00', '\x06', '\x00', '\x1d', '\x00', '\x00', '\x00', '\x6d', '\x6f', '\x64', '\x65', '\x6c', '\x54', '\x72', '\x61', '\x6e', '\x73', '\x66', '\x6f', '\x72', '\x6d', '\x73', '\x00', '\x05', '\x00', '\x05', '\x00', '\x23', '\x00', '\x00', '\x00', '\x69', '\x6e', '\x50', '\x6f', '\x73', '\x69', '\x74', '\x69', '\x6f', '\x6e', '\x00', '\x00', '\x05', '\x00', '\x05', '\x00', '\x31', '\x00', '\x00', '\x00', '\x6f', '\x75', '\x74', '\x50', '\x6f', '\x73', '\x69', '\x74', '\x69', '\x6f', '\x6e', '\x00', '\x05', '\x00', '\x05', '\x00', '\x3e', '\x00', '\x00', '\x00', '\x6f', '\x75', '\x74', '\x4e', '\x6f', '\x72', '\x6d', '\x61', '\x6c', '\x00', '\x00', '\x00', '\x05', '\x00', '\x05', '\x00', '\x49', '\x00', '\x00', '\x00', '\x69', '\x6e', '\x4e', '\x6f', '\x72', '\x6d', '\x61', '\x6c', '\x00', '\x00', '\x00', '\x00', '\x05', '\x00', '\x05', '\x00', '\x4c', '\x00', '\x00', '\x00', '\x6f', '\x75', '\x74', '\x43', '\x6f', '\x6c', '\x6f', '\x72', '\x00', '\x00', '\x00', '\x00', '\x05', '\x00', '\x04', '\x00', '\x4e', '\x00', '\x00', '\x00', '\x69', '\x6e', '\x43', '\x6f', '\x6c', '\x6f', '\x72', '\x00', '\x48', '\x00', '\x05', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x04', '\x00', '\x00', '\x00', '\x47', '\x00', '\x03', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x48', '\x00', '\x04', '\x00', '\x11', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x05', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x11', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x23', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x11', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x07', '\x00', '\x00', '\x00', '\x10', '\x00', '\x00', '\x00', '\x48', '\x00', '\x04', '\x00', '\x11', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x05', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x11', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x23', '\x00', '\x00', '\x00', '\x40', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x11', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x07', '\x00', '\x00', '\x00', '\x10', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x11', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x23', '\x00', '\x00', '\x00', '\x80', '\x00', '\x00', '\x00', '\x47', '\x00', '\x03', '\x00', '\x11', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x13', '\x00', '\x00', '\x00', '\x22', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x13', '\x00', '\x00', '\x00', '\x21', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x48', '\x00', '\x04', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x05', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x23', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x07', '\x00', '\x00', '\x00', '\x10', '\x00', '\x00', '\x00', '\x48', '\x00', '\x04', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x05', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x23', '\x00', '\x00', '\x00', '\x40', '\x00', '\x00', '\x00', '\x48', '\x00', '\x05', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x07', '\x00', '\x00', '\x00', '\x10', '\x00', '\x00', '\x00', '\x47', '\x00', '\x03', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x1d', '\x00', '\x00', '\x00', '\x22', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x1d', '\x00', '\x00', '\x00', '\x21', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x23', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x31', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x3e', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x49', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x4c', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x47', '\x00', '\x04', '\x00', '\x4e', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x13', '\x00', '\x02', '\x00', '\x02', '\x00', '\x00', '\x00', '\x21', '\x00', '\x03', '\x00', '\x03', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x16', '\x00', '\x03', '\x00', '\x06', '\x00', '\x00', '\x00', '\x20', '\x00', '\x00', '\x00', '\x17', '\x00', '\x04', '\x00', '\x07', '\x00', '\x00', '\x00', '\x06', '\x00', '\x00', '\x00', '\x04', '\x00', '\x00', '\x00', '\x15', '\x00', '\x04', '\x00', '\x08', '\x00', '\x00', '\x00', '\x20', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x2b', '\x00', '\x04', '\x00', '\x08', '\x00', '\x00', '\x00', '\x09', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x1c', '\x00', '\x04', '\x00', '\x0a', '\x00', '\x00', '\x00', '\x06', '\x00', '\x00', '\x00', '\x09', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x06', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x07', '\x00', '\x00', '\x00', '\x06', '\x00', '\x00', '\x00', '\x0a', '\x00', '\x00', '\x00', '\x0a', '\x00', '\x00', '\x00', '\x20', '\x00', '\x04', '\x00', '\x0c', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x0b', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x04', '\x00', '\x0c', '\x00', '\x00', '\x00', '\x0d', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x15', '\x00', '\x04', '\x00', '\x0e', '\x00', '\x00', '\x00', '\x20', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x2b', '\x00', '\x04', '\x00', '\x0e', '\x00', '\x00', '\x00', '\x0f', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x18', '\x00', '\x04', '\x00', '\x10', '\x00', '\x00', '\x00', '\x07', '\x00', '\x00', '\x00', '\x04', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x05', '\x00', '\x11', '\x00', '\x00', '\x00', '\x10', '\x00', '\x00', '\x00', '\x10', '\x00', '\x00', '\x00', '\x07', '\x00', '\x00', '\x00', '\x20', '\x00', '\x04', '\x00', '\x12', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x11', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x04', '\x00', '\x12', '\x00', '\x00', '\x00', '\x13', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x20', '\x00', '\x04', '\x00', '\x14', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x10', '\x00', '\x00', '\x00', '\x2b', '\x00', '\x04', '\x00', '\x0e', '\x00', '\x00', '\x00', '\x17', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x04', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x10', '\x00', '\x00', '\x00', '\x10', '\x00', '\x00', '\x00', '\x20', '\x00', '\x04', '\x00', '\x1c', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x1b', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x04', '\x00', '\x1c', '\x00', '\x00', '\x00', '\x1d', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x17', '\x00', '\x04', '\x00', '\x21', '\x00', '\x00', '\x00', '\x06', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x20', '\x00', '\x04', '\x00', '\x22', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x21', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x04', '\x00', '\x22', '\x00', '\x00', '\x00', '\x23', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x2b', '\x00', '\x04', '\x00', '\x06', '\x00', '\x00', '\x00', '\x25', '\x00', '\x00', '\x00', '\x00', '\x00', '\x80', '\x3f', '\x20', '\x00', '\x04', '\x00', '\x2b', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x07', '\x00', '\x00', '\x00', '\x2b', '\x00', '\x04', '\x00', '\x06', '\x00', '\x00', '\x00', '\x2d', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x40', '\x20', '\x00', '\x04', '\x00', '\x2e', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x06', '\x00', '\x00', '\x00', '\x20', '\x00', '\x04', '\x00', '\x30', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x21', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x04', '\x00', '\x30', '\x00', '\x00', '\x00', '\x31', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x04', '\x00', '\x30', '\x00', '\x00', '\x00', '\x3e', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x18', '\x00', '\x04', '\x00', '\x41', '\x00', '\x00', '\x00', '\x21', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x04', '\x00', '\x22', '\x00', '\x00', '\x00', '\x49', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x04', '\x00', '\x2b', '\x00', '\x00', '\x00', '\x4c', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\x20', '\x00', '\x04', '\x00', '\x4d', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x07', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x04', '\x00', '\x4d', '\x00', '\x00', '\x00', '\x4e', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x36', '\x00', '\x05', '\x00', '\x02', '\x00', '\x00', '\x00', '\x04', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x03', '\x00', '\x00', '\x00', '\xf8', '\x00', '\x02', '\x00', '\x05', '\x00', '\x00', '\x00', '\x41', '\x00', '\x05', '\x00', '\x14', '\x00', '\x00', '\x00', '\x15', '\x00', '\x00', '\x00', '\x13', '\x00', '\x00', '\x00', '\x0f', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x04', '\x00', '\x10', '\x00', '\x00', '\x00', '\x16', '\x00', '\x00', '\x00', '\x15', '\x00', '\x00', '\x00', '\x41', '\x00', '\x05', '\x00', '\x14', '\x00', '\x00', '\x00', '\x18', '\x00', '\x00', '\x00', '\x13', '\x00', '\x00', '\x00', '\x17', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x04', '\x00', '\x10', '\x00', '\x00', '\x00', '\x19', '\x00', '\x00', '\x00', '\x18', '\x00', '\x00', '\x00', '\x92', '\x00', '\x05', '\x00', '\x10', '\x00', '\x00', '\x00', '\x1a', '\x00', '\x00', '\x00', '\x16', '\x00', '\x00', '\x00', '\x19', '\x00', '\x00', '\x00', '\x41', '\x00', '\x05', '\x00', '\x14', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x00', '\x00', '\x1d', '\x00', '\x00', '\x00', '\x0f', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x04', '\x00', '\x10', '\x00', '\x00', '\x00', '\x1f', '\x00', '\x00', '\x00', '\x1e', '\x00', '\x00', '\x00', '\x92', '\x00', '\x05', '\x00', '\x10', '\x00', '\x00', '\x00', '\x20', '\x00', '\x00', '\x00', '\x1a', '\x00', '\x00', '\x00', '\x1f', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x04', '\x00', '\x21', '\x00', '\x00', '\x00', '\x24', '\x00', '\x00', '\x00', '\x23', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x06', '\x00', '\x00', '\x00', '\x26', '\x00', '\x00', '\x00', '\x24', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x06', '\x00', '\x00', '\x00', '\x27', '\x00', '\x00', '\x00', '\x24', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x06', '\x00', '\x00', '\x00', '\x28', '\x00', '\x00', '\x00', '\x24', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x50', '\x00', '\x07', '\x00', '\x07', '\x00', '\x00', '\x00', '\x29', '\x00', '\x00', '\x00', '\x26', '\x00', '\x00', '\x00', '\x27', '\x00', '\x00', '\x00', '\x28', '\x00', '\x00', '\x00', '\x25', '\x00', '\x00', '\x00', '\x91', '\x00', '\x05', '\x00', '\x07', '\x00', '\x00', '\x00', '\x2a', '\x00', '\x00', '\x00', '\x20', '\x00', '\x00', '\x00', '\x29', '\x00', '\x00', '\x00', '\x41', '\x00', '\x05', '\x00', '\x2b', '\x00', '\x00', '\x00', '\x2c', '\x00', '\x00', '\x00', '\x0d', '\x00', '\x00', '\x00', '\x0f', '\x00', '\x00', '\x00', '\x3e', '\x00', '\x03', '\x00', '\x2c', '\x00', '\x00', '\x00', '\x2a', '\x00', '\x00', '\x00', '\x41', '\x00', '\x05', '\x00', '\x2e', '\x00', '\x00', '\x00', '\x2f', '\x00', '\x00', '\x00', '\x0d', '\x00', '\x00', '\x00', '\x17', '\x00', '\x00', '\x00', '\x3e', '\x00', '\x03', '\x00', '\x2f', '\x00', '\x00', '\x00', '\x2d', '\x00', '\x00', '\x00', '\x41', '\x00', '\x05', '\x00', '\x14', '\x00', '\x00', '\x00', '\x32', '\x00', '\x00', '\x00', '\x1d', '\x00', '\x00', '\x00', '\x0f', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x04', '\x00', '\x10', '\x00', '\x00', '\x00', '\x33', '\x00', '\x00', '\x00', '\x32', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x04', '\x00', '\x21', '\x00', '\x00', '\x00', '\x34', '\x00', '\x00', '\x00', '\x23', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x06', '\x00', '\x00', '\x00', '\x35', '\x00', '\x00', '\x00', '\x34', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x06', '\x00', '\x00', '\x00', '\x36', '\x00', '\x00', '\x00', '\x34', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x06', '\x00', '\x00', '\x00', '\x37', '\x00', '\x00', '\x00', '\x34', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x50', '\x00', '\x07', '\x00', '\x07', '\x00', '\x00', '\x00', '\x38', '\x00', '\x00', '\x00', '\x35', '\x00', '\x00', '\x00', '\x36', '\x00', '\x00', '\x00', '\x37', '\x00', '\x00', '\x00', '\x25', '\x00', '\x00', '\x00', '\x91', '\x00', '\x05', '\x00', '\x07', '\x00', '\x00', '\x00', '\x39', '\x00', '\x00', '\x00', '\x33', '\x00', '\x00', '\x00', '\x38', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x06', '\x00', '\x00', '\x00', '\x3a', '\x00', '\x00', '\x00', '\x39', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x06', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x00', '\x00', '\x39', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x06', '\x00', '\x00', '\x00', '\x3c', '\x00', '\x00', '\x00', '\x39', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x50', '\x00', '\x06', '\x00', '\x21', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x00', '\x00', '\x3a', '\x00', '\x00', '\x00', '\x3b', '\x00', '\x00', '\x00', '\x3c', '\x00', '\x00', '\x00', '\x3e', '\x00', '\x03', '\x00', '\x31', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x00', '\x00', '\x41', '\x00', '\x05', '\x00', '\x14', '\x00', '\x00', '\x00', '\x3f', '\x00', '\x00', '\x00', '\x1d', '\x00', '\x00', '\x00', '\x17', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x04', '\x00', '\x10', '\x00', '\x00', '\x00', '\x40', '\x00', '\x00', '\x00', '\x3f', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x07', '\x00', '\x00', '\x00', '\x42', '\x00', '\x00', '\x00', '\x40', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x4f', '\x00', '\x08', '\x00', '\x21', '\x00', '\x00', '\x00', '\x43', '\x00', '\x00', '\x00', '\x42', '\x00', '\x00', '\x00', '\x42', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x07', '\x00', '\x00', '\x00', '\x44', '\x00', '\x00', '\x00', '\x40', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x4f', '\x00', '\x08', '\x00', '\x21', '\x00', '\x00', '\x00', '\x45', '\x00', '\x00', '\x00', '\x44', '\x00', '\x00', '\x00', '\x44', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x51', '\x00', '\x05', '\x00', '\x07', '\x00', '\x00', '\x00', '\x46', '\x00', '\x00', '\x00', '\x40', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x4f', '\x00', '\x08', '\x00', '\x21', '\x00', '\x00', '\x00', '\x47', '\x00', '\x00', '\x00', '\x46', '\x00', '\x00', '\x00', '\x46', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x02', '\x00', '\x00', '\x00', '\x50', '\x00', '\x06', '\x00', '\x41', '\x00', '\x00', '\x00', '\x48', '\x00', '\x00', '\x00', '\x43', '\x00', '\x00', '\x00', '\x45', '\x00', '\x00', '\x00', '\x47', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x04', '\x00', '\x21', '\x00', '\x00', '\x00', '\x4a', '\x00', '\x00', '\x00', '\x49', '\x00', '\x00', '\x00', '\x91', '\x00', '\x05', '\x00', '\x21', '\x00', '\x00', '\x00', '\x4b', '\x00', '\x00', '\x00', '\x48', '\x00', '\x00', '\x00', '\x4a', '\x00', '\x00', '\x00', '\x3e', '\x00', '\x03', '\x00', '\x3e', '\x00', '\x00', '\x00', '\x4b', '\x00', '\x00', '\x00', '\x3d', '\x00', '\x04', '\x00', '\x07', '\x00', '\x00', '\x00', '\x4f', '\x00', '\x00', '\x00', '\x4e', '\x00', '\x00', '\x00', '\x3e', '\x00', '\x03', '\x00', '\x4c', '\x00', '\x00', '\x00', '\x4f', '\x00', '\x00', '\x00', '\xfd', '\x00', '\x01', '\x00', '\x38', '\x00', '\x01', '\x00' };
