#ifndef MODELSTRUCTS_HPP
#define MODELSTRUCTS_HPP

#include "./../util/GL-math.hpp"

namespace GL {

	struct SampleSettings {

		bool albedoTex3D = false;
		bool normalTex3D = false;
		bool metallicTex3D = false;
		bool roughnessTex3D = false;

	};

	struct ShadowSettings {

		bool hasOverheadShadow = false;
		bool overheadShadowFancy = false;
		unsigned int overheadMapSideLength = 4096u;

		unsigned int numLights = 0u;
		unsigned int shadowCubeMapSideLength = 2048u;

	};

	struct BoundingBox {

		vec3 start, end;

		BoundingBox();
		BoundingBox(vec3 s, vec3 e);

		BoundingBox operator + (BoundingBox bb);
		BoundingBox operator * (BoundingBox bb);

	};

}

#endif