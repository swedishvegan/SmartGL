
#include "./ModelConverter.hpp"

#ifdef BUILD_MODEL_CONVERTER

GL::ModelConverter::ModelConverter(const char* meshFile, const char* outFile) {

	WriteBinaryFile wbf(outFile, 1024 * 512);

	importer.SetPropertyInteger(AI_CONFIG_PP_FD_REMOVE, 1);
	scene = importer.ReadFile(meshFile, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_FindDegenerates | aiProcess_GenUVCoords | aiProcess_TransformUVCoords);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) throw Exception("Assimp import error: " + std::string(importer.GetErrorString()));
	
	getNumMeshes(scene->mRootNode);

	if (scene->mNumAnimations > 0 && scene->mAnimations) {

		boneNodes = new ModelConverter_types::BoneNode[numBoneNodes];
		unsigned int idx = 0u;

		buildBoneNodeArray(scene->mRootNode, idx, numBoneNodes, 0u);
		getAnimationData();
		findWhichNodesAreBones(scene->mRootNode);

		for (unsigned int i = 0u; i < numBoneNodes; i++) if (boneNodes[i].isBone) {

			boneNodes[i].glIndex = numBones;
			numBones++;

		}

		if (numBones > 64u) throw Exception("The maximum number of allowed bones per model is 64 but the model \"" + std::string(meshFile) + "\" has " + std::to_string(numBones) + ".");

	}

	std::string baseDirectory(meshFile);
	baseDirectory = baseDirectory.substr(0, baseDirectory.find_last_of('/') + 1);
	loadTextures(baseDirectory);

	meshes_head = new ModelConverter_types::VertexArrayData{ };
	indices = new unsigned int* [numMeshes];
	numElements = new unsigned int[numMeshes];
	matIndices = new unsigned int[numMeshes];

	unsigned int index = 0u;
	ModelConverter_types::VertexArrayData* curMesh = meshes_head;
	processNode(scene->mRootNode, aiMatrix4x4(), curMesh, index);

	wbf.writeRawData((char*)"model", 5);
	saveMaterials(wbf);
	saveBoneNodes(wbf);
	saveMeshes(wbf);

}

GL::ModelConverter::~ModelConverter() {

	if (mats) {

		for (unsigned int i = 0u; i < numMats; i++) {

			if (mats[i].baseTexData) delete[] mats[i].baseTexData;
			if (mats[i].metallicRoughnessTexData) delete[] mats[i].metallicRoughnessTexData;
			if (mats[i].normalTexData) delete[] mats[i].normalTexData;

		}
		
		delete[] mats;
		delete[] matFormats;
		delete[] representations;

	}

	if (meshes_head) {

		ModelConverter_types::VertexArrayData* mesh = meshes_head;
		while (mesh->next) {

			ModelConverter_types::VertexArrayData* temp = mesh->next;
			delete[] (char*)mesh->data;
			delete mesh;
			mesh = temp;

		}

		for (unsigned int i = 0u; i < numMeshes; i++) { delete[] indices[i]; }
		delete[] indices;

		delete[] numElements;
		delete[] matIndices;

	}

	if (boneNodes) {

		for (unsigned int i = 0u; i < numBoneNodes; i++) {

			if (boneNodes[i].childIndices) delete[] boneNodes[i].childIndices;
			if (boneNodes[i].animations) {

				for (unsigned int j = 0u; j < numAnimations; j++) if (boneNodes[i].animations[j]) {

					if (boneNodes[i].animations[j]->translations) delete[] boneNodes[i].animations[j]->translations;
					if (boneNodes[i].animations[j]->rots) delete[] boneNodes[i].animations[j]->rots;
					if (boneNodes[i].animations[j]->scalings) delete[] boneNodes[i].animations[j]->scalings;

					delete[] boneNodes[i].animations[j];

				}

				delete[] boneNodes[i].animations;

			}

		}

		delete[] boneNodes;

	}

	if (animationData) delete[] animationData;

}

void GL::ModelConverter::buildBoneNodeArray(aiNode* node, unsigned int& idx, unsigned int parentIdx, unsigned int parentArrayIdx) {

	boneNodes[idx].name = node->mName.C_Str();
	boneNodes[idx].trans = assimpToGL(node->mTransformation);
	boneNodes[idx].glIndex = 64u;
	boneNodes[idx].numChildren = node->mNumChildren;
	boneNodes[idx].offset = mat4();
	boneNodes[idx].isBone = false;
	if (boneNodes[idx].numChildren) boneNodes[idx].childIndices = new unsigned int[boneNodes[idx].numChildren];
	if (parentIdx < numBoneNodes) boneNodes[parentIdx].childIndices[parentArrayIdx] = idx;
	
	unsigned int tempIdx = idx;
	idx++;

	for (unsigned int i = 0u; i < boneNodes[tempIdx].numChildren; i++) buildBoneNodeArray(node->mChildren[i], idx, tempIdx, i);

}

void GL::ModelConverter::getAnimationData() {

	numAnimations = scene->mNumAnimations;
	if (!numAnimations) return;
	animationData = new ModelConverter_types::AnimationData[numAnimations];

	for (unsigned int i = 0u; i < numAnimations; i++) {

		aiAnimation* anim = scene->mAnimations[i];
		float duration = (float)anim->mDuration;
		float TPS = (float)anim->mTicksPerSecond;
		if (TPS == 0.0f) TPS = 25.0f;

		animationData[i].duration = duration;
		animationData[i].TPS = TPS;

		for (unsigned int j = 0u; j < anim->mNumChannels; j++) {

			aiNodeAnim* channel = anim->mChannels[j];
			unsigned int boneIdx = numBoneNodes;
			for (unsigned int k = 0u; k < numBoneNodes; k++) if (boneNodes[k].name == channel->mNodeName.C_Str()) { boneIdx = k; break; }
			if (boneIdx == numBoneNodes) continue;

			ModelConverter_types::BoneNode& bone = boneNodes[boneIdx];
			if (!bone.animations) {

				bone.animations = new ModelConverter_types::Animation * [numAnimations];
				for (unsigned int k = 0u; k < numAnimations; k++) bone.animations[k] = nullptr;

			}
			bone.animations[i] = new ModelConverter_types::Animation{ };
			ModelConverter_types::Animation& a = *(bone.animations[i]);
			a.numTranslations = channel->mNumPositionKeys;
			a.numRots = channel->mNumRotationKeys;
			a.numScalings = channel->mNumScalingKeys;
			a.translations = new vec3[a.numTranslations];
			a.rots = new vec4[a.numRots];
			a.scalings = new vec3[a.numScalings];
			a.translationTimes = new float[a.numTranslations];
			a.rotTimes = new float[a.numRots];
			a.scalingTimes = new float[a.numScalings];

			for (unsigned int k = 0u; k < a.numTranslations; k++) {

				aiVector3D temp = channel->mPositionKeys[k].mValue;
				a.translations[k] = vec3(temp.x, temp.y, temp.z);
				a.translationTimes[k] = (float)channel->mPositionKeys[k].mTime;

			}

			for (unsigned int k = 0u; k < a.numRots; k++) {

				aiQuaternion temp = channel->mRotationKeys[k].mValue;
				a.rots[k] = vec4(temp.x, temp.y, temp.z, temp.w);
				a.rotTimes[k] = (float)channel->mRotationKeys[k].mTime;

			}

			for (unsigned int k = 0u; k < a.numScalings; k++) {

				aiVector3D temp = channel->mScalingKeys[k].mValue;
				a.scalings[k] = vec3(temp.x, temp.y, temp.z);
				a.scalingTimes[k] = (float)channel->mScalingKeys[k].mTime;

			}
		}
	}
}

void GL::ModelConverter::findWhichNodesAreBones(aiNode* node) {

	for (unsigned int i = 0u; i < node->mNumMeshes; i++) {

		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		for (unsigned int j = 0u; j < mesh->mNumBones; j++) {

			aiBone* bone = mesh->mBones[j];
			if (bone->mNumWeights == 0u) continue;

			const char* boneName = bone->mName.C_Str();
			for (unsigned int k = 0u; k < numBoneNodes; k++) if (boneNodes[k].name == boneName) { boneNodes[k].isBone = true; break; }

		}

	}

	for (unsigned int i = 0u; i < node->mNumChildren; i++) findWhichNodesAreBones(node->mChildren[i]);

}

#define _GL_ModelConverter_loadTexture(aiType, textureType, unit, format) \
if (mat->GetTextureCount(aiTextureType_ ## aiType)) { \
	\
	aiString textureFile; \
	mat->Get(AI_MATKEY_TEXTURE(aiTextureType_ ## aiType, 0), textureFile); \
	if (auto texture = scene->GetEmbeddedTexture(textureFile.C_Str())) { \
		\
		if (texture->mHeight == 0u) { \
			\
			Image image((const unsigned char*)(texture->pcData), texture->mWidth, ColorFormat::RGBA, 0u, textureFile.C_Str()); \
			if (image.loadSuccess()) mats[i].textureType ## Idx = loadTexGL(image, unit, i, ColorFormat::format, mats[i].textureType ## TexData, mats[i].textureType ## Width, mats[i].textureType ## Height); \
			mats[i].path_ ## textureType = textureFile.C_Str(); \
			\
		} \
		\
	} \
	else { \
		\
		std::string tempDir = dir; \
		tempDir.append(textureFile.C_Str()); \
		\
		Image image(tempDir.c_str()); \
		if (image.loadSuccess()) mats[i].textureType ## Idx = loadTexGL(image, unit, i, ColorFormat::format, mats[i].textureType ## TexData, mats[i].textureType ## Width, mats[i].textureType ## Height); \
		mats[i].path_ ## textureType = tempDir.c_str(); \
		\
	} \
	\
}

void GL::ModelConverter::loadTextures(std::string thisDirectory) {

	numMats = scene->mNumMaterials;
	mats = new ModelConverter_types::Material[numMats]{ };
	matFormats = new ModelConverter_types::MaterialFormat[numMats];
	representations = new VertexStructRepresentation[numMats];

	aiString aistr;
	std::string dir = thisDirectory;

	for (unsigned int i = 0u; i < numMats; i++) {

		aiMaterial* mat = scene->mMaterials[i];

		_GL_ModelConverter_loadTexture(DIFFUSE, base, 0u, RGBA);
		_GL_ModelConverter_loadTexture(NORMALS, normal, 2u, RGB);
		_GL_ModelConverter_loadTexture(UNKNOWN, metallicRoughness, 1u, RG);

		aiColor3D aiColor;
		if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == AI_SUCCESS) mats[i].baseColor = vec4(aiColor[0], aiColor[1], aiColor[2], 1.0f);

		bool hasAlbedo = (mats[i].baseTexData || mats[i].baseIdx >= 0);
		bool hasMR = (mats[i].metallicRoughnessTexData || mats[i].metallicRoughnessIdx >= 0);
		bool hasN = (mats[i].normalTexData || mats[i].normalIdx >= 0);

		if (hasMR && hasN) matFormats[i] = ModelConverter_types::MaterialFormat::BMRN;
		else if (hasMR && !hasN) matFormats[i] = ModelConverter_types::MaterialFormat::BMR;
		else if (!hasMR && hasN) matFormats[i] = ModelConverter_types::MaterialFormat::BN;
		else matFormats[i] = ModelConverter_types::MaterialFormat::B;

		representations[i] = VertexStructRepresentation(hasAlbedo || hasMR || hasN, hasN, boneNodes != nullptr);

	}

}

void GL::ModelConverter::getNumMeshes(aiNode* node) {

	numMeshes += node->mNumMeshes;
	numBoneNodes++;
	for (unsigned int i = 0u; i < node->mNumChildren; i++) getNumMeshes(node->mChildren[i]);

}

#define _GL_ModelConverter_declareReference(name, type, whichEnum, theIndex) \
type& name = *(type*)((char*)curMesh->data + rep.dataSize * theIndex + rep.getComponentOffset(VertexStructRepresentation::whichEnum));

void GL::ModelConverter::processNode(aiNode* node, aiMatrix4x4 transform, ModelConverter_types::VertexArrayData*& curMesh, unsigned int& index) {

	transform = transform * node->mTransformation;
	mat4 GL_transform = (boneNodes) ? mat4() : assimpToGL(transform);
	mat3 normalTransform = (boneNodes) ? mat3() : calcNormalMatrix(GL_transform);

	for (unsigned int i = 0u; i < node->mNumMeshes; i++) {

		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		matIndices[index] = mesh->mMaterialIndex;
		VertexStructRepresentation& rep = representations[matIndices[index]];
		
		curMesh->hasNormalMap = (int)matFormats[matIndices[index]] % 4 >= 2;
		curMesh->dataSize = mesh->mNumVertices * rep.dataSize;
		curMesh->vertexSize = rep.dataSize;
		curMesh->data = (void*)(new char[curMesh->dataSize]);

		for (unsigned int j = 0u; j < mesh->mNumVertices; j++) {

			aiVector3D aiPos = mesh->mVertices[j];
			aiVector3D aiNormal = mesh->mNormals[j];

			_GL_ModelConverter_declareReference(pos, vec3, POSITION, j);
			_GL_ModelConverter_declareReference(normal, vec3, NORMAL, j);

			pos = (GL_transform * vec4(aiPos.x, aiPos.y, aiPos.z, 1.0f))(0, 1, 2);
			normal = normalize(normalTransform * vec3(aiNormal));

			if (rep.hasComponent(VertexStructRepresentation::TEX_COORDS)) {

				_GL_ModelConverter_declareReference(texCoords, vec2, TEX_COORDS, j);

				aiVector3D aiTexCoords;
				if (mesh->mTextureCoords[0]) aiTexCoords = mesh->mTextureCoords[0][j];
				texCoords = vec2(aiTexCoords);

			}

			if (rep.hasComponent(VertexStructRepresentation::TANGENT)) {

				_GL_ModelConverter_declareReference(tangent, vec3, TANGENT, j);
				_GL_ModelConverter_declareReference(bitangent, vec3, BITANGENT, j);

				aiVector3D aiTangents = mesh->mTangents[j];
				aiVector3D aiBitangents = mesh->mBitangents[j];

				tangent = normalize(vec3((GL_transform * vec4(aiTangents.x, aiTangents.y, aiTangents.z, 1.0f))(0, 1, 2)));
				bitangent = normalize(vec3((GL_transform * vec4(aiBitangents.x, aiBitangents.y, aiBitangents.z, 1.0f))(0, 1, 2)));

			}

			if (rep.hasComponent(VertexStructRepresentation::BONE_INDICES)) {

				_GL_ModelConverter_declareReference(boneIndices, ivec4, BONE_INDICES, j);
				_GL_ModelConverter_declareReference(boneWeights, vec4, BONE_WEIGHTS, j);

				boneIndices = ivec4(); boneWeights = ivec4();

			}

			if (first) {

				bboxStart = pos;
				bboxEnd = pos;
				first = false;

			}
			else for (int k = 0; k < 3; k++) {

				if (pos[k] < bboxStart[k]) bboxStart[k] = pos[k];
				if (pos[k] > bboxEnd[k]) bboxEnd[k] = pos[k];

			}
			
		}

		numElements[index] = 3u * mesh->mNumFaces;
		indices[index] = new unsigned int[numElements[index]];

		for (unsigned int j = 0u; j < mesh->mNumFaces; j++)
			for (unsigned int k = 0u; k < mesh->mFaces[j].mNumIndices; k++) indices[index][3u * j + k] = mesh->mFaces[j].mIndices[k];
		
		if (boneNodes) for (unsigned int j = 0u; j < mesh->mNumBones; j++) {

			aiBone* bone = mesh->mBones[j];

			unsigned int glIndex = 0u;
			for (unsigned int k = 0u; k < numBoneNodes; k++) if (boneNodes[k].name == bone->mName.C_Str()) {

				boneNodes[k].offset = assimpToGL(bone->mOffsetMatrix);
				glIndex = boneNodes[k].glIndex;
				break;

			}

			for (unsigned int k = 0u; k < bone->mNumWeights; k++) {

				unsigned int idx = bone->mWeights[k].mVertexId;
				float weight = bone->mWeights[k].mWeight;

				_GL_ModelConverter_declareReference(boneIndices, ivec4, BONE_INDICES, idx);
				_GL_ModelConverter_declareReference(boneWeights, vec4, BONE_WEIGHTS, idx);

				for (unsigned int l = 0u; l < 4u; l++)

					if (boneWeights[l] == 0.0f) {

						boneWeights[l] = weight;
						boneIndices[l] = (int)glIndex;
						break;

					}
			}
		}
		
		index++;
		curMesh->next = new ModelConverter_types::VertexArrayData{ nullptr, 0u, 0u, false, nullptr };
		curMesh = curMesh->next;

	}

	for (int i = 0; i < node->mNumChildren; i++) processNode(node->mChildren[i], transform, curMesh, index);

}

int GL::ModelConverter::loadTexGL(Image& image, unsigned unit, unsigned int mat_curIndex, ColorFormat format, unsigned char*& texData, unsigned int& w, unsigned int& h) {

	for (unsigned int i = 0u; i < mat_curIndex; i++) {

		if (unit == 0u && image.getInputString() == mats[i].path_base) return i;
		if (unit == 1u && image.getInputString() == mats[i].path_metallicRoughness) return i;
		if (unit == 2u && image.getInputString() == mats[i].path_normal) return i;

	}

	unsigned int numComps = (unsigned int)format + 1u;
	unsigned char* newImage = new unsigned char[image.getWidth() * image.getHeight() * numComps];

	static unsigned int indices[] = {
		0u, 1u, 2u, 3u,
		2u, 1u,
		0u, 1u, 2u
	};
	static unsigned int indexIndices[] = { 0u, 4u, 6u };

	for (unsigned int x = 0u; x < image.getWidth(); x++)
		for (unsigned int y = 0u; y < image.getHeight(); y++)
			for (unsigned int comp = 0u; comp < numComps; comp++)
				newImage[numComps * (image.getWidth() * y + x) + comp] = image(x, y, indices[indexIndices[unit] + comp]);

	texData = newImage;
	w = image.getWidth();
	h = image.getHeight();
	return -1;

}

#define _GL_Model_saveMaterialData(matType, numComps) \
w = mats[i].matType ## Width; h = mats[i].matType ## Height; idx = mats[i].matType ## Idx; \
wbf.write<int>(idx); \
wbf.write<unsigned int>(w); wbf.write<unsigned int>(h); \
if (idx == -1 && w * h != 0u) wbf.writeRawData((char*)mats[i].matType ## TexData, w * h * numComps);

void GL::ModelConverter::saveMaterials(WriteBinaryFile& wbf) {

	wbf.write<unsigned int>((unsigned int)numMats);

	for (unsigned int i = 0u; i < numMats; i++) {

		wbf.write<unsigned int>((unsigned int)matFormats[i]);
		for (int j = 0; j < 4; j++) wbf.write<float>(mats[i].baseColor[j]);

		unsigned int w, h; int idx;
		_GL_Model_saveMaterialData(base, 4);
		_GL_Model_saveMaterialData(normal, 3);
		_GL_Model_saveMaterialData(metallicRoughness, 2);

	}

}

#define _GL_Model_saveAnimationData(animationType, AnimationType, numComps) \
wbf.write<unsigned int>(animation->num ## AnimationType); \
for (unsigned int k = 0u; k < animation->num ## AnimationType; k++) { \
	\
	wbf.write<float>(animation->animationType ## Times[k]); \
	for (unsigned int l = 0u; l < numComps; l++) wbf.write<float>(animation->animationType ## s[k][l]); \
	\
}

void GL::ModelConverter::saveBoneNodes(WriteBinaryFile& wbf) {

	wbf.write<unsigned int>(numAnimations);
	if (numAnimations) {

		wbf.write<unsigned int>(numBones);
		wbf.write<unsigned int>(numBoneNodes);

		for (unsigned int i = 0u; i < numAnimations; i++) {

			wbf.write<float>(animationData[i].duration);
			wbf.write<float>(animationData[i].TPS);

		}

		for (unsigned int i = 0u; i < numBoneNodes; i++) {

			wbf.write<unsigned int>(boneNodes[i].glIndex);
			wbf.write<unsigned int>(boneNodes[i].numChildren);
			wbf.write<bool>((bool)boneNodes[i].animations);
			for (int x = 0; x < 4; x++) for (int y = 0; y < 4; y++) wbf.write<float>(boneNodes[i].trans[x][y]);
			for (int x = 0; x < 4; x++) for (int y = 0; y < 4; y++) wbf.write<float>(boneNodes[i].offset[x][y]);

			for (unsigned int j = 0u; j < boneNodes[i].numChildren; j++) wbf.write<unsigned int>(boneNodes[i].childIndices[j]);

			if (boneNodes[i].animations) for (unsigned int j = 0u; j < numAnimations; j++) {

				ModelConverter_types::Animation* animation = boneNodes[i].animations[j];

				wbf.write<bool>((bool)animation);
				if (animation) {

					_GL_Model_saveAnimationData(scaling, Scalings, 3);
					_GL_Model_saveAnimationData(rot, Rots, 4);
					_GL_Model_saveAnimationData(translation, Translations, 3);

				}
			}
		}
	}
	else for (int i = 0; i < 3; i++) {

		wbf.write<float>(bboxStart[i]);
		wbf.write<float>(bboxEnd[i]);

	}

}

void GL::ModelConverter::saveMeshes(WriteBinaryFile& wbf) {

	wbf.write<unsigned int>(numMeshes);
	ModelConverter_types::VertexArrayData* curMesh = meshes_head;

	for (unsigned int i = 0u; i < numMeshes; i++) {

		wbf.write<unsigned int>(curMesh->dataSize);
		wbf.write<unsigned int>(curMesh->vertexSize);
		wbf.write<bool>(curMesh->hasNormalMap);
		wbf.write<unsigned int>(numElements[i]);
		wbf.write<unsigned int>(matIndices[i]);

		if (numElements[i] && curMesh->dataSize) {

			wbf.writeRawData((char*)curMesh->data, curMesh->dataSize);
			wbf.writeRawData((char*)indices[i], numElements[i] * sizeof(unsigned int));

		}

		curMesh = curMesh->next;

	}

}

GL::mat3 GL::ModelConverter::calcNormalMatrix(mat4 model) {

	mat3 normalMatrix = inverse(mat3(model[0](0, 1, 2), model[1](0, 1, 2), model[2](0, 1, 2)));
	normalMatrix = mat3(
		normalMatrix[0][0], normalMatrix[1][0], normalMatrix[2][0],
		normalMatrix[0][1], normalMatrix[1][1], normalMatrix[2][1],
		normalMatrix[0][2], normalMatrix[1][2], normalMatrix[2][2]
	);
	return normalMatrix;

}

GL::mat4 GL::ModelConverter::assimpToGL(aiMatrix4x4& m) {

	return mat4(
		m.a1, m.b1, m.c1, m.d1,
		m.a2, m.b2, m.c2, m.d2,
		m.a3, m.b3, m.c3, m.d3,
		m.a4, m.b4, m.c4, m.d4
	);

}

#endif