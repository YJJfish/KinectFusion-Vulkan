/***********************************************************************
 * @file	transform.hpp
 * @author	jjyou
 * @date	2024-2-3
 * @brief	This file implements some matrix transform functions.
***********************************************************************/

#ifndef jjyou_glsl_transform_hpp
#define jjyou_glsl_transform_hpp

#if !defined(JJYOU_USE_OPENGL) && !defined(JJYOU_USE_VULKAN)
static_assert(0, "Please specify the API you use. E.g. define JJYOU_USE_OPENGL or JJYOU_USE_VULKAN");
#endif

#include <cmath>
#include <numbers>

namespace jjyou {

	namespace glsl {

		/** @brief	Convert a rotation vector to a rotation matrix.
		  * @param	vec		3-D rotation vector.
		  * @return	3x3 rotation matrix.
		  */
		template <class T> inline mat<T, 3, 3> rodrigues(const vec<T, 3>& v) {
			const mat<T, 3, 3> I3x3 = identity<T, 3>();
			T theta = norm(v);
			if (theta == T())
				return I3x3;
			vec<T, 3> r = v / theta;
			T cos_theta = std::cos(theta);
			T sin_theta = std::sin(theta);
			mat<T, 3, 3> r_outer = outer(r, r);
			mat<T, 3, 3> r_cross = cross(r);
			return cos_theta * I3x3 + (static_cast<T>(1.0) - cos_theta) * r_outer + sin_theta * r_cross;
		}

		/** @brief	Convert a rotation matrix to a rotation vector.
		  * @param	vec		3x3 rotation matrix.
		  * @return	3-D rotation vector.
		  */
		template <class T> inline vec<T, 3> rodrigues(const mat<T, 3, 3>& m) {
			T cos_theta = (trace(m) - static_cast<T>(1.0)) / static_cast<T>(2.0);
			if (cos_theta == static_cast<T>(1.0))
				return zeros<T, 3>();
			else if (cos_theta == static_cast<T>(-1.0)) {
				mat<T, 3, 3> uuT = (m + identity<T, 3>()) / static_cast<T>(2.0);
				vec<T, 3> v = uuT[0];
				if (squaredNorm(v) == static_cast<T>(0.0))
					v = uuT[1];
				if (squaredNorm(v) == static_cast<T>(0.0))
					v = uuT[2];
				v = normalized(v) * std::numbers::pi_v<T>;
				if ((v.x == static_cast<T>(0.0) && v.y == static_cast<T>(0.0) && v.z < static_cast<T>(0.0)) ||
					(v.x == static_cast<T>(0.0) && v.y < static_cast<T>(0.0)) ||
					(v.x < static_cast<T>(0.0)))
					v = -v;
				return v;
			}
			T theta = std::acos(cos_theta);
			mat<T, 3, 3> A = (m - transpose(m)) / static_cast<T>(2.0);
			vec<T, 3> v(A[1][2], A[2][0], A[0][1]);
			normalize(v);
			v *= theta;
			return v;
		}

		/** @brief	Get camera transform matrix (transform points in world coordinate system to points in camera coordinate system)
		  *			according to position, target, and up vectors.
		  * @param	position	Camera position in world coordinate system.
		  * @param	target		The target position the camera is looking at.
		  * @param	up			Camera's up direction in world coordinate system.
		  * @return	4x4 transform matrix.
		  */
		template <class T> mat<T, 4, 4> lookAt(const vec<T, 3>& position, const vec<T, 3>& target, const vec<T, 3>& up) {
			vec<T, 3> z = normalized(target - position);
			vec<T, 3> x = normalized(cross(z, up));
			vec<T, 3> y = cross(z, x);
			mat<T, 4, 4> res{};
			res[0][0] = x.x;
			res[1][0] = x.y;
			res[2][0] = x.z;
			res[0][1] = y.x;
			res[1][1] = y.y;
			res[2][1] = y.z;
			res[0][2] = z.x;
			res[1][2] = z.y;
			res[2][2] = z.z;
			res[3] = vec<T, 4>(-dot(position, x), -dot(position, y), -dot(position, z), static_cast<T>(1.0));
			return res;
		}

		/** @brief	Get perspective camera projection matrix.
		  * @param	yFov		Field of view in y direction.
		  * @param	aspectRatio	Aspect ratio (width / height).
		  * @param	zNear		Near clipping plane.
		  * @param	zFar		Far clipping plane.
		  * @return	4x4 perspective projection matrix.
		  */
		template <class T> mat<T, 4, 4> perspective(T yFov, T aspectRatio, T zNear, T zFar) {
#if defined(JJYOU_USE_OPENGL)
			T tanHalfYFov = std::tan(yFov / static_cast<T>(2.0));
			T tanHalfXFov = aspectRatio * tanHalfYFov;
			mat<T, 4, 4> res{};
			res[0][0] = static_cast<T>(1.0) / tanHalfXFov;
			res[1][1] = -static_cast<T>(1.0) / tanHalfYFov;
			res[2][2] = (zFar + zNear) / (zFar - zNear);
			res[2][3] = static_cast<T>(1.0);
			res[3][2] = -(static_cast<T>(2.0) * zFar * zNear) / (zFar - zNear);
			return res;
#elif defined(JJYOU_USE_VULKAN)
			T tanHalfYFov = std::tan(yFov / static_cast<T>(2.0));
			T tanHalfXFov = aspectRatio * tanHalfYFov;
			mat<T, 4, 4> res{};
			res[0][0] = static_cast<T>(1.0) / tanHalfXFov;
			res[1][1] = static_cast<T>(1.0) / tanHalfYFov;
			res[2][2] = zFar / (zFar - zNear);
			res[2][3] = static_cast<T>(1.0);
			res[3][2] = -(zFar * zNear) / (zFar - zNear);
			return res;
#endif
		}

		/** @brief	Get orthographic camera projection matrix.
		  * @param	yFov		Field of view in y direction.
		  * @param	aspectRatio	Aspect ratio (width / height).
		  * @param	zNear		Near clipping plane.
		  * @param	zFar		Far clipping plane.
		  * @return	4x4 perspective projection matrix.
		  */
		template <class T> mat<T, 4, 4> orthographic(T width, T height, T zNear, T zFar) {
#if defined(JJYOU_USE_OPENGL)
			mat<T, 4, 4> res;
			res[0][0] = static_cast<T>(2.0) / width;
			res[1][1] = -static_cast<T>(2.0) / height;
			res[2][2] = static_cast<T>(2.0) / (zFar - zNear);
			res[3][2] = -(zFar + zNear) / (zFar - zNear);
			res[3][3] = static_cast<T>(1.0);
			return res;
#elif defined(JJYOU_USE_VULKAN)
			mat<T, 4, 4> res;
			res[0][0] = static_cast<T>(2.0) / width;
			res[1][1] = static_cast<T>(2.0) / height;
			res[2][2] = static_cast<T>(1.0) / (zFar - zNear);
			res[3][2] = -zNear / (zFar - zNear);
			res[3][3] = static_cast<T>(1.0);
			return res;
#endif
		}

		/** @brief	Get pinhole camera projection matrix (fx=fy=f, cx=width/2, cy=height/2).
		  * @param	yFov		Field of view in y direction.
		  * @param	width		Image width.
		  * @param	height		Image height.
		  * @return	3x3 pinhole projection matrix.
		  */
		template <class T, class U> mat<T, 3, 3> pinhole(T yFov, U width, U height) {
			mat<T, 3, 3> res;
			T tanHalfYFov = std::tan(yFov / static_cast<T>(2.0));
			T f = static_cast<T>(1.0) / tanHalfYFov * static_cast<T>(height) / static_cast<T>(2.0);
			res[0][0] = f; // fx
			res[1][1] = f; // fy
			res[2][0] = static_cast<T>(width) / static_cast<T>(2.0); // cx
			res[2][1] = static_cast<T>(height) / static_cast<T>(2.0); // cy
			res[2][2] = static_cast<T>(1.0);
			return res;
		}

	}

}

#endif /* jjyou_glsl_transform_hpp */