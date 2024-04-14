/***********************************************************************
 * @file	DeviceArray.hpp
 * @author	jjyou
 * @date	2023-5-23
 * @brief	This file implements DeviceArray1D, DeviceArray2D,
 *			DeviceArray3D classes.
***********************************************************************/
#ifndef jjyou_cuda_DeviceArray_hpp
#define jjyou_cuda_DeviceArray_hpp

#include <cuda_runtime.h>
#include "utils.hpp"
namespace jjyou {
	namespace cuda {

		/***********************************************************************
		 * @class DeviceArray1D
		 * @brief One-dimensional device array class.
		 *
		 * This class provides C++ API for manipulating
		 * one-dimensional device array.
		 * @param T		Element type.
		 ***********************************************************************/
		template <class T>
		class DeviceArray1D {
		public:

			/***********************************************************************
			 * @struct Data
			 * @brief Struct for reading/writing device array in kernel functions.
			 *
			 * This struct provides device API for reading/writing
			 * one-dimensional device array in kernel functions.
			 ***********************************************************************/
			struct Data {

				/** @brief Pointer to data.
				  */
				T* data;

				/** @brief Length of array.
				  */
				size_t length;

				/** @brief Constructor.
				  *
				  * @param data		Pointer to data.
				  * @param length	Length of array.
				  */
				Data(T* data, size_t length);

				/** @brief Fetch the element at the specified position.
				  *
				  * @param pos		Position of the desired element.
				  * @return			Reference to the fetched element.
				  */
				__device__ T& operator[](size_t pos);

				/** @brief Fetch the element at the specified position.
				  *
				  * @param pos		Position of the desired element.
				  * @return			Const reference to the fetched element.
				  */
				__device__ const T& operator[](size_t pos) const;

				/** @brief Fetch the element at the specified position.
				  *
				  * @param pos		Position of the desired element.
				  * @return			Reference to the fetched element.
				  */
				__device__ T& operator()(size_t pos);

				/** @brief Fetch the element at the specified position.
				  *
				  * @param pos		Position of the desired element.
				  * @return			Const reference to the fetched element.
				  */
				__device__ const T& operator()(size_t pos) const;
			};

			/***********************************************************************
			 * @struct ConstData
			 * @brief Struct for reading device array in kernel functions.
			 *
			 * This struct provides device API for reading
			 * one-dimensional device array in kernel functions.
			 ***********************************************************************/
			struct ConstData {

				/** @brief Pointer to data.
				  */
				const T* data;

				/** @brief Length of array.
				  */
				size_t length;

				/** @brief Constructor.
				  *
				  * @param data		Pointer to data.
				  * @param length	Length of array.
				  */
				ConstData(const T* data, size_t length);

				/** @brief Fetch the element at the specified position.
				  *
				  * @param pos		Position of the desired element.
				  * @return			Const reference to the fetched element.
				  */
				__device__ const T& operator[](size_t pos) const;

				/** @brief Fetch the element at the specified position.
				  *
				  * @param pos		Position of the desired element.
				  * @return			Const reference to the fetched element.
				  */
				__device__ const T& operator()(size_t pos) const;
			};

		public:

			/** @brief Construct an empty instance.
			  */
			DeviceArray1D(void);

			/** @brief Construct and allocate memory.
			  *
			  * @param length	Length of array.
			  */
			DeviceArray1D(size_t length);

			/** @brief Construct with given memory address.
			  *
			  * @param length		Length of array.
			  * @param deviceData	Pointer to user-allocated memory in GPU.
			  *
			  * @note Reference counting is disabled in this case.
			  */
			DeviceArray1D(size_t length, T* deviceData);

			/** @brief Copy constructor.
			  *
			  * No copy is performed. Simply increment the reference counter
			  * by 1 (if reference counting is enabled).
			  */
			DeviceArray1D(const DeviceArray1D<T>& other);

			/** @brief Destructor.
			  */
			~DeviceArray1D(void);

			/** @brief Assignment.
			  *
			  * No copy is performed. Simply increment the reference counter
			  * by 1 (if reference counting is enabled).
			  */
			DeviceArray1D<T>& operator=(const DeviceArray1D<T>& other);

			/** @brief Create an array with specified length.
			  *
			  * If the current array length is different from \p length,
			  * the old buffer is deallocated and a new one is allocated.
			  * Otherwise, do nothing.
			  *
			  * @param length		Length of array.
			  */
			void create(size_t length);

			/** @brief Copy data from host memory to device memory.
			  *
			  * DeviceArray1D::create will be called before copying data,
			  * which means if the current array length is different from \p length,
			  * the old buffer will be deallocated.
			  *
			  * @param hostData		Pointer to host memory.
			  * @param length		Length of array.
			  */
			void upload(const T* hostData, size_t length);

			/** @brief Copy data from device memory to host memory.
			  *
			  * The user should make sure there is enough valid memory space
			  * in \p hostData.
			  *
			  * @param hostData		Pointer to host memory.
			  */
			void download(T* hostData) const;

			/** @brief Copy data from a device array to another device array.
			  *
			  * DeviceArray1D::create will be called for \p other before copying data,
			  * which means if the length of current array is different from that of
			  * \p other, the old buffer of \p other will be deallocated.
			  *
			  * @param other		Another instance of DeviceArray1D.
			  */
			void copyTo(DeviceArray1D<T>& other) const;

			/** @brief Copy data from a device array to the
			  *		   specified device memory address.
			  *
			  * The user should make sure there is enough valid memory space
			  * in \p deviceData.
			  *
			  * @param deviceData	Pointer to device memory.
			  */
			void copyTo(T* deviceData) const;

			/** @brief Create a full copy of the current array.
			  *
			  * @return 	The cloned array.
			  */
			DeviceArray1D<T> clone(void) const;

			/** @brief Swap two arrays.
			  *
			  * @param other	Another instance of DeviceArray1D.
			  */
			void swap(DeviceArray1D<T>& other);

			/** @brief Release the current array.
			  *
			  * Decrements the reference counter and deallocate the internal memory
			  * if needed.
			  */
			void release(void);

			/** @brief Return `true` if the array has no element.
			  *
			  * @return `true` if the array has no element.
			  */
			bool empty(void) const;

			/** @brief Get the address of internal memory buffer.
			  *
			  * @return address of internal memory buffer.
			  */
			T* data(void);

			/** @brief Get the address of internal memory buffer.
			  *
			  * @return address of internal memory buffer.
			  */
			const T* data(void) const;

			/** @brief Get the value of the reference counter.
			  *
			  * @return the value of the reference counter if reference counting is
			  * enabled, otherwise 0.
			  */
			int referenceCounter(void) const;

			/** @brief Get the length of the array.
			  *
			  * @return the length of the array.
			  */
			size_t length(void) const;

			/** @brief Converse to DeviceArray1D::Data for
			  * passing to kernel functions.
			  */
			operator Data();

			/** @brief Converse to DeviceArray1D::ConstData for
			  * passing to kernel functions.
			  */
			operator ConstData() const;

		private:
			T* _data;
			int* _referenceCounter;
			size_t _length;
		};

		/***********************************************************************
		 * @class DeviceArray2D
		 * @brief Two-dimensional device array class.
		 *
		 * This class provides C++ API for manipulating
		 * two-dimensional device array.
		 * @param T		Element type.
		 ***********************************************************************/
		template <class T>
		class DeviceArray2D {
		public:
			/***********************************************************************
			 * @struct Data
			 * @brief Struct for reading/writing device array in kernel functions.
			 *
			 * This struct provides device API for reading/writing
			 * two-dimensional device array in kernel functions.
			 ***********************************************************************/
			struct Data {

				/** @brief Pointer to data.
				  */
				T* data;

				/** @brief Pitch of array.
				  */
				size_t pitch;

				/** @brief Size of array.
				  */
				size_t rows, cols;

				/** @brief Constructor.
				  *
				  * @param data		Pointer to data.
				  * @param pitch	Pitch of array.
				  * @param rows		Number of rows in array.
				  * @param cols		Number of columns in array.
				  */
				Data(T* data, size_t pitch, size_t rows, size_t cols);

				/** @brief Fetch a row of elements.
				  *
				  * @param row		The row number of the desired position.
				  * @return			Pointer to a row of elements.
				  */
				__device__ T* operator[](size_t row);

				/** @brief Fetch a row of elements.
				  *
				  * @param row		The row number of the desired position.
				  * @return			Pointer to a row of elements.
				  */
				__device__ const T* operator[](size_t row) const;

				/** @brief Fetch the element at the specified position.
				  *
				  * @param row		The row number of the desired element.
				  * @param col		The column number of the desired element.
				  * @return			Reference to the fetched element.
				  */
				__device__ T& operator()(size_t row, size_t col);

				/** @brief Fetch the element at the specified position.
				  *
				  * @param row		The row number of the desired element.
				  * @param col		The column number of the desired element.
				  * @return			Const reference to the fetched element.
				  */
				__device__ const T& operator()(size_t row, size_t col) const;
			};

			/***********************************************************************
			 * @struct ConstData
			 * @brief Struct for reading device array in kernel functions.
			 *
			 * This struct provides device API for reading
			 * two-dimensional device array in kernel functions.
			 ***********************************************************************/
			struct ConstData {

				/** @brief Pointer to data.
				  */
				const T* data;

				/** @brief Pitch of array.
				  */
				size_t pitch;

				/** @brief Size of array.
				  */
				size_t rows, cols;

				/** @brief Constructor.
				  *
				  * @param data		Pointer to data.
				  * @param pitch	Pitch of array.
				  * @param rows		Number of rows in array.
				  * @param cols		Number of columns in array.
				  */
				ConstData(const T* data, size_t pitch, size_t rows, size_t cols);

				/** @brief Fetch a row of elements.
				  *
				  * @param row		The row number of the desired position.
				  * @return			Pointer to a row of elements.
				  */
				__device__ const T* operator[](size_t row) const;

				/** @brief Fetch the element at the specified position.
				  *
				  * @param row		The row number of the desired element.
				  * @param col		The column number of the desired element.
				  * @return			Const reference to the fetched element.
				  */
				__device__ const T& operator()(size_t row, size_t col) const;
			};
		public:

			/** @brief Construct an empty instance.
			  */
			DeviceArray2D(void);

			/** @brief Construct and allocate memory.
			  *
			  * @param rows		Number of rows of array.
			  * @param cols		Number of columns of array.
			  */
			DeviceArray2D(size_t rows, size_t cols);

			/** @brief Construct with given memory address.
			  *
			  * @param rows			Number of rows of array.
			  * @param cols			Number of columns of array.
			  * @param deviceData	Pointer to user-allocated memory in GPU.
			  * @param pitch		Pitch of \p deviceData
			  *
			  * @note Reference counting is disabled in this case.
			  */
			DeviceArray2D(size_t rows, size_t cols, T* deviceData, size_t pitch);

			/** @brief Copy constructor.
			  *
			  * No copy is performed. Simply increment the reference counter
			  * by 1 (if reference counting is enabled).
			  */
			DeviceArray2D(const DeviceArray2D<T>& other);

			/** @brief Destructor.
			  */
			~DeviceArray2D(void);

			/** @brief Assignment.
			  *
			  * No copy is performed. Simply increment the reference counter
			  * by 1 (if reference counting is enabled).
			  */
			DeviceArray2D<T>& operator=(const DeviceArray2D<T>& other);

			/** @brief Create an array with specified size.
			  *
			  * If the current array size is different from the given parameters,
			  * the old buffer is deallocated and a new one is allocated.
			  * Otherwise, do nothing.
			  *
			  * @param rows			Number of rows of array.
			  * @param cols			Number of columns of array.
			  */
			void create(size_t rows, size_t cols);

			/** @brief Copy data from host memory to device memory.
			  *
			  * DeviceArray2D::create will be called before copying data,
			  * which means if the current array size is different from the given
			  * parameters, the old buffer will be deallocated.
			  *
			  * @param hostData		Pointer to host memory.
			  * @param rows			Number of rows of array.
			  * @param cols			Number of columns of array.
			  */
			void upload(const T* hostData, size_t rows, size_t cols);

			/** @brief Copy data from device memory to host memory.
			  *
			  * The user should make sure there is enough valid memory space
			  * in \p hostData.
			  *
			  * @param hostData		Pointer to host memory.
			  */
			void download(T* hostData) const;

			/** @brief Copy data from a device array to another device array.
			  *
			  * DeviceArray2D::create will be called for \p other before copying data,
			  * which means if the size of current array is different from that of
			  * \p other, the old buffer of \p other will be deallocated.
			  *
			  * @param other		Another instance of DeviceArray1D.
			  */
			void copyTo(DeviceArray2D<T>& other) const;

			/** @brief Copy data from a device array to the
			  *		   specified device memory address.
			  *
			  * The user should make sure there is enough valid memory space
			  * in \p deviceData.
			  *
			  * @param deviceData	Pointer to device memory.
			  * @param pitch		Pitch of \p deviceData
			  */
			void copyTo(T* deviceData, size_t pitch) const;

			/** @brief Create a full copy of the current array.
			  *
			  * @return 	The cloned array.
			  */
			DeviceArray2D<T> clone(void) const;

			/** @brief Swap two arrays.
			  *
			  * @param other	Another instance of DeviceArray2D.
			  */
			void swap(DeviceArray2D<T>& other);

			/** @brief Release the current array.
			  *
			  * Decrements the reference counter and deallocate the internal memory
			  * if needed.
			  */
			void release(void);

			/** @brief Return `true` if the array has no element.
			  *
			  * @return `true` if the array has no element.
			  */
			bool empty(void) const;

			/** @brief Get the address of internal memory buffer.
			  *
			  * @return address of internal memory buffer.
			  */
			T* data(void);

			/** @brief Get the address of internal memory buffer.
			  *
			  * @return address of internal memory buffer.
			  */
			const T* data(void) const;


			/** @brief Get the pitch of internal memory buffer.
			  *
			  * @return pitch of internal memory buffer.
			  */
			size_t pitch(void) const;

			/** @brief Get the value of the reference counter.
			  *
			  * @return the value of the reference counter if reference counting is
			  * enabled, otherwise 0.
			  */
			int referenceCounter(void) const;

			/** @brief Get the number of rows of the array.
			  *
			  * @return the number of rows of the array.
			  */
			size_t rows(void) const;

			/** @brief Get the number of columns of the array.
			  *
			  * @return the number of columns of the array.
			  */
			size_t cols(void) const;

			/** @brief Converse to DeviceArray2D::Data for
			  * passing to kernel functions.
			  */
			operator Data();

			/** @brief Converse to DeviceArray2D::ConstData for
			  * passing to kernel functions.
			  */
			operator ConstData() const;
		private:
			T* _data;
			size_t _pitch;
			int* _referenceCounter;
			size_t _rows, _cols;
		};
	}
}


/*======================================================================
 | Implementation
 ======================================================================*/
 /// @cond

#include <iostream>
namespace jjyou {
	namespace cuda {

		//Implementation of DeviceArray1D
		template <class T> DeviceArray1D<T>::DeviceArray1D(void) :
			_data(nullptr),
			_referenceCounter(nullptr),
			_length(0)
		{}

		template <class T> DeviceArray1D<T>::DeviceArray1D(size_t length) :
			_data(nullptr),
			_referenceCounter(nullptr),
			_length(0)
		{
			this->create(length);
		}

		template <class T> DeviceArray1D<T>::DeviceArray1D(size_t length, T* deviceData) :
			_data(deviceData),
			_referenceCounter(nullptr),
			_length(length)
		{}

		template <class T> DeviceArray1D<T>::DeviceArray1D(const DeviceArray1D<T>& other) :
			_data(other._data),
			_referenceCounter(other._referenceCounter),
			_length(other._length)
		{
			if (this->_referenceCounter)
				_InterlockedExchangeAdd((volatile long*)this->_referenceCounter, 1);
		}

		template <class T> DeviceArray1D<T>::~DeviceArray1D(void) {
			this->release();
		}

		template <class T> DeviceArray1D<T>& DeviceArray1D<T>::operator=(const DeviceArray1D<T>& other) {
			if (this == &other) return *this;
			if (other._referenceCounter)
				_InterlockedExchangeAdd((volatile long*)other._referenceCounter, 1);
			this->release();
			this->_data = other._data;
			this->_referenceCounter = other._referenceCounter;
			this->_length = other._length;
			return *this;
		}

		template <class T> void DeviceArray1D<T>::create(size_t length) {
			if (this->_length == length)
				return;
			this->release();
			if (length > 0) {
				this->_length = length;
				utils::cudaSafeCall(cudaMalloc((void**)&this->_data, this->_length * sizeof(T)), "DeviceArray1D::create, cudaMalloc");
				this->_referenceCounter = new int(1);
			}
		}

		template <class T> void DeviceArray1D<T>::upload(const T* hostData, size_t length) {
			this->create(length);
			utils::cudaSafeCall(cudaMemcpy((void*)this->_data, (const void*)hostData, length * sizeof(T), cudaMemcpyHostToDevice), "DeviceArray1D::upload, cudaMemcpy");
			utils::cudaSafeCall(cudaDeviceSynchronize(), "DeviceArray1D::upload, cudaDeviceSynchronize");
		}

		template <class T> void DeviceArray1D<T>::download(T* hostData) const {
			if (this->_data) {
				utils::cudaSafeCall(cudaMemcpy((void*)hostData, (const void*)this->_data, this->_length * sizeof(T), cudaMemcpyDeviceToHost), "DeviceArray1D::download, cudaMemcpy");
				utils::cudaSafeCall(cudaDeviceSynchronize(), "DeviceArray1D::download, cudaDeviceSynchronize");
			}
		}

		template <class T> void DeviceArray1D<T>::copyTo(DeviceArray1D<T>& other) const {
			other.create(this->_length);
			utils::cudaSafeCall(cudaMemcpy((void*)other._data, (void*)this->_data, this->_length * sizeof(T), cudaMemcpyDeviceToDevice), "DeviceArray1D::copyTo cudaMemcpy");
			utils::cudaSafeCall(cudaDeviceSynchronize(), "DeviceArray1D::copyTo, cudaDeviceSynchronize");
		}

		template <class T> void DeviceArray1D<T>::copyTo(T* deviceData) const {
			utils::cudaSafeCall(cudaMemcpy((void*)deviceData, (void*)this->_data, this->_length * sizeof(T), cudaMemcpyDeviceToDevice), "DeviceArray1D::copyTo cudaMemcpy");
			utils::cudaSafeCall(cudaDeviceSynchronize(), "DeviceArray1D::copyTo, cudaDeviceSynchronize");
		}

		template <class T> DeviceArray1D<T> DeviceArray1D<T>::clone(void) const {
			DeviceArray1D<T> other;
			this->copyTo(other);
			return other;
		}

		template <class T> void DeviceArray1D<T>::swap(DeviceArray1D<T>& other) {
			std::swap(this->_data, other._data);
			std::swap(this->_referenceCounter, other._referenceCounter);
			std::swap(this->_length, other._length);
		}

		template <class T> void DeviceArray1D<T>::release(void) {
			if (this->_referenceCounter && _InterlockedExchangeAdd((volatile long*)this->_referenceCounter, -1) == 1) {
				delete this->_referenceCounter;
				utils::cudaSafeCall(cudaFree((void*)this->_data), "DeviceArray1D::release, cudaFree");
			}
			this->_data = nullptr;
			this->_referenceCounter = nullptr;
			this->_length = 0;
		}

		template <class T> inline bool DeviceArray1D<T>::empty(void) const {
			return !this->_data;
		}

		template <class T> inline T* DeviceArray1D<T>::data(void) {
			return this->_data;
		}

		template <class T> inline const T* DeviceArray1D<T>::data(void) const {
			return this->_data;
		}

		template <class T> inline int DeviceArray1D<T>::referenceCounter(void) const {
			if (this->_referenceCounter)
				return *this->_referenceCounter;
			else
				return 0;
		}

		template <class T> inline size_t DeviceArray1D<T>::length(void) const {
			return this->_length;
		}

		template <class T> inline DeviceArray1D<T>::operator Data() {
			return Data(this->_data, this->_length);
		}
		template <class T> inline DeviceArray1D<T>::operator ConstData() const {
			return ConstData(this->_data, this->_length);
		}

		template <class T> inline DeviceArray1D<T>::Data::Data(T* data, size_t length) :
			data(data), length(length) {}

		template <class T> inline T& DeviceArray1D<T>::Data::operator[](size_t pos) {
			return this->data[pos];
		}

		template <class T> inline const T& DeviceArray1D<T>::Data::operator[](size_t pos) const {
			return this->data[pos];
		}

		template <class T> inline T& DeviceArray1D<T>::Data::operator()(size_t pos) {
			return this->data[pos];
		}

		template <class T> inline const T& DeviceArray1D<T>::Data::operator()(size_t pos) const {
			return this->data[pos];
		}

		template <class T> inline DeviceArray1D<T>::ConstData::ConstData(const T* data, size_t length) :
			data(data), length(length) {}

		template <class T> inline const T& DeviceArray1D<T>::ConstData::operator[](size_t pos) const {
			return this->data[pos];
		}

		template <class T> inline const T& DeviceArray1D<T>::ConstData::operator()(size_t pos) const {
			return this->data[pos];
		}

		//Implementation of DeviceArray2D
		template <class T> DeviceArray2D<T>::DeviceArray2D(void) :
			_data(nullptr),
			_pitch(0),
			_referenceCounter(nullptr),
			_rows(0),
			_cols(0)
		{}

		template <class T> DeviceArray2D<T>::DeviceArray2D(size_t rows, size_t cols) :
			_data(nullptr),
			_pitch(0),
			_referenceCounter(nullptr),
			_rows(0),
			_cols(0)
		{
			this->create(rows, cols);
		}

		template <class T> DeviceArray2D<T>::DeviceArray2D(size_t rows, size_t cols, T* deviceData, size_t pitch) :
			_data(deviceData),
			_pitch(pitch),
			_referenceCounter(nullptr),
			_rows(rows),
			_cols(cols)
		{}

		template <class T> DeviceArray2D<T>::DeviceArray2D(const DeviceArray2D<T>& other) :
			_data(other._data),
			_pitch(other._pitch),
			_referenceCounter(other._referenceCounter),
			_rows(other._rows),
			_cols(other._cols)
		{
			if (this->_referenceCounter)
				_InterlockedExchangeAdd((volatile long*)this->_referenceCounter, 1);
		}

		template <class T> DeviceArray2D<T>::~DeviceArray2D(void) {
			this->release();
		}

		template <class T> DeviceArray2D<T>& DeviceArray2D<T>::operator=(const DeviceArray2D<T>& other) {
			if (this == &other) return *this;
			if (other._referenceCounter)
				_InterlockedExchangeAdd((volatile long*)other._referenceCounter, 1);
			this->release();
			this->_data = other._data;
			this->_pitch = other._pitch;
			this->_referenceCounter = other._referenceCounter;
			this->_rows = other._rows;
			this->_cols = other._cols;
			return *this;
		}

		template <class T> void DeviceArray2D<T>::create(size_t rows, size_t cols) {
			if (this->_rows == rows && this->_cols == cols)
				return;
			this->release();
			if (rows > 0 && cols > 0) {
				this->_rows = rows;
				this->_cols = cols;
				utils::cudaSafeCall(cudaMallocPitch((void**)&this->_data, &this->_pitch, this->_cols * sizeof(T), this->_rows), "DeviceArray2D::create, cudaMallocPitch");
				this->_referenceCounter = new int(1);
			}
		}

		template <class T> void DeviceArray2D<T>::upload(const T* hostData, size_t rows, size_t cols) {
			this->create(rows, cols);
			utils::cudaSafeCall(cudaMemcpy2D((void*)this->_data, this->_pitch, (const void*)hostData, cols * sizeof(T), cols * sizeof(T), rows, cudaMemcpyHostToDevice), "DeviceArray2D::upload, cudaMemcpy2D");
			utils::cudaSafeCall(cudaDeviceSynchronize(), "DeviceArray2D::upload, cudaDeviceSynchronize");
		}

		template <class T> void DeviceArray2D<T>::download(T* hostData) const {
			if (this->_data) {
				if (this->_referenceCounter == nullptr) {
					std::cout << "no reference counter" << std::endl;
				}
				utils::cudaSafeCall(cudaMemcpy2D((void*)hostData, this->_cols * sizeof(T), (const void*)this->_data, this->_pitch, this->_cols * sizeof(T), this->_rows, cudaMemcpyDeviceToHost), "DeviceArray2D::download, cudaMemcpy2D");
				utils::cudaSafeCall(cudaDeviceSynchronize(), "DeviceArray2D::download, cudaDeviceSynchronize");
			}
		}

		template <class T> void DeviceArray2D<T>::copyTo(DeviceArray2D<T>& other) const {
			other.create(this->_rows, this->_cols);
			utils::cudaSafeCall(cudaMemcpy2D((void*)other._data, other._pitch, (void*)this->_data, this->_pitch, this->_cols * sizeof(T), this->_rows, cudaMemcpyDeviceToDevice), "DeviceArray2D::copyTo cudaMemcpy2D");
			utils::cudaSafeCall(cudaDeviceSynchronize(), "DeviceArray2D::copyTo, cudaDeviceSynchronize");
		}

		template <class T> void DeviceArray2D<T>::copyTo(T* deviceData, size_t pitch) const {
			utils::cudaSafeCall(cudaMemcpy2D((void*)deviceData, pitch, (void*)this->_data, this->_pitch, this->_cols * sizeof(T), this->_rows, cudaMemcpyDeviceToDevice), "DeviceArray2D::copyTo cudaMemcpy2D");
			utils::cudaSafeCall(cudaDeviceSynchronize(), "DeviceArray2D::copyTo, cudaDeviceSynchronize");
		}

		template <class T> DeviceArray2D<T> DeviceArray2D<T>::clone(void) const {
			DeviceArray2D<T> other;
			this->copyTo(other);
			return other;
		}

		template <class T> void DeviceArray2D<T>::swap(DeviceArray2D<T>& other) {
			std::swap(this->_data, other._data);
			std::swap(this->_pitch, other._pitch);
			std::swap(this->_referenceCounter, other._referenceCounter);
			std::swap(this->_rows, other._rows);
			std::swap(this->_cols, other._cols);
		}

		template <class T> void DeviceArray2D<T>::release(void) {
			if (this->_referenceCounter && _InterlockedExchangeAdd((volatile long*)this->_referenceCounter, -1) == 1) {
				delete this->_referenceCounter;
				utils::cudaSafeCall(cudaFree((void*)this->_data), "DeviceArray2D::release, cudaFree");
			}
			this->_data = nullptr;
			this->_pitch = 0;
			this->_referenceCounter = nullptr;
			this->_rows = 0;
			this->_cols = 0;
		}

		template <class T> inline bool DeviceArray2D<T>::empty(void) const {
			return !this->_data;
		}

		template <class T> inline T* DeviceArray2D<T>::data(void) {
			return this->_data;
		}

		template <class T> inline const T* DeviceArray2D<T>::data(void) const {
			return this->_data;
		}

		template <class T> inline size_t DeviceArray2D<T>::pitch(void) const {
			return this->_pitch;
		}

		template <class T> inline int DeviceArray2D<T>::referenceCounter(void) const {
			if (this->_referenceCounter)
				return *this->_referenceCounter;
			else
				return 0;
		}

		template <class T> inline size_t DeviceArray2D<T>::rows(void) const {
			return this->_rows;
		}

		template <class T> inline size_t DeviceArray2D<T>::cols(void) const {
			return this->_cols;
		}

		template <class T> inline DeviceArray2D<T>::operator Data() {
			return Data(this->_data, this->_pitch, this->_rows, this->_cols);
		}
		template <class T> inline DeviceArray2D<T>::operator ConstData() const {
			return ConstData(this->_data, this->_pitch, this->_rows, this->_cols);
		}

		template <class T> inline DeviceArray2D<T>::Data::Data(T* data, size_t pitch, size_t rows, size_t cols) :
			data(data), pitch(pitch), rows(rows), cols(cols) {}

		template <class T> inline T* DeviceArray2D<T>::Data::operator[](size_t row) {
			return (T*)((char*)this->data + this->pitch * row);
		}

		template <class T> inline const T* DeviceArray2D<T>::Data::operator[](size_t row) const {
			return (const T*)((const char*)this->data + this->pitch * row);
		}

		template <class T> inline T& DeviceArray2D<T>::Data::operator()(size_t row, size_t col) {
			return (*this)[row][col];
		}

		template <class T> inline const T& DeviceArray2D<T>::Data::operator()(size_t row, size_t col) const {
			return (*this)[row][col];
		}

		template <class T> inline DeviceArray2D<T>::ConstData::ConstData(const T* data, size_t pitch, size_t rows, size_t cols) :
			data(data), pitch(pitch), rows(rows), cols(cols) {}

		template <class T> inline const T* DeviceArray2D<T>::ConstData::operator[](size_t row) const {
			return (const T*)((const char*)this->data + this->pitch * row);
		}

		template <class T> inline const T& DeviceArray2D<T>::ConstData::operator()(size_t row, size_t col) const {
			return (*this)[row][col];
		}

	}

}
/// @endcond

#endif /* jjyou_cuda_DeviceArray_hpp */