#ifndef MODELCONVERTER_HPP
#define MODELCONVERTER_HPP

#ifdef BUILD_MODEL_CONVERTER

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "./../Texture/Image.hpp"
#include "./../util/enums.hpp"
#include "./../util/Exception.hpp"
#include "./../util/BinaryFile.hpp"
#include "./ModelConverter_types.hpp"
#include "./VertexStructRepresentation.hpp"

namespace GL {

	class ModelConverter {
	public:

		ModelConverter(const char* meshFile, const char* outFile);

		~ModelConverter();

		ModelConverter(const ModelConverter&) = delete;
		void operator = (const ModelConverter&) = delete;

	private:

		Assimp::Importer importer;
		const aiScene* scene;

		unsigned int numMeshes = 0u;
		ModelConverter_types::VertexArrayData* meshes_head = nullptr;
		unsigned int** indices;
		unsigned int* numElements;

		unsigned int numMats;
		ModelConverter_types::Material* mats = nullptr;
		ModelConverter_types::MaterialFormat* matFormats;
		VertexStructRepresentation* representations;
		unsigned int* matIndices;

		ModelConverter_types::BoneNode* boneNodes = nullptr;
		unsigned int numBoneNodes = 0u;
		unsigned int numBones = 0u;
		unsigned int numAnimations = 0u;
		ModelConverter_types::AnimationData* animationData = nullptr;

		vec3 bboxStart, bboxEnd;
		bool first = true;

		void buildBoneNodeArray(aiNode* node, unsigned int& idx, unsigned int parentIdx, unsigned int parentArrayIdx);

		void getAnimationData();

		void findWhichNodesAreBones(aiNode* node);

		void loadTextures(std::string thisDirectory);

		void getNumMeshes(aiNode* node);

		void processNode(aiNode* node, aiMatrix4x4 transform, ModelConverter_types::VertexArrayData*& curMesh, unsigned int& index);

		int loadTexGL(Image& image, unsigned int unit, unsigned int mat_curIndex, ColorFormat format, unsigned char*& texData, unsigned int& w, unsigned int& h);

		void saveMaterials(WriteBinaryFile& wbf);

		void saveBoneNodes(WriteBinaryFile& wbf);

		void saveMeshes(WriteBinaryFile& wbf);

		static mat3 calcNormalMatrix(mat4 model);

		static mat4 assimpToGL(aiMatrix4x4& m);

	};

}

#endif

#endif