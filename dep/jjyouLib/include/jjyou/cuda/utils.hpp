#ifndef jjyou_cuda_utils_hpp
#define jjyou_cuda_utils_hpp

#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cstdio>

namespace jjyou {
	namespace cuda {
		namespace utils {
			static inline void cudaSafeCall(cudaError_t err, const char* msg = nullptr) {
				if (cudaSuccess != err) {
					printf("CUDA error(%s): %s\n", msg, cudaGetErrorString(err));
					exit(-1);
				}
			}
			template <class IntegerType> static inline IntegerType divUp(IntegerType a, IntegerType b) {
				return (a + b - 1) / b;
			}
		}
	}
}

#endif /* jjyou_cuda_Utils_hpp */