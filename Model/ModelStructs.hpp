#ifndef MODELSTRUCTS_HPP
#define MODELSTRUCTS_HPP

namespace GL {

	struct SampleSettings {

		bool albedoTex3D = false;
		bool normalTex3D = false;
		bool metallicTex3D = false;
		bool roughnessTex3D = false;

	};

	struct ShadowSettings {

		bool hasOverheadShadow = false;
		unsigned int overheadMapSideLength = 4096u;

		unsigned int numLights = 0u;
		unsigned int shadowCubeMapSideLength = 2048u;

	};

	struct BoundingBox {

		vec3 start, end;

		BoundingBox();
		BoundingBox(vec3 s, vec3 e);

	};

}

GL::BoundingBox::BoundingBox() : start(0.0f), end(0.0f) { }

GL::BoundingBox::BoundingBox(vec3 s, vec3 e) : start(min(s, e)), end(max(s, e)) { }

#endif