#ifndef GL_MATH_INCLUDED
#define GL_MATH_INCLUDED

#include <cmath>
#include "./Exception.hpp"

typedef unsigned int uint;

#define _va_args(...) __VA_ARGS__

#define _MakeVectorOperator(op, numelements) \
void operator op (T val) { for (int i = 0; i < numelements; i++) this->operator[](i) op val; } \
template <typename S> void operator op (S vec) { for (int i = 0; i < numelements; i++) this->operator[](i) op (T)vec[i]; }

#define _MakeVectorOperators(numelements) \
_MakeVectorOperator(+=, numelements) \
_MakeVectorOperator(-=, numelements) \
_MakeVectorOperator(*=, numelements) \
_MakeVectorOperator(/=, numelements) \
_MakeVectorOperator(%=, numelements) \
_MakeVectorOperator(&=, numelements) \
_MakeVectorOperator(|=, numelements) \
_MakeVectorOperator(^=, numelements) \
_MakeVectorOperator(>>=, numelements) \
_MakeVectorOperator(<<=, numelements)

#define _MakeSwizzleBaseClass(swizzlebaseclassname, numelements, members, constargs, initmembers, initmembers2, bracketret) \
namespace GL { \
	\
	template <typename T> \
	struct swizzlebaseclassname { \
		\
		T members; \
		\
		swizzlebaseclassname(constargs) : initmembers { } \
		template <typename S> swizzlebaseclassname(S& vec) : initmembers2 { } \
		\
		template <typename S> void operator = (S vec) { for (int i = 0u; i < numelements; i++) this->operator[](i) = (T)vec[i]; }\
		\
		_MakeVectorOperators(numelements) \
		\
		T& operator [] (uint idx) { return bracketret; }\
		\
	}; \
	\
}

_MakeSwizzleBaseClass(SwizzleBase2D, 2, _va_args(&x, &y), _va_args(T& x, T& y), _va_args(x(x), y(y)), _va_args(x(vec.x), y(vec.y)), (idx == 0u) ? x : y)
_MakeSwizzleBaseClass(SwizzleBase3D, 3, _va_args(&x, &y, &z), _va_args(T& x, T& y, T& z), _va_args(x(x), y(y), z(z)), _va_args(x(vec.x), y(vec.y), z(vec.z)), (idx == 0u) ? x : ((idx == 1u) ? y : z))
_MakeSwizzleBaseClass(SwizzleBase4D, 4, _va_args(&x, &y, &z, &w), _va_args(T& x, T& y, T& z, T& w), _va_args(x(x), y(y), z(z), w(w)), _va_args(x(vec.x), y(vec.y), z(vec.z), w(vec.w)), (idx == 0u) ? x : ((idx == 1u) ? y : ((idx == 2u) ? z : w)))

#define _MakeVectorBinaryOperator(rtype, op, Vectorclassname, swizzleclassname, numelements) \
template <typename T> Vectorclassname<rtype> operator op (Vectorclassname<T> lh, Vectorclassname<T> rh) { Vectorclassname<rtype> ret; for (int i = 0; i < numelements; i++) ret[i] = lh[i] op rh[i]; return ret; } \
template <typename T> Vectorclassname<rtype> operator op (swizzleclassname<T> lh, Vectorclassname<T> rh) { Vectorclassname<rtype> ret; for (int i = 0; i < numelements; i++) ret[i] = lh[i] op rh[i]; return ret; } \
template <typename T> Vectorclassname<rtype> operator op (Vectorclassname<T> lh, swizzleclassname<T> rh) { Vectorclassname<rtype> ret; for (int i = 0; i < numelements; i++) ret[i] = lh[i] op rh[i]; return ret; } \
template <typename T> Vectorclassname<rtype> operator op (swizzleclassname<T> lh, swizzleclassname<T> rh) { Vectorclassname<rtype> ret; for (int i = 0; i < numelements; i++) ret[i] = lh[i] op rh[i]; return ret; } \
template <typename T> Vectorclassname<rtype> operator op (T lh, Vectorclassname<T> rh) { Vectorclassname<rtype> ret; for (int i = 0; i < numelements; i++) ret[i] = lh op rh[i]; return ret; } \
template <typename T> Vectorclassname<rtype> operator op (Vectorclassname<T> lh, T rh) { Vectorclassname<rtype> ret; for (int i = 0; i < numelements; i++) ret[i] = lh[i] op rh; return ret; } \
template <typename T> Vectorclassname<rtype> operator op (T lh, swizzleclassname<T> rh) { Vectorclassname<rtype> ret; for (int i = 0; i < numelements; i++) ret[i] = lh op rh[i]; return ret; } \
template <typename T> Vectorclassname<rtype> operator op (swizzleclassname<T> lh, T rh) { Vectorclassname<rtype> ret; for (int i = 0; i < numelements; i++) ret[i] = lh[i] op rh; return ret; }

#define _MakeVectorClass(Vectorclassname, swizzleclassname, numelements, structmembers, constargs, constbody, swizzleoperators) \
namespace GL { \
	\
	template <typename T> \
	struct Vectorclassname { \
		\
		union { \
			T data[numelements]; \
			struct { T structmembers; }; \
		}; \
		\
		Vectorclassname() { for (int i = 0; i < numelements; i++) data[i] = (T)0; } \
		Vectorclassname(T val) { for (int i = 0; i < numelements; i++) data[i] = val; } \
		Vectorclassname(constargs) : constbody { } \
		template <typename S> Vectorclassname(S vec) { for (int i = 0; i < numelements; i++) data[i] = (T)(vec[i]); } \
		\
		void operator = (T val) { for (int i = 0; i < numelements; i++) data[i] = val; } \
		template <typename S> void operator = (S vec) { for (int i = 0; i < numelements; i++) data[i] = (T)(vec[i]); } \
		\
		T& operator [] (uint idx) { if (idx >= numelements) return data[numelements - 1]; return data[idx]; } \
		\
		_MakeVectorOperators(numelements) \
		\
		swizzleoperators \
		\
	}; \
	\
	_MakeVectorBinaryOperator(T, +, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, -, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, *, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, /, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, %, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, &&, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, ||, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, &, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, |, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, ^, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, >>, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(T, <<, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(bool, ==, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(bool, !=, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(bool, >=, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(bool, <=, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(bool, >, Vectorclassname, swizzleclassname, numelements) \
	_MakeVectorBinaryOperator(bool, <, Vectorclassname, swizzleclassname, numelements) \
	template <typename T> Vectorclassname<T> operator ~ (Vectorclassname<T> vec) { Vectorclassname<T> ret; for (int i = 0; i < numelements; i++) ret[i] = ~vec[i]; return ret; } \
	\
}

#define _MakeSwizzle(returntype, args, econdition, returnargs) \
returntype operator () (args) { if econdition throw Exception("Invalid Vector swizzle indices."); return returntype(returnargs); }

#define _MakeSwizzle2D() \
_MakeSwizzle(SwizzleBase2D<T>, _va_args(uint ix, uint iy), (ix >= 2 || iy >= 2), _va_args(this->operator[](ix), this->operator[](iy)))

#define _MakeSwizzle3D() \
_MakeSwizzle(SwizzleBase2D<T>, _va_args(uint ix, uint iy), (ix >= 3 || iy >= 3), _va_args(this->operator[](ix), this->operator[](iy))) \
_MakeSwizzle(SwizzleBase3D<T>, _va_args(uint ix, uint iy, uint iz), (ix >= 3 || iy >= 3 || iz >= 3), _va_args(this->operator[](ix), this->operator[](iy), this->operator[](iz)))

#define _MakeSwizzle4D() \
_MakeSwizzle(SwizzleBase2D<T>, _va_args(uint ix, uint iy), (ix >= 4 || iy >= 4), _va_args(this->operator[](ix), this->operator[](iy))) \
_MakeSwizzle(SwizzleBase3D<T>, _va_args(uint ix, uint iy, uint iz), (ix >= 4 || iy >= 4 || iz >= 4), _va_args(this->operator[](ix), this->operator[](iy), this->operator[](iz))) \
_MakeSwizzle(SwizzleBase4D<T>, _va_args(uint ix, uint iy, uint iz, uint iw), (ix >= 4 || iy >= 4 || iz >= 4 || iw >= 4), _va_args(this->operator[](ix), this->operator[](iy), this->operator[](iz), this->operator[](iw)))

_MakeVectorClass(Vector2D, SwizzleBase2D, 2, _va_args(x, y), _va_args(T x, T y), _va_args(x(x), y(y)), _MakeSwizzle2D())
_MakeVectorClass(Vector3D, SwizzleBase3D, 3, _va_args(x, y, z), _va_args(T x, T y, T z), _va_args(x(x), y(y), z(z)), _MakeSwizzle3D())
_MakeVectorClass(Vector4D, SwizzleBase4D, 4, _va_args(x, y, z, w), _va_args(T x, T y, T z, T w), _va_args(x(x), y(y), z(z), w(w)), _MakeSwizzle4D())

namespace GL {

	typedef Vector2D<float> vec2;
	typedef Vector2D<double> dvec2;
	typedef Vector2D<int> ivec2;
	typedef Vector2D<uint> uvec2;
	typedef Vector2D<bool> bvec2;

	typedef Vector3D<float> vec3;
	typedef Vector3D<double> dvec3;
	typedef Vector3D<int> ivec3;
	typedef Vector3D<uint> uvec3;
	typedef Vector3D<bool> bvec3;

	typedef Vector4D<float> vec4;
	typedef Vector4D<double> dvec4;
	typedef Vector4D<int> ivec4;
	typedef Vector4D<uint> uvec4;
	typedef Vector4D<bool> bvec4;

	template <typename T>
	T radians(T degrees) { return degrees * (T)0.01745329251; }
	template <typename T>
	T degrees(T radians) { return radians * (T)57.2957795131; }
	template <typename T>
	T sign(T x) { return (x > (T)0) ? (T)1 : (x < (T)0) ? (T)-1 : (T)0; }
	template <typename T>
	T fract(T x) { return x - std::floor(x); }
	template <typename T>
	T mod(T x, T y) { return x - y * std::floor(x / y); }
	template <typename T>
	T modf(T x, T& y) {
		T m = mod(x, y);
		y = floor(x / y);
		return m;
	}
	template <typename T>
	T min(T x, T y) { return (x < y) ? x : y; }
	template <typename T>
	T max(T x, T y) { return (x > y) ? x : y; }
	template <typename T>
	T clamp(T x, T minVal, T maxVal) { return min(max(x, minVal), maxVal); }
	template <typename T>
	T mix(T x, T y, T a) { return x * ((T)1 - a) + y * a; }
	template <typename T>
	T step(T edge, T x) { return (x < edge) ? (T)0 : (T)1; }
	template <typename T>
	T smoothstep(T edge0, T edge1, T x) {
		T t = clamp((x - edge0) / (edge1 - edge0), (T)0, (T)1);
		return t * t * ((T)3 - (T)2 * t);
	}

}

#define _MakeVectorUpscaleFunction(vectorclassname, dim) \
namespace GL { \
	\
	template <typename Vector> \
	Vector upscale(vectorclassname vec) { \
		\
		Vector ret; \
		for (int i = 0; i < dim; i++) ret[i] = vec[i]; \
		return ret; \
		\
	} \
	\
}

#define _MakeVectorUpscaleFunctions(dim) \
_MakeVectorUpscaleFunction(vec ## dim, dim) _MakeVectorUpscaleFunction(dvec ## dim, dim) _MakeVectorUpscaleFunction(ivec ## dim, dim) _MakeVectorUpscaleFunction(uvec ## dim, dim) _MakeVectorUpscaleFunction(bvec ## dim, dim)

#define _MakeUnaryFunction(funcname, classname, numelements, prefix) \
namespace GL { template <typename T> classname funcname(classname vec) { for (int i = 0; i < numelements; i++) vec[i] = prefix funcname(vec[i]); return vec; } }

#define _MakeBinaryFunction(funcname, retclassname, leftclassname, rightclassname, numelements, leftindex, rightindex, prefix) \
namespace GL { \
	\
	template <typename T> \
	retclassname funcname(leftclassname lh, rightclassname rh) { \
		\
		retclassname ret; \
		for (int i = 0; i < numelements; i++) ret[i] = prefix funcname(lh leftindex, rh rightindex); \
		return ret; \
		\
	} \
	\
}

#define _MakeTrinaryFunction(funcname, retclassname, leftclassname, middleclassname, rightclassname, numelements, leftindex, middleindex, rightindex) \
namespace GL { \
	\
	template <typename T> \
	retclassname funcname(leftclassname lh, middleclassname mh, rightclassname rh) { \
		\
		retclassname ret; \
		for (int i = 0; i < numelements; i++) ret[i] = funcname(lh leftindex, mh middleindex, rh rightindex); \
		return ret; \
		\
	} \
	\
}

#define _MakeUnaryFunctions(funcname, prefix) \
_MakeUnaryFunction(funcname, Vector2D<T>, 2, prefix) \
_MakeUnaryFunction(funcname, Vector3D<T>, 3, prefix) \
_MakeUnaryFunction(funcname, Vector4D<T>, 4, prefix)

#define _MakeBinaryFunctions(funcname, reference, prefix) \
_MakeBinaryFunction(funcname, Vector2D<T>, Vector2D<T>, Vector2D<T>reference, 2, [i], [i], prefix) \
_MakeBinaryFunction(funcname, Vector2D<T>, Vector2D<T>, T reference, 2, [i], , prefix) \
_MakeBinaryFunction(funcname, Vector2D<T>, T, Vector2D<T>reference, 2, , [i], prefix) \
_MakeBinaryFunction(funcname, Vector3D<T>, Vector3D<T>, Vector3D<T>reference, 3, [i], [i], prefix) \
_MakeBinaryFunction(funcname, Vector3D<T>, Vector3D<T>, T reference, 3, [i], , prefix) \
_MakeBinaryFunction(funcname, Vector3D<T>, T, Vector3D<T> reference, 3, , [i], prefix) \
_MakeBinaryFunction(funcname, Vector4D<T>, Vector4D<T>, Vector4D<T> reference, 4, [i], [i], prefix) \
_MakeBinaryFunction(funcname, Vector4D<T>, Vector4D<T>, T reference, 4, [i], , prefix) \
_MakeBinaryFunction(funcname, Vector4D<T>, T, Vector4D<T> reference, 4, , [i], prefix)

#define _MakeTrinaryFunctions_oneDimension(funcname, Vectorclassname, scalarclassname, numelements) \
_MakeTrinaryFunction(funcname, Vectorclassname, Vectorclassname, Vectorclassname, Vectorclassname, numelements, [i], [i], [i]) \
_MakeTrinaryFunction(funcname, Vectorclassname, Vectorclassname, Vectorclassname, scalarclassname, numelements, [i], [i],) \
_MakeTrinaryFunction(funcname, Vectorclassname, Vectorclassname, scalarclassname, Vectorclassname, numelements, [i], , [i]) \
_MakeTrinaryFunction(funcname, Vectorclassname, Vectorclassname, scalarclassname, scalarclassname, numelements, [i], ,) \
_MakeTrinaryFunction(funcname, Vectorclassname, scalarclassname, Vectorclassname, Vectorclassname, numelements, , [i], [i]) \
_MakeTrinaryFunction(funcname, Vectorclassname, scalarclassname, Vectorclassname, scalarclassname, numelements, , [i],) \
_MakeTrinaryFunction(funcname, Vectorclassname, scalarclassname, scalarclassname, Vectorclassname, numelements, , , [i])

#define _MakeTrinaryFunctions(funcname) \
_MakeTrinaryFunctions_oneDimension(funcname, Vector2D<T>, T, 2) \
_MakeTrinaryFunctions_oneDimension(funcname, Vector3D<T>, T, 3) \
_MakeTrinaryFunctions_oneDimension(funcname, Vector4D<T>, T, 4)

#define _MakeLengthFunction(classname, numelements) \
namespace GL { template <typename T> T length(classname vec) { T len = (T)0; for (int i = 0; i < numelements; i++) len += vec[i] * vec[i]; return std::sqrt(len); } }

#define _MakeLengthFunctions() \
_MakeLengthFunction(Vector2D<T>, 2) \
_MakeLengthFunction(Vector3D<T>, 3) \
_MakeLengthFunction(Vector4D<T>, 4)

#define _MakeDistanceFunction(classname) \
namespace GL { template <typename T> T distance(classname p0, classname p1) { return length(p0 - p1); } }

#define _MakeDistanceFunctions() \
_MakeDistanceFunction(Vector2D<T>) \
_MakeDistanceFunction(Vector3D<T>) \
_MakeDistanceFunction(Vector4D<T>)

#define _MakeDotFunction(classname, numelements) \
namespace GL { template <typename T> T dot(classname p0, classname p1) { T d = (T)0; for (int i = 0; i < numelements; i++) d += p0[i] * p1[i]; return d; } }

#define _MakeDotFunctions() \
_MakeDotFunction(Vector2D<T>, 2) \
_MakeDotFunction(Vector3D<T>, 3) \
_MakeDotFunction(Vector4D<T>, 4)

#define _MakeNormalizeFunction(classname) \
namespace GL { template <typename T> classname normalize(classname vec) { T l = length(vec); return (l == (T)0) ? classname() : vec / l; } }

#define _MakeNormalizeFunctions() \
_MakeNormalizeFunction(Vector2D<T>) \
_MakeNormalizeFunction(Vector3D<T>) \
_MakeNormalizeFunction(Vector4D<T>)

#define _MakeReflectFunction(classname) \
namespace GL { template <typename T> classname reflect(classname I, classname N) { return I - N * dot(N, I) * (T)2; } }

#define _MakeReflectFunctions() \
_MakeReflectFunction(Vector2D<T>) \
_MakeReflectFunction(Vector3D<T>) \
_MakeReflectFunction(Vector4D<T>)

#define _MakeBoolFunction(funcname, classname, numelements, op) \
namespace GL { template <typename T> T funcname(classname vec) { T ret = vec[0]; for (int i = 1; i < numelements; i++) ret = ret op vec[i]; return ret; } }

#define _MakeBoolFunctions(funcname, op) \
_MakeBoolFunction(funcname, Vector2D<T>, 2, op) \
_MakeBoolFunction(funcname, Vector3D<T>, 3, op) \
_MakeBoolFunction(funcname, Vector4D<T>, 4, op)

#define _MakeAnyAndAllFunctions() \
_MakeBoolFunctions(any, ||) \
_MakeBoolFunctions(all, &&)

#define _MakeProjFunction(dim) \
namespace GL { \
	\
	template <typename T> \
	Vector ## dim ## D<T> proj (Vector ## dim ## D<T> u, Vector ## dim ## D<T> v) { return v * dot(u, v) / dot(v, v); } \
	\
}

#define _MakeProjFunctions() \
_MakeProjFunction(2) \
_MakeProjFunction(3) \
_MakeProjFunction(4)

_MakeVectorUpscaleFunctions(2)
_MakeVectorUpscaleFunctions(3)
_MakeVectorUpscaleFunctions(4)

_MakeUnaryFunctions(radians,)
_MakeUnaryFunctions(degrees,)
_MakeUnaryFunctions(sin, std::)
_MakeUnaryFunctions(cos, std::)
_MakeUnaryFunctions(tan, std::)
_MakeUnaryFunctions(asin, std::)
_MakeUnaryFunctions(acos, std::)
_MakeUnaryFunctions(atan, std::)
_MakeUnaryFunctions(sinh, std::)
_MakeUnaryFunctions(cosh, std::)
_MakeUnaryFunctions(tanh, std::)
_MakeUnaryFunctions(asinh, std::)
_MakeUnaryFunctions(acosh, std::)
_MakeUnaryFunctions(atanh, std::)
_MakeUnaryFunctions(exp, std::)
_MakeUnaryFunctions(log, std::)
_MakeUnaryFunctions(exp2, std::)
_MakeUnaryFunctions(log2, std::)
_MakeUnaryFunctions(sqrt, std::)
_MakeUnaryFunctions(abs, std::)
_MakeUnaryFunctions(sign,)
_MakeUnaryFunctions(floor, std::)
_MakeUnaryFunctions(ceil, std::)
_MakeUnaryFunctions(fract,)
_MakeUnaryFunctions(isnan, std::)
_MakeUnaryFunctions(isinf, std::)

_MakeBinaryFunctions(pow, , std::)
_MakeBinaryFunctions(mod, ,)
_MakeBinaryFunctions(modf, &,)
_MakeBinaryFunctions(min, ,)
_MakeBinaryFunctions(max, ,)
_MakeBinaryFunctions(step, ,)

_MakeTrinaryFunctions(clamp)
_MakeTrinaryFunctions(mix)
_MakeTrinaryFunctions(smoothstep)

_MakeLengthFunctions()
_MakeDistanceFunctions()
_MakeDotFunctions()
_MakeNormalizeFunctions()
_MakeReflectFunctions()
_MakeAnyAndAllFunctions()

_MakeProjFunctions()

namespace GL {
	
	template <typename T>
	Vector3D<T> cross(Vector3D<T> x, Vector3D<T> y) { return Vector3D<T>(x.y * y.z - x.z * y.y, x.z * y.x - x.x * y.z, x.x * y.y - x.y * y.x); }

	template <typename T>
	Vector4D<T> toVec4(Vector3D<T> vec) { return Vector4D<T>(vec.x, vec.y, vec.z, (T)1); }

}

#define _MakeMatrixOperator(op, numrows, numcols) \
void operator op ## = (T val) { for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) cols[col][row] op ## = val; }

#define _MakeMatrixOperator2(op, numrows, numcols) \
_MakeMatrixOperator(op, numrows, numcols) \
void operator op ## = (Matrix ## numrows ## x ## numcols<T> mat) { for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) cols[col][row] op ## = (T)mat[col][row]; }

#define _MakeMatrixOperator3(op, numrows, numcols) \
template <typename T> Matrix ## numrows ## x ## numcols<T> operator op (Matrix ## numrows ## x ## numcols<T> mat, T val) { for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) mat[col][row] = mat[col][row] op val; return mat; } \
template <typename T> Matrix ## numrows ## x ## numcols<T> operator op (T val, Matrix ## numrows ## x ## numcols<T> mat) { for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) mat[col][row] = val op mat[col][row]; return mat; } \

#define _MakeMatrixOperator4(op, numrows, numcols) \
_MakeMatrixOperator3(op, numrows, numcols) \
template <typename T> Matrix ## numrows ## x ## numcols<T> operator op (Matrix ## numrows ## x ## numcols<T> lh, Matrix ## numrows ## x ## numcols<T> rh) { for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) lh[col][row] = lh[col][row] op rh[col][row]; return lh; }

#define _MakeMatrixClass(numrows, numcols, initializers, initializers2, initializers3, initializers4) \
namespace GL { \
	\
	template <typename T> \
	struct Matrix ## numrows ## x ## numcols { \
		\
		Vector ## numrows ## D<T> cols[numcols]; \
		\
		Matrix ## numrows ## x ## numcols() { for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) cols[col][row] = (row == col) ? (T)1 : (T)0; } \
		Matrix ## numrows ## x ## numcols(T val) { for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) cols[col][row] = (row == col) ? val : (T)0; } \
		Matrix ## numrows ## x ## numcols(initializers) : cols{ initializers2 } { } \
		Matrix ## numrows ## x ## numcols(initializers3) : cols{ initializers4 } { } \
		template <typename S> Matrix ## numrows ## x ## numcols(S mat) { for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) cols[col][row] = (T)mat[col][row]; } \
		\
		Vector ## numrows ## D<T>& operator [] (uint idx) { if (idx >= numcols) return cols[numcols - 1]; return cols[idx]; } \
		\
		void operator = (T val) { for (int row = 0; row < numrows; row++) for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) cols[col][row] = (row == col) ? val : (T)0; } \
		template <typename S> void operator = (S mat) { for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) cols[col][row] = (T)mat[col][row]; } \
		\
		_MakeMatrixOperator2(+, numrows, numcols) \
		_MakeMatrixOperator2(-, numrows, numcols) \
		_MakeMatrixOperator(*, numrows, numcols) \
		_MakeMatrixOperator2(/, numrows, numcols) \
		\
	}; \
	\
	_MakeMatrixOperator4(+, numrows, numcols) \
	_MakeMatrixOperator4(-, numrows, numcols) \
	_MakeMatrixOperator4(/, numrows, numcols) \
	\
	_MakeMatrixOperator3(*, numrows, numcols) \
	template <typename T> Vector ## numrows ## D<T> operator * (Matrix ## numrows ## x ## numcols<T> mat, Vector ## numcols ## D<T> vec) { \
		\
		Vector ## numrows ## D<T> ret; \
		for (int row = 0; row < numrows; row++) { \
			\
			Vector ## numcols ## D<T> temp; for (int col = 0; col < numcols; col++) temp[col] = mat[col][row]; \
			ret[row] = dot(vec, temp); \
			\
		} \
		return ret; \
		\
	} \
	\
}

_MakeMatrixClass(2, 2, _va_args(Vector2D<T> col0, Vector2D<T> col1), _va_args(col0, col1), _va_args(T col00, T col01, T col10, T col11), _va_args(Vector2D<T>(col00, col01), Vector2D<T>(col10, col11)));
_MakeMatrixClass(2, 3, _va_args(Vector2D<T> col0, Vector2D<T> col1, Vector2D<T> col2), _va_args(col0, col1, col2), _va_args(T col00, T col01, T col10, T col11, T col20, T col21), _va_args(Vector2D<T>(col00, col01), Vector2D<T>(col10, col11), Vector2D<T>(col20, col21)));
_MakeMatrixClass(2, 4, _va_args(Vector2D<T> col0, Vector2D<T> col1, Vector2D<T> col2, Vector2D<T> col3), _va_args(col0, col1, col2, col3), _va_args(T col00, T col01, T col10, T col11, T col20, T col21, T col30, T col31), _va_args(Vector2D<T>(col00, col01), Vector2D<T>(col10, col11), Vector2D<T>(col20, col21), Vector2D<T>(col30, col31)));

_MakeMatrixClass(3, 2, _va_args(Vector3D<T> col0, Vector3D<T> col1), _va_args(col0, col1), _va_args(T col00, T col01, T col02, T col10, T col11, T col12), _va_args(Vector3D<T>(col00, col01, col02), Vector3D<T>(col10, col11, col12)));
_MakeMatrixClass(3, 3, _va_args(Vector3D<T> col0, Vector3D<T> col1, Vector3D<T> col2), _va_args(col0, col1, col2), _va_args(T col00, T col01, T col02, T col10, T col11, T col12, T col20, T col21, T col22), _va_args(Vector3D<T>(col00, col01, col02), Vector3D<T>(col10, col11, col12), Vector3D<T>(col20, col21, col22)));
_MakeMatrixClass(3, 4, _va_args(Vector3D<T> col0, Vector3D<T> col1, Vector3D<T> col2, Vector3D<T> col3), _va_args(col0, col1, col2, col3), _va_args(T col00, T col01, T col02, T col10, T col11, T col12, T col20, T col21, T col22, T col30, T col31, T col32), _va_args(Vector3D<T>(col00, col01, col02), Vector3D<T>(col10, col11, col12), Vector3D<T>(col20, col21, col22), Vector3D<T>(col30, col31, col32)));

_MakeMatrixClass(4, 2, _va_args(Vector4D<T> col0, Vector4D<T> col1), _va_args(col0, col1), _va_args(T col00, T col01, T col02, T col03, T col10, T col11, T col12, T col13), _va_args(Vector4D<T>(col00, col01, col02, col03), Vector4D<T>(col10, col11, col12, col13)));
_MakeMatrixClass(4, 3, _va_args(Vector4D<T> col0, Vector4D<T> col1, Vector4D<T> col2), _va_args(col0, col1, col2), _va_args(T col00, T col01, T col02, T col03, T col10, T col11, T col12, T col13, T col20, T col21, T col22, T col23), _va_args(Vector4D<T>(col00, col01, col02, col03), Vector4D<T>(col10, col11, col12, col13), Vector4D<T>(col20, col21, col22, col23)));
_MakeMatrixClass(4, 4, _va_args(Vector4D<T> col0, Vector4D<T> col1, Vector4D<T> col2, Vector4D<T> col3), _va_args(col0, col1, col2, col3), _va_args(T col00, T col01, T col02, T col03, T col10, T col11, T col12, T col13, T col20, T col21, T col22, T col23, T col30, T col31, T col32, T col33), _va_args(Vector4D<T>(col00, col01, col02, col03), Vector4D<T>(col10, col11, col12, col13), Vector4D<T>(col20, col21, col22, col23), Vector4D<T>(col30, col31, col32, col33)));

namespace GL {

	typedef Matrix2x2<float> mat2;
	typedef Matrix2x2<double> dmat2;
	typedef Matrix2x3<float> mat2x3;
	typedef Matrix2x3<double> dmat2x3;
	typedef Matrix2x4<float> mat2x4;
	typedef Matrix2x4<double> dmat2x4;

	typedef Matrix3x2<float> mat3x2;
	typedef Matrix3x2<double> dmat3x2;
	typedef Matrix3x3<float> mat3;
	typedef Matrix3x3<double> dmat3;
	typedef Matrix3x4<float> mat3x4;
	typedef Matrix3x4<double> dmat3x4;

	typedef Matrix4x2<float> mat4x2;
	typedef Matrix4x2<double> dmat4x2;
	typedef Matrix4x3<float> mat4x3;
	typedef Matrix4x3<double> dmat4x3;
	typedef Matrix4x4<float> mat4;
	typedef Matrix4x4<double> dmat4;

}

#define _MakeMatrixUpscaleFunction(matrixclassname, numrows, numcols) \
namespace GL { \
	\
	template <typename Matrix> \
	Matrix upscale(matrixclassname mat) { \
		\
		Matrix ret; \
		for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) ret[col][row] = mat[col][row]; \
		return ret; \
		\
	} \
	\
}

_MakeMatrixUpscaleFunction(mat2, 2, 2) _MakeMatrixUpscaleFunction(dmat2, 2, 2)
_MakeMatrixUpscaleFunction(mat2x3, 2, 3) _MakeMatrixUpscaleFunction(dmat2x3, 2, 3)
_MakeMatrixUpscaleFunction(mat2x4, 2, 4) _MakeMatrixUpscaleFunction(dmat2x4, 2, 4)

_MakeMatrixUpscaleFunction(mat3x2, 3, 2) _MakeMatrixUpscaleFunction(dmat3x2, 3, 2)
_MakeMatrixUpscaleFunction(mat3, 3, 3) _MakeMatrixUpscaleFunction(dmat3, 3, 3)
_MakeMatrixUpscaleFunction(mat3x4, 3, 4) _MakeMatrixUpscaleFunction(dmat3x4, 3, 4)

_MakeMatrixUpscaleFunction(mat4x2, 4, 2) _MakeMatrixUpscaleFunction(dmat4x2, 4, 2)
_MakeMatrixUpscaleFunction(mat4x3, 4, 3) _MakeMatrixUpscaleFunction(dmat4x3, 4, 3)
_MakeMatrixUpscaleFunction(mat4, 4, 4) _MakeMatrixUpscaleFunction(dmat4, 4, 4)

#define _MakeMatmulFunction(d1, d2, d3) \
namespace GL { \
	\
	template <typename T> \
	Matrix ## d1 ## x ## d3<T> operator * (Matrix ## d1 ## x ## d2<T> lh, Matrix ## d2 ## x ## d3<T> rh) { \
		\
		Matrix ## d1 ## x ## d3<T> ret; \
		for (int col = 0; col < d3; col++) ret[col] = lh * rh[col]; \
		return ret; \
		\
	} \
	\
}

_MakeMatmulFunction(2, 2, 2) _MakeMatmulFunction(2, 3, 2) _MakeMatmulFunction(2, 4, 2)
_MakeMatmulFunction(2, 2, 3) _MakeMatmulFunction(2, 3, 3) _MakeMatmulFunction(2, 4, 3)
_MakeMatmulFunction(2, 2, 4) _MakeMatmulFunction(2, 3, 4) _MakeMatmulFunction(2, 4, 4)

_MakeMatmulFunction(3, 2, 2) _MakeMatmulFunction(3, 3, 2) _MakeMatmulFunction(3, 4, 2)
_MakeMatmulFunction(3, 2, 3) _MakeMatmulFunction(3, 3, 3) _MakeMatmulFunction(3, 4, 3)
_MakeMatmulFunction(3, 2, 4) _MakeMatmulFunction(3, 3, 4) _MakeMatmulFunction(3, 4, 4)

_MakeMatmulFunction(4, 2, 2) _MakeMatmulFunction(4, 3, 2) _MakeMatmulFunction(4, 4, 2)
_MakeMatmulFunction(4, 2, 3) _MakeMatmulFunction(4, 3, 3) _MakeMatmulFunction(4, 4, 3)
_MakeMatmulFunction(4, 2, 4) _MakeMatmulFunction(4, 3, 4) _MakeMatmulFunction(4, 4, 4)

#define _MakeTransposeFunction(numrows, numcols) \
namespace GL { \
	\
	template <typename T> \
	Matrix ## numcols ## x ## numrows<T> transpose(Matrix ## numrows ## x ## numcols<T> mat) { \
		\
		Matrix ## numcols ## x ## numrows<T> ret; \
		for (int row = 0; row < numrows; row++) for (int col = 0; col < numcols; col++) ret[row][col] = mat[col][row]; \
		return ret; \
		\
	} \
	\
}

_MakeTransposeFunction(2, 2)
_MakeTransposeFunction(2, 3)
_MakeTransposeFunction(2, 4)

_MakeTransposeFunction(3, 2)
_MakeTransposeFunction(3, 3)
_MakeTransposeFunction(3, 4)

_MakeTransposeFunction(4, 2)
_MakeTransposeFunction(4, 3)
_MakeTransposeFunction(4, 4)

#define _TriangulateMatrix(ret, rowop_0, rowop_1) \
for (uint r = 0u; r < numComps - 1u; r++) { \
	\
	uint firstNonzero = numComps; \
	for (uint i = r; i < numComps; i++) if (m[r][i] != (T)0) { firstNonzero = i; break; } \
	if (firstNonzero == numComps) return ret; \
	\
	if (firstNonzero != r) { rowop_0 } \
	\
	for (uint i = r + 1u; i < numComps; i++) if (m[r][i] != (T)0) { rowop_1 } \
	\
}

#define _MakeDoubleRowop(rowopname, args) \
_rowop_ ## rowopname<S, T>(inv, args, numComps); \
_rowop_ ## rowopname<S, T>(m, args, numComps);

namespace GL {

	template <typename S, typename T>
	void _rowop_scale(S& m, uint row, float factor, uint numComps) { for (uint i = 0u; i < numComps; i++) m[i][row] *= factor; }

	template <typename S, typename T>
	void _rowop_swap(S& m, uint row0, uint row1, uint numComps) {

		Vector4D<T> temp(m[0u][row0], m[1u][row0], m[2u][row0], m[3u][row0]);
		for (uint i = 0u; i < numComps; i++) m[i][row0] = m[i][row1];
		for (uint i = 0u; i < numComps; i++) m[i][row1] = temp[i];

	}
	
	template <typename S, typename T>
	void _rowop_scaledSum(S& m, uint rowSrc, uint rowDest, float factor, uint numComps) { for (uint i = 0u; i < numComps; i++) m[i][rowDest] += m[i][rowSrc] * factor; }

	template <typename S, typename T>
	T _determinant(S& m, uint numComps) {

		T sign = (T)1;

		_TriangulateMatrix((T)0, _va_args(_rowop_swap<S, T>(m, firstNonzero, r, numComps); sign *= (T)-1;), _va_args(_rowop_scaledSum<S, T>(m, r, i, -m[r][i] / m[r][r], numComps);))
		
		T det = m[0u][0u];
		for (uint i = 1u; i < numComps; i++) det *= m[i][i];
		return det * sign;

	}

	template <typename S, typename T>
	S _inverse(S& m, uint numComps) {

		S inv;

		_TriangulateMatrix(inv, _MakeDoubleRowop(swap, _va_args(firstNonzero, r)), _MakeDoubleRowop(scaledSum, _va_args(r, i, -m[r][i] / m[r][r])))
		
		for (uint r = 1u; r < numComps; r++) for (uint s = 0u; s < r; s++) if (m[r][s] != (T)0) { _MakeDoubleRowop(scaledSum, _va_args(r, s, -m[r][s] / m[r][r])) }
		for (uint r = 0u; r < numComps; r++) { _MakeDoubleRowop(scale, _va_args(r, (T)1 / m[r][r])) }
		
		return inv;

	}

	template <typename T> T determinant(Matrix2x2<T> mat) { return _determinant<Matrix2x2<T>, T>(mat, 2u); }
	template <typename T> T determinant(Matrix3x3<T> mat) { return _determinant<Matrix3x3<T>, T>(mat, 3u); }
	template <typename T> T determinant(Matrix4x4<T> mat) { return _determinant<Matrix4x4<T>, T>(mat, 4u); }

	template <typename T> Matrix2x2<T> inverse(Matrix2x2<T> mat) { return _inverse<Matrix2x2<T>, T>(mat, 2u); }
	template <typename T> Matrix3x3<T> inverse(Matrix3x3<T> mat) { return _inverse<Matrix3x3<T>, T>(mat, 3u); }
	template <typename T> Matrix4x4<T> inverse(Matrix4x4<T> mat) { return _inverse<Matrix4x4<T>, T>(mat, 4u); }

	template <typename T>
	Matrix2x2<T> rotate(T angle) {

		T c = std::cos(angle);
		T s = std::sin(angle);
		
		if (std::abs(c) < (T)0.0001) c = (T)0;
		if (std::abs(s) < (T)0.0001) s = (T)0;

		return Matrix2x2<T>(c, s, -s, c);

	}

	template <typename T>
	Vector3D<T> orthogonalTo(Vector3D<T> v) {

		if (v.x != (T)0) return Vector3D<T>((-v.y - v.z) / v.x, (T)1, (T)1);
		else if (v.y != (T)0) return Vector3D<T>((T)1, (-v.x - v.z) / v.y, (T)1);
		else if (v.z != (T)0) return Vector3D<T>((T)1, (T)1, (-v.x - v.y) / v.z);
		else return Vector3D<T>();

	}

	template <typename T>
	Matrix4x4<T> translate(Vector3D<T> v) {

		return Matrix4x4<T>(
			(T)1, (T)0, (T)0, (T)0,
			(T)0, (T)1, (T)0, (T)0,
			(T)0, (T)0, (T)1, (T)0,
			v.x, v.y, v.z, (T)1
		);

	}

	template <typename T>
	Matrix4x4<T> scale(Vector3D<T> v) {

		return Matrix4x4<T>(
			v.x, (T)0, (T)0, (T)0,
			(T)0, v.y, (T)0, (T)0,
			(T)0, (T)0, v.z, (T)0,
			(T)0, (T)0, (T)0, (T)1
		);

	}

	template <typename T>
	Matrix4x4<T> rotate(Vector3D<T> pos, Vector3D<T> axis, T angle) {

		Vector3D<T> v3 = normalize(axis);
		Vector3D<T> v2 = normalize(orthogonalTo(v3));
		Vector3D<T> v1 = normalize(cross(v2, v3));
		Matrix3x3<T> m(v1, v2, v3);

		return translate(pos) * upscale<Matrix4x4<T>>(m * upscale<Matrix3x3<T>>(rotate(angle)) * transpose(m)) * translate((T)0-pos);

	}

	template <typename T>
	Matrix3x3<T> cameraRotation(Vector3D<T> forward, Vector3D<T> up) {

		forward = normalize(forward);
		Vector3D<T> right = normalize(cross(forward, up));
		up = normalize(cross(right, forward));

		return Matrix3x3<T>(right, up, (T)0-forward);

	}

	template <typename T>
	Matrix4x4<T> lookAt(Vector3D<T> pos, Vector3D<T> target, Vector3D<T> up) { return upscale<Matrix4x4<T>>(transpose(cameraRotation(target - pos, up))) * translate((T)0-pos); }

	template <typename T>
	Matrix4x4<T> ortho(T left, T right, T bottom, T top, T zNear, T zFar) {

		return Matrix4x4<T>(
			2.0f / (right - left), 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
			0.0f, 0.0f, 2.0f / (zNear - zFar), 0.0f,
			(left + right) / (left - right), (bottom + top) / (bottom - top), (zNear + zFar) / (zNear - zFar), 1.0f
		);

	}

	template <typename T>
	Matrix4x4<T> ortho(Vector2D<T> screenSize, T zFar) { return ortho(-screenSize.x / (T)2, screenSize.x / (T)2, -screenSize.y / (T)2, screenSize.y / (T)2, (T)0, zFar); }
	
	template <typename T>
	Matrix4x4<T> frustum(T left, T right, T bottom, T top, T zNear, T zFar) {

		return Matrix4x4<T>(
			2.0f * zNear / (right - left), (T)0, (T)0, (T)0,
			(T)0, 2.0f * zNear / (top - bottom), (T)0, (T)0,
			(right + left) / (right - left), (top + bottom) / (top - bottom), (zNear + zFar) / (zNear - zFar), (T)-1,
			(T)0, (T)0, (T)2 * zNear * zFar / (zNear - zFar), (T)0
		);

	}

	template <typename T>
	Matrix4x4<T> frustum(Vector2D<T> screenSize, T zNear, T zFar) { return frustum(-screenSize.x / (T)2, screenSize.x / (T)2, -screenSize.y / (T)2, screenSize.y / (T)2, zNear, zFar); }

	template <typename T>
	Vector2D<T> getScreenDims(T maxFOV, T aspectRatio, T zNear) {

		T x, y;
		if (aspectRatio > (T)1) {

			x = zNear * tanf(maxFOV / (T)2);
			y = x / aspectRatio;

		}
		else {

			y = zNear * tanf(maxFOV / (T)2);
			x = y * aspectRatio;

		}

		return Vector2D<T>(x, y) * (T)2;

	}

	template <typename T>
	Matrix4x4<T> perspective(T maxFOV, T aspectRatio, T zNear, T zFar) { return frustum(getScreenDims(maxFOV, aspectRatio, zNear), zNear, zFar); }

}

#endif

// Hi lol