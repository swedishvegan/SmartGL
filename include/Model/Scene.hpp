#ifndef SCENE_HPP
#define SCENE_HPP

#include <vector>

#include "./../Program/ShaderLoader.hpp"
#include "./../Program/Program.hpp"
#include "./../Uniform/UniformTable.hpp"
#include "./../Uniform/UniformBufferTable.hpp"
#include "./../Texture/Texture2D.hpp"
#include "./../Texture/TextureCubeMap.hpp"
#include "./../util/CubeMapConvolution.hpp"
#include "./../Framebuffer/RenderTexture.hpp"
#include "./../Framebuffer/Framebuffer.hpp"
#include "./../VertexArray/VertexArray.hpp"
#include "./ShadowRenderer.hpp"
#include "./ModelStructs.hpp"

namespace GL {

	class Scene;
	class Drawable { public: virtual void draw(Scene& scene, SampleSettings reqSettings) = 0; virtual void drawShadow(mat4 PV, mat4 model, Scene& scene) = 0; virtual mat4 getModelMatrix() const = 0; virtual bool isAnimated() const = 0; virtual bool isInstanced() const = 0; virtual BoundingBox getWorldSpaceBoundingBoxApproximation() const = 0; };
	
	class Scene : public _util {
	public:
		
		Scene(float zNear, float zFar, ShadowSettings shadowSettings = ShadowSettings{ });
		
		Scene(ImageTextureCubeMap& background, float zNear, float zFar, ShadowSettings shadowSettings = ShadowSettings{ }, unsigned int specularMapDetail = 512u);

		Scene(uvec2 screenSize, float zNear, float zFar, ShadowSettings shadowSettings = ShadowSettings{ });

		Scene(uvec2 screenSize, ImageTextureCubeMap& background, float zNear, float zFar, ShadowSettings shadowSettings = ShadowSettings{ }, unsigned int specularMapDetail = 512u);

		void addModel(Drawable& model, bool use = true, bool castsShadow = true, SampleSettings sampleSettings = SampleSettings{ });

		void useModel(Drawable& model, bool use);

		void setModelShouldCastShadow(Drawable& model, bool castsShadow);

		void applySampleSettings(Drawable& model, SampleSettings sampleSettings);

		void draw();

		void draw(Framebuffer& fb);

		bool hasBackground() const;

		void setLightPosition(unsigned int index, vec3 position);

		void setLightColor(unsigned int index, vec3 color);

		void setBackgroundColor(vec3 color);

		void setBackgroundBrightness(float brightness);

		void setOverheadLightDirection(vec3 direction);

		void setOverheadLightColor(vec3 color);
		
		vec3 getLightPosition(unsigned int index) const;

		vec3 getLightColor(unsigned int index) const;

		vec3 getBackgroundColor() const;

		float getBackgroundBrightness() const;

		vec3 getOverheadLightDirection() const;

		vec3 getOverheadLightColor() const;

		vec3 getCameraPosition() const;

		mat4 getPerspectiveMatrix() const;

		mat4 getOverheadShadowPVMatrix() const;

		float getOverheadShadowFarPlane() const;

		BoundingBox getOverheadShadowBoundingBox() const;

		mat4 getOverheadOuterShadowPVMatrix() const;

		float getOverheadOuterShadowFarPlane() const;

		BoundingBox getOverheadOuterShadowBoundingBox() const;

		BoundingBox getBoundingBoxApproximation();

		bool usesFancyOverheadShadows() const;

		unsigned int getNumPointLights() const;

		void updateCamera(BoundingBox sceneBB, vec3 cameraPosition, vec3 lookingAt, vec3 up, float FoV);

		void updateCamera(BoundingBox closeBB, BoundingBox farBB, vec3 cameraPosition, vec3 lookingAt, vec3 up, float FoV);
		
		void drawBackground();

		void drawToFramebuffer(Framebuffer* fb);

		void use();

		~Scene();

	private:

		ColorRenderTexture fbColor;
		DepthStencilRenderTexture fbDepth;
		Framebuffer fb;

		std::vector<Drawable*> models;
		std::vector<bool> isModelUsed;
		std::vector<bool> modelCastsShadow;
		std::vector<SampleSettings> modelSettings;
		
		float zNear;
		float zFar;
		float aspectRatio;
		vec3 camPos;
		mat4 perspectiveMatrix;

		vec3 lightPositions[16];
		vec3 lightColors[16];
		vec3 bgColor;
		float bgBrightness = 1.0f;
		ImageTextureCubeMap* bg;
		IrradianceCubeMap<unsigned char>* irradianceMap = nullptr;
		SpecularCubeMap<unsigned char>* specularMap = nullptr;

		bool cameraUpdated = false;
		bool backgroundUsed = false;

		unsigned int numPointLights;
		PointLightShadowRenderer** pointLightShadowRenderers = nullptr;
		OverheadShadowRenderer* overheadShadowRenderer = nullptr;
		BoundingBox bb, outerBb;

		bool alreadyUsing = false;

		vec3 lightDirection = vec3(0.0f, -1.0f, 0.0f);
		vec3 lightColor = vec3(0.0f);

		unsigned int specularDim = 512u;

		static Texture2D* texBRDF;
		static Program* progBRDF;
		static Program* progScene;
		static Program* progLights;
		static UniformTable* progLightsUnis;
		static Program* progSkybox;
		static GLint progSkybox_bgBrightness;
		static UniformBufferTable* raytraceUnis;

		static const char* progBRDF_source[3];
		static const char* progScene_source[2];
		static const char* progLights_source[2];
		static const char* progSkybox_source[2];
		
		void init(ImageTextureCubeMap* background, float zNear, float zFar, ShadowSettings shadowSettings);

		void updateCamera_commonCode(BoundingBox closeBB, BoundingBox* farBB, vec3 cameraPosition, vec3 lookingAt, vec3 up, float FoV);

		void draw_commonCode(Framebuffer* fb);

	};

}

#endif