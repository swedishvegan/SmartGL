#ifndef	SHADOWRENDERER_HPP
#define SHADOWRENDERER_HPP

#include "./../util/util.hpp"
#include "./../Framebuffer/Framebuffer.hpp"
#include "./../Framebuffer/Renderbuffer.hpp"
#include "./../Texture/Texture2D.hpp"
#include "./../Texture/TextureCubeMap.hpp"
#include "./../Program/Program.hpp"
#include "./../Uniform/UniformTable.hpp"
#include "./../Uniform/UniformBufferTable.hpp"
#include "./ModelStructs.hpp"

namespace GL {

	class ShadowRenderer : public _util {
	protected:

		static const char* shadowRenderer_vs;

		static ShaderLoader* vertShader_noAnim;
		static ShaderLoader* vertShader_anim;

		static void initializeVertShader();

	};

	class PointLightShadowRenderer : public ShadowRenderer {
	public:

		PointLightShadowRenderer(ShadowSettings settings, unsigned int lightIndex);

		void bindTextures();

		void setUpFaceForDrawing(unsigned int face);

		void setUpShadersForDrawing(bool isAnimated, bool isInstanced);

		~PointLightShadowRenderer();

		static mat4 getPVMatrix(unsigned int face, vec3 lightPos, float zFar);

	private:

		ImageTextureCubeMap lightCubeMap;
		DepthStencilRenderbuffer depthBuffer;
		GLuint fbo = 0u;
		unsigned int lightIndex;

		static Program* shadowRenderer[2];
		static UniformTable* shadowRenderer_unis[2];	
		static const char* shadowRenderer_fs;

	};

	class OverheadShadowRenderer : public ShadowRenderer {
	public:

		OverheadShadowRenderer(ShadowSettings settings);

		bool isFancy();

		void bindTextures();

		void setUpForDrawing(bool outerBuffer);

		void setUpShadersForDrawing(bool isAnimated, bool isInstanced);
		
		void calcPVMatrix(vec3 lightDir, BoundingBox sceneBB, BoundingBox* outerBB, bool isOuter);

		~OverheadShadowRenderer();

		mat4 PV;
		float farPlane;
		
		mat4 outerPV;
		float outerFarPlane;

	protected:

		Framebuffer fb;
		DepthStencilRenderTexture depthTexture;
		Framebuffer* outerFb = nullptr;
		DepthStencilRenderTexture* outerDepthTexture;

		vec3 frustumEndpoints[5];

		static Program* shadowRenderer[2];
		static UniformTable* shadowRenderer_unis[2];
		static const char* shadowRenderer_fs;

	};

}

#endif