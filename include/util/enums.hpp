#ifndef ENUMS_HPP
#define ENUMS_HPP

namespace GL {

	enum class DataType { F16, F32, I8, I16, I32, U8, U16, U32 };
	enum class ColorFormat { R, RG, RGB, RGBA };
	enum class TextureWrap { REPEAT, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER };
	enum class TextureFilter { NEAREST, LINEAR };
	enum class DepthStencilFormat { DEPTH_32, DEPTH_24, DEPTH_16, DEPTH_32_STENCIL_8, DEPTH_24_STENCIL_8 };
	enum class DrawMode { POINTS, LINES, TRIANGLES, LINE_STRIP, LINE_LOOP, TRIANGLE_STRIP, TRIANGLE_FAN };
	enum class UniformType {
		INT, UINT, FLOAT, DOUBLE, IVEC2, UVEC2, VEC2, DVEC2, IVEC3, UVEC3, VEC3, DVEC3, IVEC4, UVEC4, VEC4, DVEC4,
		MAT2, DMAT2, MAT3, DMAT3, MAT4, DMAT4, MAT2x3, DMAT2x3, MAT2x4, DMAT2x4, MAT3x2, DMAT3x2, MAT3x4, DMAT3x4, MAT4x2, DMAT4x2, MAT4x3, DMAT4x3,
		SAMPLER = INT
	};
	enum class ShaderType { VERTEX, FRAGMENT, COMPUTE };
	enum class AccessType { READ_ONLY, WRITE_ONLY, READ_WRITE };
	enum class CullMode { NONE, FRONT_FACE, BACK_FACE };
	enum class TextureType { ALBEDO, NORMAL, METALLIC, ROUGHNESS };
	enum class FontWrap { NONE, LETTER, WORD };
	enum BufferType { COLOR_BUFFER = 1, DEPTH_BUFFER = 2, STENCIL_BUFFER = 4 };
	enum ChartFormat { POINTS = 0, COLORS = 1, WIDTHS = 2 };
	
}

#endif