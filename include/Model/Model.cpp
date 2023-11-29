

#include "./Model.hpp"

GL::Model::Model(const char* filePath, bool preCompilePrograms) { 

	isPhysicsModel = false;
	_GL_Model_constructor();

}

void GL::Model::setModelMatrix(GL::mat4 model) { this->model = model; }

GL::mat4 GL::Model::getModelMatrix() const { return model; }

void GL::Model::setColor(GL::vec3 color) { customColor = true; albedoColor = clamp(color, 0.0f, 1.0f); }

void GL::Model::useDefaultColor() { customColor = false; }

void GL::Model::setMetallicValue(float value) { metallic = std::sqrt(clamp(value, 0.0f, 1.0f)); }

void GL::Model::setRoughnessValue(float value) { roughness = clamp(value * value, 0.0f, 1.0f); }

void GL::Model::setAmbientOcclusion(float value) { ao = clamp(value, 0.0f, 1.0f); }

void GL::Model::setShadowOcclusion(float value) { shadowOcclusion = clamp(value, 0.0f, 1.0f); }

void GL::Model::setInstanceBuffer(GL::ModelInstanceBuffer& instances) { instancesBuf = &instances; shouldUseInstances = true; }

void GL::Model::useInstances(bool use) { shouldUseInstances = use; }

void GL::Model::setCustomProgram(GL::ModelProgram& program) { customProg = &program; shouldUseProg = true; }

void GL::Model::useCustomProgram(bool use) { shouldUseProg = use; }

float GL::Model::getMetallicValue() const { return metallic; }

float GL::Model::getRoughnessValue() const { return roughness; }

float GL::Model::getAmbientOcclusion() const { return ao; }

float GL::Model::getShadowOcclusion() const { return shadowOcclusion; }

unsigned int GL::Model::getNumAnimations() const { return modelData[thisModelDataIndex].numAnimations; }

GL::BoundingBox GL::Model::getBoundingBox() const {
	
	if (modelData[thisModelDataIndex].numAnimations) throw Exception("An animated model has no default bounding box.");
	return modelData[thisModelDataIndex].bbox;

}

GL::BoundingBox GL::Model::getWorldSpaceBoundingBoxApproximation() const {

	if (modelData[thisModelDataIndex].numAnimations) throw Exception("An animated model has no default bounding box.");
	
	BoundingBox bb = modelData[thisModelDataIndex].bbox;
	return BoundingBox(
		(this->getModelMatrix() * toVec4(bb.start))(0, 1, 2),
		(this->getModelMatrix() * toVec4(bb.end))(0, 1, 2)
	);

}

bool GL::Model::isAnimated() const { return modelData[thisModelDataIndex].boneNodes; }

void GL::Model::playAnimation(unsigned int index, bool loop) {

	if (!modelData[thisModelDataIndex].numAnimations) return;

	index %= modelData[thisModelDataIndex].numAnimations;
	animationIndex = index;
	isAnimationPlaying = true;
	isAnimationLooped = loop;
	isAnimationFinished = false;
	t0 = 0.0f;
	timer.reset();

}

void GL::Model::stopAnimation() { isAnimationPlaying = false; isAnimationFinished = true; }

void GL::Model::resetAnimation() { animationIndex = 0u; t = 0.0f; t0 = 0.0f; isAnimationPlaying = false; isAnimationFinished = false; }

bool GL::Model::isAnimationDone() const { return isAnimationFinished; }

bool GL::Model::isInstanced() const { return instancesBuf && shouldUseInstances; }

void GL::Model::setSamplingFactor3D(GL::TextureType type, float value) {

	if (value <= 0.0f) value = _GL_Model_defaultStretch;
	if (type == TextureType::ALBEDO) albedoStretch = value;
	else if (type == TextureType::METALLIC) metallicStretch = value;
	else if (type == TextureType::ROUGHNESS) roughnessStretch = value;
	else normalStretch = value;

}

void GL::Model::draw(GL::Scene& scene, GL::SampleSettings reqSettings) {
	
	ModelInstanceBuffer* instances = nullptr;
	if (shouldUseInstances) instances = instancesBuf;

	if (instances && isPhysicsModel) throw Exception("Physics models cannot be instanced.");

	ModelProgram* program = nullptr;
	if (shouldUseProg) program = customProg;

	mat3 normalMatrix(getModelMatrix());
	normalMatrix = transpose(inverse(normalMatrix));

	scene.drawBackground();
	scene.use();

	initUBOs();
	updateUBOs(scene.getPerspectiveMatrix(), getModelMatrix(), normalMatrix, scene, false);

	if (instances) {

		instances->update();
		instances->bind();

	}

	for (int i = 0; i < modelData[thisModelDataIndex].numMeshes; i++) {

		if (!modelData[thisModelDataIndex].vaos[i]) continue;

		Model_types::Material& mat = modelData[thisModelDataIndex].mats[modelData[thisModelDataIndex].matIndices[i]];

		if (mat.baseTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mat.baseTex);
		}

		if (mat.metallicRoughnessTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mat.metallicRoughnessTex);
		}

		if (mat.normalTex) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, mat.normalTex);
		}

		Model_types::SampleType albedoType = mat.baseTex ? Model_types::SampleType::TEXTURE_2D : (reqSettings.albedoTex3D ? Model_types::SampleType::TEXTURE_3D : Model_types::SampleType::UNIFORM);
		Model_types::SampleType normalType = mat.normalTex ? Model_types::SampleType::TEXTURE_2D : (reqSettings.normalTex3D ? Model_types::SampleType::TEXTURE_3D : Model_types::SampleType::UNIFORM);
		Model_types::SampleType metallicType = reqSettings.metallicTex3D ? Model_types::SampleType::TEXTURE_3D : Model_types::SampleType::UNIFORM;
		Model_types::SampleType roughnessType = reqSettings.roughnessTex3D ? Model_types::SampleType::TEXTURE_3D : Model_types::SampleType::UNIFORM;

		if (program) {

			for (unsigned int j = 0u; j < 16u; j++) program->markUnitAsOccupied(j, false);
			if (mat.baseTex) program->markUnitAsOccupied(ALBEDO_TEXTURE_UNIT, true);
			if (mat.normalTex) program->markUnitAsOccupied(NORMAL_TEXTURE_UNIT, true);
			if (mat.metallicRoughnessTex) program->markUnitAsOccupied(METALLIC_ROUGHNESS_TEXTURE_UNIT, true);
			for (unsigned int j = 0u; j < scene.getNumPointLights(); j++) program->markUnitAsOccupied(8u + j, true);
			if (scene.getOverheadShadowFarPlane() > 0.0f) program->markUnitAsOccupied(14u, true);
			if (scene.getOverheadOuterShadowFarPlane() > 0.0f) program->markUnitAsOccupied(15u, true);
			
			ModelFormat format;
			format.hasAlbedoMap = mat.baseTex;
			format.hasMetallicRoughnessMap = mat.metallicRoughnessTex;
			format.hasNormalMap = mat.normalTex;
			format.hasShadowMap = scene.getNumPointLights() || (scene.getOverheadShadowFarPlane() > 0.0f);
			format.isAnimated = isAnimated();

			program->prepareForUse(format);
			UniformTable& ut = program->getUniformTable();

			ut.set("isInstanced", (instances) ? 1 : 0);
			ut.set("_color", customColor ? (upscale<vec4>(albedoColor) + vec4(0.0f, 0.0f, 0.0f, 1.0f)) : mat.baseColor);
			ut.set("metallic", metallic);
			ut.set("roughness", roughness);

			if (format.hasShadowMap) {

				for (unsigned int j = 0u; j < 6u; j++) ut.setElement("_hasShadowSampler", j, (j < scene.getNumPointLights()) ? 1 : 0);
				ut.setElement("_overheadPVMatrix", 0u, scene.getOverheadShadowPVMatrix());
				ut.setElement("_overheadFarPlane", 0u, scene.getOverheadShadowFarPlane());
				ut.setElement("_bboxStart", 0u, scene.getOverheadShadowBoundingBox().start);
				ut.setElement("_bboxEnd", 0u, scene.getOverheadShadowBoundingBox().end);
				ut.setElement("_overheadPVMatrix", 1u, scene.getOverheadOuterShadowPVMatrix());
				ut.setElement("_overheadFarPlane", 1u, scene.getOverheadOuterShadowFarPlane());
				ut.setElement("_bboxStart", 1u, scene.getOverheadOuterShadowBoundingBox().start);
				ut.setElement("_bboxEnd", 1u, scene.getOverheadOuterShadowBoundingBox().end);
				ut.set("overheadLightDirection", scene.getOverheadLightDirection());
				ut.set("overheadLightColor", scene.getOverheadLightColor());

			}

			ut.update(program->getProgram(format));
			program->getProgram(format).use();

		}
		else {

			int idx = getProgramIndex((bool)modelData[thisModelDataIndex].boneNodes, scene.hasBackground(), albedoType, normalType, (mat.metallicRoughnessTex > 0u), metallicType, roughnessType);
			compileProgram(idx);

			PBR_uniforms[idx]->set("metallic", metallic);
			PBR_uniforms[idx]->set("roughness", roughness);
			PBR_uniforms[idx]->set("ao", ao);
			PBR_uniforms[idx]->set("shadowOcclusion", shadowOcclusion);
			PBR_uniforms[idx]->set("albedoStretch", albedoStretch);
			PBR_uniforms[idx]->set("metallicStretch", metallicStretch);
			PBR_uniforms[idx]->set("roughnessStretch", roughnessStretch);
			PBR_uniforms[idx]->set("normalStretch", normalStretch);
			PBR_uniforms[idx]->set("inColor", customColor ? (upscale<vec4>(albedoColor) + vec4(0.0f, 0.0f, 0.0f, 1.0f)) : mat.baseColor);
			PBR_uniforms[idx]->set("bgColor", scene.getBackgroundColor());
			PBR_uniforms[idx]->set("bgBrightness", scene.getBackgroundBrightness());
			for (unsigned int j = 0u; j < 6u; j++) PBR_uniforms[idx]->setElement("hasShadowSampler", j, (j < scene.getNumPointLights()) ? 1 : 0);
			PBR_uniforms[idx]->setElement("overheadPVMatrix", 0u, scene.getOverheadShadowPVMatrix());
			PBR_uniforms[idx]->setElement("overheadFarPlane", 0u, scene.getOverheadShadowFarPlane());
			PBR_uniforms[idx]->setElement("bboxStart", 0u, scene.getOverheadShadowBoundingBox().start);
			PBR_uniforms[idx]->setElement("bboxEnd", 0u, scene.getOverheadShadowBoundingBox().end);
			PBR_uniforms[idx]->setElement("overheadPVMatrix", 1u, scene.getOverheadOuterShadowPVMatrix());
			PBR_uniforms[idx]->setElement("overheadFarPlane", 1u, scene.getOverheadOuterShadowFarPlane());
			PBR_uniforms[idx]->setElement("bboxStart", 1u, scene.getOverheadOuterShadowBoundingBox().start);
			PBR_uniforms[idx]->setElement("bboxEnd", 1u, scene.getOverheadOuterShadowBoundingBox().end);
			PBR_uniforms[idx]->set("overheadLightDirection", scene.getOverheadLightDirection());
			PBR_uniforms[idx]->set("overheadLightColor", scene.getOverheadLightColor());
			PBR_uniforms[idx]->set("isInstanced", (instances) ? 1 : 0);
			PBR_uniforms[idx]->update();
			PBR_programs[idx]->use();

		}

		glBindVertexArray(modelData[thisModelDataIndex].vaos[i]);
		if (instances) glDrawElementsInstanced(GL_TRIANGLES, modelData[thisModelDataIndex].numElements[i], GL_UNSIGNED_INT, nullptr, instances->getLength());
		else glDrawElements(GL_TRIANGLES, modelData[thisModelDataIndex].numElements[i], GL_UNSIGNED_INT, nullptr);

	}

}

void GL::Model::drawShadow(GL::mat4 PV, GL::mat4 model, GL::Scene& scene) {

	ModelInstanceBuffer* instances = nullptr;
	if (shouldUseInstances) instances = instancesBuf;

	initUBOs();
	updateUBOs(PV, model, mat3(), scene, true);

	if (instancesBuf) { instancesBuf->bind(); instancesBuf->update(); }

	for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numMeshes; i++) {

		glBindVertexArray(modelData[thisModelDataIndex].vaos_shadow[i]);
		if (instances) glDrawElementsInstanced(GL_TRIANGLES, modelData[thisModelDataIndex].numElements[i], GL_UNSIGNED_INT, nullptr, instances->getLength());
		else glDrawElements(GL_TRIANGLES, modelData[thisModelDataIndex].numElements[i], GL_UNSIGNED_INT, nullptr);

	}

}

GL::Model::~Model() {
		
	modelData[thisModelDataIndex].referenceCount--;
	if (modelData[thisModelDataIndex].referenceCount == 0u) {
		
		modelData[thisModelDataIndex].name = nullptr;

		if (modelData[thisModelDataIndex].mats) {

			for (int i = 0; i < modelData[thisModelDataIndex].numMats; i++) {

				if (modelData[thisModelDataIndex].mats[i].baseTex) glDeleteTextures(1, &modelData[thisModelDataIndex].mats[i].baseTex);
				if (modelData[thisModelDataIndex].mats[i].normalTex) glDeleteTextures(1, &modelData[thisModelDataIndex].mats[i].normalTex);
				if (modelData[thisModelDataIndex].mats[i].metallicRoughnessTex) glDeleteTextures(1, &modelData[thisModelDataIndex].mats[i].metallicRoughnessTex);

			}

			delete[] modelData[thisModelDataIndex].mats;
			delete[] modelData[thisModelDataIndex].matFormats;

		}

		if (modelData[thisModelDataIndex].vaos) {

			for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numMeshes; i++) if (modelData[thisModelDataIndex].vaos[i]) {

				glDeleteVertexArrays(1, &modelData[thisModelDataIndex].vaos[i]);
				glDeleteVertexArrays(1, &modelData[thisModelDataIndex].vaos_shadow[i]);
				glDeleteBuffers(1, &modelData[thisModelDataIndex].vbos[i]);
				glDeleteBuffers(1, &modelData[thisModelDataIndex].ebos[i]);

			}

			delete[] modelData[thisModelDataIndex].vaos;
			delete[] modelData[thisModelDataIndex].vaos_shadow;
			delete[] modelData[thisModelDataIndex].vbos;
			delete[] modelData[thisModelDataIndex].ebos;
			delete[] modelData[thisModelDataIndex].numElements;
			delete[] modelData[thisModelDataIndex].matIndices;

		}

		if (modelData[thisModelDataIndex].boneNodes) {

			for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numBoneNodes; i++) {

				if (modelData[thisModelDataIndex].boneNodes[i].childIndices) delete[] modelData[thisModelDataIndex].boneNodes[i].childIndices;
				if (modelData[thisModelDataIndex].boneNodes[i].animations) {

					for (unsigned int j = 0u; j < modelData[thisModelDataIndex].numAnimations; j++) if (modelData[thisModelDataIndex].boneNodes[i].animations[j]) {

						if (modelData[thisModelDataIndex].boneNodes[i].animations[j]->translations) delete[] modelData[thisModelDataIndex].boneNodes[i].animations[j]->translations;
						if (modelData[thisModelDataIndex].boneNodes[i].animations[j]->rots) delete[] modelData[thisModelDataIndex].boneNodes[i].animations[j]->rots;
						if (modelData[thisModelDataIndex].boneNodes[i].animations[j]->scalings) delete[] modelData[thisModelDataIndex].boneNodes[i].animations[j]->scalings;

						delete[] modelData[thisModelDataIndex].boneNodes[i].animations[j];

					}

					delete[] modelData[thisModelDataIndex].boneNodes[i].animations;

				}

			}

			delete[] modelData[thisModelDataIndex].boneNodes;

		}

		if (modelData[thisModelDataIndex].animationData) delete[] modelData[thisModelDataIndex].animationData;

	}

}

#define _GL_Model_loadMaterialData(matType, numComps, unit, format) \
idx = rbf.read<int>(); \
w = rbf.read<unsigned int>(); h = rbf.read<unsigned int>(); \
\
if (idx >= 0) modelData[thisModelDataIndex].mats[i].matType ## Tex = modelData[thisModelDataIndex].mats[idx].matType ## Tex; \
else if (w * h != 0u) { \
	\
	char* texData = new char[w * h * numComps]; \
	for (unsigned int i = 0u; i < w * h * numComps; i++) texData[i] = rbf.readByte(); \
	\
	GLuint tex; glGenTextures(1, &tex); \
	glActiveTexture(GL_TEXTURE0 + unit); \
	glBindTexture(GL_TEXTURE_2D, tex); \
	\
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ ## format ## 16F, w, h, 0, GL_ ## format, GL_UNSIGNED_BYTE, texData); \
	glGenerateMipmap(GL_TEXTURE_2D); \
	\
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); \
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); \
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); \
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); \
	\
	delete[] texData; \
	modelData[thisModelDataIndex].mats[i].matType ## Tex = tex; \
	\
} \
else modelData[thisModelDataIndex].mats[i].matType ## Tex = 0u;

void GL::Model::loadMaterials(ReadBinaryFile& rbf) {

	modelData[thisModelDataIndex].numMats = rbf.read<unsigned int>();
	if (modelData[thisModelDataIndex].numMats) {

		modelData[thisModelDataIndex].mats = new Model_types::Material[modelData[thisModelDataIndex].numMats];
		modelData[thisModelDataIndex].matFormats = new Model_types::MaterialFormat[modelData[thisModelDataIndex].numMats];

		for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numMats; i++) {

			modelData[thisModelDataIndex].matFormats[i] = (Model_types::MaterialFormat)rbf.read<unsigned int>();
			for (int j = 0; j < 4; j++) modelData[thisModelDataIndex].mats[i].baseColor[j] = rbf.read<float>();

			unsigned int w, h; int idx;
			_GL_Model_loadMaterialData(base, 4, 0u, RGBA);
			_GL_Model_loadMaterialData(normal, 3, 2u, RGB);
			_GL_Model_loadMaterialData(metallicRoughness, 2, 1u, RG);

		}
	}
}

#define _GL_Model_loadAnimationData(animationType, AnimationType, numComps) \
animation.num ## AnimationType = rbf.read<unsigned int>(); \
if (animation.num ## AnimationType) { \
	\
	animation.animationType ## Times = new float[animation.num ## AnimationType]; \
	animation.animationType ## s = new vec ## numComps[animation.num ## AnimationType]; \
	\
	for (unsigned int k = 0u; k < animation.num ## AnimationType; k++) { \
		\
		animation.animationType ## Times[k] = rbf.read<float>(); \
		for (unsigned int l = 0u; l < numComps; l++) animation.animationType ## s[k][l] = rbf.read<float>(); \
		\
	} \
	\
}

void GL::Model::loadBoneNodes(ReadBinaryFile& rbf) {

	modelData[thisModelDataIndex].numAnimations = rbf.read<unsigned int>();
	if (modelData[thisModelDataIndex].numAnimations) {

		if (isPhysicsModel) throw Exception("A physics model cannot be animated.");

		modelData[thisModelDataIndex].numBones = rbf.read<unsigned int>();
		modelData[thisModelDataIndex].numBoneNodes = rbf.read<unsigned int>();

		modelData[thisModelDataIndex].animationData = new Model_types::AnimationData[modelData[thisModelDataIndex].numAnimations];

		for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numAnimations; i++) {

			modelData[thisModelDataIndex].animationData[i].duration = rbf.read<float>();
			modelData[thisModelDataIndex].animationData[i].TPS = rbf.read<float>();

		}

		modelData[thisModelDataIndex].boneNodes = new Model_types::BoneNode[modelData[thisModelDataIndex].numBoneNodes];
		
		for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numBoneNodes; i++) {

			Model_types::BoneNode& boneNode = modelData[thisModelDataIndex].boneNodes[i];
			
			boneNode.glIndex = rbf.read<unsigned int>();
			boneNode.numChildren = rbf.read<unsigned int>();
			bool hasAnimations = rbf.read<bool>();
			for (int x = 0; x < 4; x++) for (int y = 0; y < 4; y++) boneNode.trans[x][y] = rbf.read<float>();
			for (int x = 0; x < 4; x++) for (int y = 0; y < 4; y++) boneNode.offset[x][y] = rbf.read<float>();

			if (boneNode.numChildren) {

				boneNode.childIndices = new unsigned int[boneNode.numChildren];
				for (unsigned int j = 0u; j < boneNode.numChildren; j++) boneNode.childIndices[j] = rbf.read<unsigned int>();

			}
			else boneNode.childIndices = nullptr;

			if (hasAnimations) {

				boneNode.animations = new Model_types::Animation*[modelData[thisModelDataIndex].numAnimations];
				for (unsigned int j = 0u; j < modelData[thisModelDataIndex].numAnimations; j++) {

					bool animationExists = rbf.read<bool>();
					if (animationExists) {

						boneNode.animations[j] = new Model_types::Animation;
						Model_types::Animation& animation = *(boneNode.animations[j]);

						_GL_Model_loadAnimationData(scaling, Scalings, 3);
						_GL_Model_loadAnimationData(rot, Rots, 4);
						_GL_Model_loadAnimationData(translation, Translations, 3);

					}
					else boneNode.animations[j] = nullptr;

				}

			}
			else boneNode.animations = nullptr;

		}
	}
	else for (int i = 0; i < 3; i++) {
		
		modelData[thisModelDataIndex].bbox.start[i] = rbf.read<float>();
		modelData[thisModelDataIndex].bbox.end[i] = rbf.read<float>();
		
	}

}

void GL::Model::loadMeshes(ReadBinaryFile& rbf) { _GL_Model_loadMeshes(,,); }

void GL::Model::updateUBOs(GL::mat4 PV, GL::mat4 modelMatrix, GL::mat3 normalMatrix, GL::Scene& scene, bool drawingShadow) {

	PBR_commonUniforms->set("trans", PV);
	PBR_commonUniforms->set("modelMat", modelMatrix);
	for (unsigned int j = 0u; j < 16u; j++) PBR_commonUniforms->setElement("lightPositions", j, scene.getLightPosition(j));
	
	if (!drawingShadow) {

		PBR_commonUniforms->set("normalMat", normalMatrix);
		PBR_commonUniforms->set("camPos", scene.getCameraPosition());
		for (unsigned int j = 0u; j < 16u; j++) PBR_commonUniforms->setElement("lightColors", j, scene.getLightColor(j));

	}

	PBR_commonUniforms->update();

	if (modelData[thisModelDataIndex].boneNodes && modelData[thisModelDataIndex].numAnimations) {
	
		if (isAnimationPlaying) {
		
			t = t0 + (float)timer.time() * modelData[thisModelDataIndex].animationData[animationIndex].TPS;
			float duration = modelData[thisModelDataIndex].animationData[animationIndex].duration;

			if (t >= duration) {
			
				if (isAnimationLooped) {
					
					float k = std::floor(t / duration);
					t0 = t - k * duration;
					t = t0;
					timer.reset();
				
				}
				else {
				
					t = duration;
					isAnimationPlaying = false;
					isAnimationFinished = true;
				
				}
			}
		}
	
		updateModelMatrices(0u, mat4());
	
		for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numBoneNodes; i++)
		
			if (modelData[thisModelDataIndex].boneNodes[i].glIndex < 64u) {
			
				PBR_animationMatrices->setElement("boneModelMatrices", modelData[thisModelDataIndex].boneNodes[i].glIndex, modelData[thisModelDataIndex].boneNodes[i].modelMatrix);
				PBR_animationMatrices->setElement("boneNormalMatrices", modelData[thisModelDataIndex].boneNodes[i].glIndex, modelData[thisModelDataIndex].boneNodes[i].normalMatrix);
			
			}
	
		PBR_animationMatrices->update();
	
	}

}

void GL::Model::updateModelMatrices(unsigned int idx, GL::mat4 globalTransform) {
	
	Model_types::BoneNode& boneNode = modelData[thisModelDataIndex].boneNodes[idx];
	
	mat4 localTransform = boneNode.trans;
	if (isAnimationPlaying && boneNode.animations) if (boneNode.animations[animationIndex]) localTransform = boneNode.animations[animationIndex]->getMatrix(t);
	globalTransform = globalTransform * localTransform;

	boneNode.modelMatrix = globalTransform * boneNode.offset;
	boneNode.normalMatrix = transpose(inverse(mat3(boneNode.modelMatrix)));

	for (unsigned int i = 0u; i < boneNode.numChildren; i++) updateModelMatrices(boneNode.childIndices[i], globalTransform);

}

int GL::Model::getProgramIndex(bool isAnimated, bool hasSkybox, GL::Model_types::SampleType albedo, GL::Model_types::SampleType normal, bool hasMetallicRoughnessTex, GL::Model_types::SampleType metallic, GL::Model_types::SampleType roughness) {

	int metallicRoughness_coefficient = (hasMetallicRoughnessTex) ? 9 : _GL_Model_programBase_metallic * (int)metallic + _GL_Model_programBase_roughness * (int)roughness;
	return (
		_GL_Model_programBase_animated * (int)isAnimated +
		_GL_Model_programBase_skybox * (int)hasSkybox +
		_GL_Model_programBase_albedo * (int)albedo +
		_GL_Model_programBase_normal * (int)normal +
		_GL_Model_programBase_metallicRoughness * metallicRoughness_coefficient
	);

}

int GL::Model::getProgramIndex(unsigned int materialIndex, bool skybox) {
	
	return (
		_GL_Model_programBase_animated * (modelData[thisModelDataIndex].boneNodes != nullptr) +
		_GL_Model_programBase_skybox * skybox +
		_GL_Model_programBase_albedo * (modelData[thisModelDataIndex].mats[materialIndex].baseTex > 0u) +
		_GL_Model_programBase_normal * (modelData[thisModelDataIndex].mats[materialIndex].normalTex > 0u) +
		_GL_Model_programBase_metallicRoughness * ((modelData[thisModelDataIndex].mats[materialIndex].metallicRoughnessTex > 0u) ? _GL_Model_numOptions_metallicRoughness - 1 : 0)
	);

}

#define _GL_Model_extractCoefficient(name) \
unsigned int name ## Mode = tempIdx / _GL_Model_programBase_ ## name; tempIdx %= _GL_Model_programBase_ ## name;

void GL::Model::initUBOs() {

	if (!PBR_commonUniforms) {

		PBR_commonUniforms = new UniformBufferTable(0u);
		PBR_commonUniforms->init(
			"modelMat", UniformType::MAT4, 1,
			"normalMat", UniformType::MAT3, 1,
			"lightPositions", UniformType::VEC3, 16,
			"lightColors", UniformType::VEC3, 16,
			"trans", UniformType::MAT4, 1,
			"camPos", UniformType::VEC3, 1
		);

	}

	if (!PBR_animationMatrices) {

		PBR_animationMatrices = new UniformBufferTable(1u);
		PBR_animationMatrices->init(
			"boneModelMatrices", UniformType::MAT4, 64,
			"boneNormalMatrices", UniformType::MAT3, 64
		);

	}

}

void GL::Model::compileProgram(unsigned int idx) {
	
	static const char* vs_source[5];
	static const char* fs_source[7];

	if (PBR_programs[idx]) return;
	
	unsigned int tempIdx = idx;
	_GL_Model_extractCoefficient(animated);
	_GL_Model_extractCoefficient(skybox);
	_GL_Model_extractCoefficient(albedo);
	_GL_Model_extractCoefficient(normal);
	_GL_Model_extractCoefficient(metallicRoughness);

	unsigned int usesTextures = (unsigned int)(
		albedoMode == 1u ||
		normalMode == 1u || 
		metallicRoughnessMode / _GL_Model_programBase_roughness == 1u || 
		metallicRoughnessMode % _GL_Model_programBase_roughness == 1u || 
		metallicRoughnessMode == _GL_Model_numOptions_metallicRoughness - 1
	);

	initUBOs();
	
	unsigned int hasNormalMap = (normalMode == 1u);
	unsigned int vsIdx = 2u * animatedMode + hasNormalMap;

	if (!PBR_vertShaders[vsIdx]) {

		vs_source[0] = "#version 430 core\n";
		vs_source[1] = PBR_vert_variable_code[animatedMode];
		vs_source[2] = PBR_vert_variable_code[2u + hasNormalMap];
		vs_source[3] = PBR_vert_variable_code[4u + usesTextures];
		vs_source[4] = PBR_vert_base_code;

		PBR_vertShaders[vsIdx] = new ShaderLoader(ShaderType::VERTEX);
		PBR_vertShaders[vsIdx]->init((char**)vs_source, 5);

	}

	unsigned int fsIdx = idx % _GL_Model_numFragShaders;

	if (!PBR_fragShaders[fsIdx]) {

		fs_source[0] = "#version 430 core\n";
		fs_source[1] = PBR_frag_variable_code[_GL_Model_indexOffset_skybox + skyboxMode];
		fs_source[2] = PBR_frag_variable_code[_GL_Model_indexOffset_albedo + albedoMode];
		fs_source[3] = PBR_frag_variable_code[_GL_Model_indexOffset_normal + normalMode];
		fs_source[4] = PBR_frag_variable_code[_GL_Model_indexOffset_metallicRoughness + metallicRoughnessMode];
		fs_source[5] = PBR_frag_variable_code[_GL_Model_indexOffset_usesTextures + usesTextures];
		fs_source[6] = PBR_frag_base_code;

		PBR_fragShaders[fsIdx] = new ShaderLoader(ShaderType::FRAGMENT);
		PBR_fragShaders[fsIdx]->init((char**)fs_source, 7);

	}

	PBR_programs[idx] = new Program();
	PBR_programs[idx]->init(*(PBR_vertShaders[vsIdx]), *(PBR_fragShaders[fsIdx]));

	PBR_uniforms[idx] = new UniformTable(*PBR_programs[idx]);
	PBR_uniforms[idx]->init(

		"inColor", UniformType::VEC4, 1,
		"albedoSampler", UniformType::INT, 1,

		"normalSampler", UniformType::INT, 1,

		"metallic", UniformType::FLOAT, 1,
		"roughness", UniformType::FLOAT, 1,
		"metallicRoughnessSampler", UniformType::INT, 1,

		"bgColor", UniformType::VEC3, 1,
		"bgBrightness", UniformType::FLOAT, 1,
		"irradianceSampler", UniformType::INT, 1,
		"specularSampler", UniformType::INT, 1,
		"BRDFSampler", UniformType::INT, 1,
		
		"metallicSampler", UniformType::INT, 1,
		"roughnessSampler", UniformType::INT, 1,

		"albedoStretch", UniformType::FLOAT, 1,
		"metallicStretch", UniformType::FLOAT, 1,
		"roughnessStretch", UniformType::FLOAT, 1,
		"normalStretch", UniformType::FLOAT, 1,

		"ao", UniformType::FLOAT, 1,
		"shadowOcclusion", UniformType::FLOAT, 1,

		"shadowSamplers", UniformType::INT, 6,
		"hasShadowSampler", UniformType::INT, 6,

		"overheadShadowSampler", UniformType::INT, 2,
		"overheadPVMatrix", UniformType::MAT4, 2,
		"overheadFarPlane", UniformType::FLOAT, 2,
		"bboxStart", UniformType::VEC3, 2,
		"bboxEnd", UniformType::VEC3, 2,
		"overheadLightDirection", UniformType::VEC3, 1,
		"overheadLightColor", UniformType::VEC3, 1,

		"isInstanced", UniformType::INT, 1

	);
	PBR_uniforms[idx]->set<int>("albedoSampler", 0);
	PBR_uniforms[idx]->set<int>("metallicRoughnessSampler", 1);
	PBR_uniforms[idx]->set<int>("normalSampler", 2);
	PBR_uniforms[idx]->set<int>("irradianceSampler", 3);
	PBR_uniforms[idx]->set<int>("specularSampler", 4);
	PBR_uniforms[idx]->set<int>("BRDFSampler", 5);
	PBR_uniforms[idx]->set<int>("metallicSampler", 6);
	PBR_uniforms[idx]->set<int>("roughnessSampler", 7);
	for (unsigned int i = 0u; i < 6u; i++) { 
		
		PBR_uniforms[idx]->setElement<int>("shadowSamplers", i, (int)(8u + i));
		PBR_uniforms[idx]->setElement<int>("hasShadowSampler", i, 0);
		
	}
	for (unsigned int i = 0u; i < 2u; i++) PBR_uniforms[idx]->setElement<int>("overheadShadowSampler", i, (int)(14u + i));

}

std::vector<GL::Model_types::ModelData> GL::Model::modelData;

GL::ShaderLoader* GL::Model::PBR_vertShaders[];
GL::ShaderLoader* GL::Model::PBR_fragShaders[];
GL::Program* GL::Model::PBR_programs[];
GL::UniformTable* GL::Model::PBR_uniforms[];
GL::UniformBufferTable* GL::Model::PBR_commonUniforms = nullptr;
GL::UniformBufferTable* GL::Model::PBR_animationMatrices = nullptr;
bool GL::Model::PBR_initialized = false;

const char* GL::Model::PBR_vert_variable_code[] = {

	"\n",
	"\n#define ANIMATED\n",

	"\n",
	"\n#define NORMAL_MAP\n",

	"\n",
	"\n#define USES_TEXTURES\n"

};

#define _GL_Model_metallicMode0 "\n#define METALLIC_UNIFORM\n"
#define _GL_Model_metallicMode1 "\n#define METALLIC_2D_TEXTURE\n"
#define _GL_Model_metallicMode2 "\n#define METALLIC_3D_TEXTURE\n"

#define _GL_Model_roughnessMode0 "\n#define ROUGHNESS_UNIFORM\n"
#define _GL_Model_roughnessMode1 "\n#define ROUGHNESS_2D_TEXTURE\n"
#define _GL_Model_roughnessMode2 "\n#define ROUGHNESS_3D_TEXTURE\n"

const char* GL::Model::PBR_frag_variable_code[] = {

	"\n",
	"\n#define SKYBOX\n",

	"\n",
	"\n#define ALBEDO_2D_TEXTURE\n",
	"\n#define ALBEDO_3D_TEXTURE\n",

	"\n",
	"\n#define NORMAL_2D_TEXTURE\n",
	"\n#define NORMAL_3D_TEXTURE\n",

	_GL_Model_metallicMode0
	_GL_Model_roughnessMode0,
	_GL_Model_metallicMode1
	_GL_Model_roughnessMode0,
	_GL_Model_metallicMode2
	_GL_Model_roughnessMode0,
	_GL_Model_metallicMode0
	_GL_Model_roughnessMode1,
	_GL_Model_metallicMode1
	_GL_Model_roughnessMode1,
	_GL_Model_metallicMode2
	_GL_Model_roughnessMode1,
	_GL_Model_metallicMode0
	_GL_Model_roughnessMode2,
	_GL_Model_metallicMode1
	_GL_Model_roughnessMode2,
	_GL_Model_metallicMode2
	_GL_Model_roughnessMode2,
	"\n#define METALLIC_ROUGHNESS_SAMPLER\n",

	"\n",
	"\n#define USES_TEXTURES\n"

};

const char* GL::Model::PBR_vert_base_code = \
\
"layout(location = 0) in vec3 pos; \
layout(location = 1) in vec3 normal; \
\
layout(location = 2) in vec2 texCoords; \
out vec2 tCoords; \
\
\n#ifdef NORMAL_MAP\n \
layout(location = 3) in vec3 tangent; \
layout(location = 4) in vec3 bitangent; \
\n#endif\n \
\
\n#ifdef ANIMATED\n \
layout(location = 5) in ivec4 boneIndices; \
layout(location = 6) in vec4 boneWeights; \
\n#endif\n \
\
layout(std430, binding = 0) buffer modelMatrixInstances { mat4 modelMatrices[]; }; \
layout(std430, binding = 1) buffer normalMatrixInstances { mat3 normalMatrices[]; }; \
\
layout(std140, binding = 0) uniform PBR_inputs { \
	\
	mat4 modelMat; \
	mat3 normalMat; \
	vec3 lightPositions[16]; \
	vec3 lightColors[16]; \
	mat4 trans; \
	vec3 camPos; \
	\
}; \
\
uniform int isInstanced; \
mat4 modelMatrix = (isInstanced == 1) ? modelMatrices[gl_InstanceID] : modelMat; \
mat3 normalMatrix = (isInstanced == 1) ? normalMatrices[gl_InstanceID] : normalMat; \
\
\n#ifdef ANIMATED\n \
layout(std140, binding = 1) uniform boneData { \
	\
	mat4 boneModelMatrices[64]; \
	mat3 boneNormalMatrices[64]; \
	\
}; \
\n#endif\n \
\
out vec3 fragPos; \
out vec3 localFragPos; \
\
\n#ifdef NORMAL_MAP\n \
out mat3 TBN; \
\n#else\n \
out vec3 N; \
\n#endif\n \
\
void main() { \
	\
	tCoords = texCoords; \
	\
	\n#ifdef ANIMATED\n \
	mat4 boneModelMatrix = mat4(0.0f); \
	mat3 boneNormalMatrix = mat3(0.0f); \
	\
	for (uint i = 0u; i < 4u; i++) { \
		\
		boneModelMatrix += boneModelMatrices[boneIndices[i]] * boneWeights[i]; \
		boneNormalMatrix += boneNormalMatrices[boneIndices[i]] * boneWeights[i]; \
		\
	} \
	\
	mat4 finalModelMatrix = modelMatrix * boneModelMatrix; \
	mat3 finalNormalMatrix = normalMatrix * boneNormalMatrix; \
	\n#else\n \
	mat4 finalModelMatrix = modelMatrix; \
	mat3 finalNormalMatrix = normalMatrix; \
	\n#endif\n \
	\
	\n#ifdef NORMAL_MAP\n \
	TBN = finalNormalMatrix * mat3(tangent, bitangent, normal); \
	\n#else\n \
	N = normalize(finalNormalMatrix * normal); \
	\n#endif\n \
	\
	vec4 worldSpace = finalModelMatrix * vec4(pos, 1.0f); \
	fragPos = worldSpace.xyz; \
	localFragPos = pos; \
	gl_Position = trans * worldSpace; \
	\
}";

// Full credit to learnopengl.com for these PBR lighting formulas
const char* GL::Model::PBR_frag_base_code = \
\
"in vec3 fragPos; \
in vec3 localFragPos; \
\
in vec2 tCoords; \
\
layout(std140, binding = 0) uniform PBR_inputs { \
	\
	mat4 modelMat; \
	mat3 normalMat; \
	vec3 lightPositions[16]; \
	vec3 lightColors[16]; \
	mat4 trans; \
	vec3 camPos; \
	\
}; \
\
\n#ifdef SKYBOX\n \
uniform samplerCube irradianceSampler; \
uniform samplerCube specularSampler; \
\n#else\n \
uniform vec3 bgColor; \
\n#endif\n \
\
\n#ifdef ALBEDO_2D_TEXTURE\n \
uniform sampler2D albedoSampler; \
\n#elif defined ALBEDO_3D_TEXTURE\n \
uniform sampler3D albedoSampler; \
uniform float albedoStretch; \
\n#else\n \
uniform vec4 inColor; \
\n#endif\n \
\
\n#ifdef NORMAL_2D_TEXTURE\n \
in mat3 TBN; \
uniform sampler2D normalSampler; \
\n#elif defined NORMAL_3D_TEXTURE\n \
in vec3 N; \
uniform sampler3D normalSampler; \
uniform float normalStretch; \
\n#else\n \
in vec3 N; \
\n#endif\n \
\
\n#ifdef METALLIC_ROUGHNESS_SAMPLER\n \
uniform sampler2D metallicRoughnessSampler; \
\n#endif\n \
\
\n#ifdef METALLIC_UNIFORM\n \
uniform float metallic; \
\n#elif defined METALLIC_2D_TEXTURE\n \
uniform sampler2D metallicSampler; \
\n#elif defined METALLIC_3D_TEXTURE\n \
uniform sampler3D metallicSampler; \
uniform float metallicStretch; \
\n#endif\n \
\
\n#ifdef ROUGHNESS_UNIFORM\n \
uniform float roughness; \
\n#elif defined ROUGHNESS_2D_TEXTURE\n \
uniform sampler2D roughnessSampler; \
\n#elif defined ROUGHNESS_3D_TEXTURE\n \
uniform sampler3D roughnessSampler; \
uniform float roughnessStretch; \
\n#endif\n \
\
uniform float ao; \
uniform float shadowOcclusion; \
\
uniform float bgBrightness; \
uniform sampler2D BRDFSampler; \
\
uniform samplerCube shadowSamplers[6]; \
uniform int hasShadowSampler[6]; \
\
uniform sampler2D overheadShadowSampler[2]; \
uniform mat4 overheadPVMatrix[2]; \
uniform float overheadFarPlane[2]; \
uniform vec3 bboxStart[2]; \
uniform vec3 bboxEnd[2]; \
uniform vec3 overheadLightDirection; \
uniform vec3 overheadLightColor; \
\
layout(location = 0) out vec4 fragColor; \
\
const float radianceMultiplier = 800.0f; \
const float overheadRadianceMultiplier = 0.01f; \
const float PI = 3.14159265359f; \
const float maxReflectionLod = 4.0f; \
const uint numLights = 16u; \
const float epsilon = 0.0002f; \
const float shadowDecay = 0.3f; \
const float offsetDist = 0.0013f; \
const int numSamples = 20; \
const float overheadSampleDist = 0.0004f; \
const float bufferValue = 1.001f; \
const float numOverheadSamples = 6.0f; \
const float overheadIncrement = 2.0f * overheadSampleDist / (numOverheadSamples - 1.0f); \
const float overheadShadowDecay = 4.0f; \
const float overheadCutoff = 0.6f; \
const float lightThreshold = 0.001f; \
\
const vec3 offsets[numSamples] = vec3[]( \
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), \
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1), \
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0), \
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1), \
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1) \
	); \
\
float getShadowValue(vec3 sampleVector, float dist, uint samplerIndex, float NdotL) { \
	\
	if (samplerIndex >= 6u) return 1.0f; \
	if (hasShadowSampler[samplerIndex] == 0) return 1.0f; \
	\
	float bias = clamp(0.005f * tan(acos(NdotL)), 0.0f, 0.01f); \
	\
	float totalShadowValue = 0.0f; \
	float minShadow, maxShadow; \
	\
	for (int i = 0; i < numSamples; i++) { \
		\
		float shadowMapDist = texture(shadowSamplers[samplerIndex], sampleVector + offsets[i] * offsetDist).r + epsilon * dist * bias; \
		float shadowValue = 1.0f - max(dist - shadowMapDist, 0.0f); \
		shadowValue = clamp(shadowValue * shadowDecay, 0.0f, 1.0f); \
		\
		totalShadowValue += shadowValue; \
		\
	} \
	\
	return totalShadowValue / float(numSamples); \
	\
} \
\
float getOverheadShadowValue_atIndex(uint i) { \
	\
	if (overheadFarPlane[i] > 0.0f) { \
		\
		float totalShadowValue = 0.0f; \
		float totalNumSamples = 0.0f; \
		\
		for (float x = -overheadSampleDist; x <= overheadSampleDist * bufferValue; x += overheadIncrement) \
			for (float y = -overheadSampleDist; y <= overheadSampleDist * bufferValue; y += overheadIncrement) { \
				\
				vec3 shadowNDC = clamp((overheadPVMatrix[i] * vec4(fragPos, 1.0f)).xyz * 0.5f + 0.5f, 0.0f, 1.0f); \
				float shadowMapDist = texture(overheadShadowSampler[i], shadowNDC.xy + vec2(x, y)).x; \
				float dist = shadowNDC.z; \
				totalShadowValue += (dist <= shadowMapDist) ? 1.0 : 0.0; \
				totalNumSamples += 1.0f; \
				\
		} \
		\
		return min(totalShadowValue / (float(totalNumSamples) * overheadCutoff), 1.0f); \
		\
	} \
	else return 1.0f; \
	\
} \
\
float getOverheadShadowValue() {  \
	\
	if (overheadFarPlane[1] == 0.0f) return getOverheadShadowValue_atIndex(0u); \
	\
	vec3 t = (fragPos - bboxStart[0]) / (bboxEnd[0] - bboxStart[0]); \
	t = 20.0f * min(t, 1.0f - t); \
	t = 1.0f - clamp(t, 0.0f, 1.0f); \
	float ts = max(t.x, max(t.y, t.z)); \
	\
	if (ts == 0.0f) return getOverheadShadowValue_atIndex(0u); \
	else if (ts == 1.0f) return getOverheadShadowValue_atIndex(1u); \
	else return mix(getOverheadShadowValue_atIndex(0u), getOverheadShadowValue_atIndex(1u), ts); \
	\
} \
\
float DistributionGGX(float NdotH, float roughness) { \
	\
	float a = roughness * roughness; \
	float a2 = a * a; \
	\
	float num = a2; \
	float denom = (NdotH * NdotH * (a2 - 1.0f) + 1.0f); \
	denom = PI * denom * denom; \
	\
	return num / denom; \
	\
} \
\
float GeometrySchlickGGX(float NdotV, float roughness) { \
	\
	float r = (roughness + 1.0f); \
	float k = (r * r) / 8.0f; \
	\
	float num = NdotV; \
	float denom = NdotV * (1.0f - k) + k; \
	\
	return num / denom; \
	\
} \
\
float GeometrySmith(float NdotV, float NdotL, float roughness) { \
	\
	float ggx2 = GeometrySchlickGGX(NdotV, roughness); \
	float ggx1 = GeometrySchlickGGX(NdotL, roughness); \
	\
	return ggx1 * ggx2; \
	\
} \
\
vec3 fresnelSchlick(float cosTheta, vec3 F0) { return F0 + (vec3(1.0f) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f); } \
\
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) { return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(max(1.0f - cosTheta, 0.0f), 5.0f); } \
\
\n#define addLightSource(L, radiance) \
\
vec3 H = normalize(V + L); \
\
float NdotH = clamp(dot(N, H), 0.0f, 1.0f); \
float HdotV = clamp(dot(H, V), 0.0f, 1.0f); \
\
float NDF = DistributionGGX(NdotH, roughness); \
float G = GeometrySmith(NdotV, NdotL, roughness); \
vec3 F = fresnelSchlick(HdotV, F0); \
\
vec3 kS = F; \
vec3 kD = vec3(1.0f) - kS; \
kD *= 1.0f - metallic; \
\
vec3 numerator = NDF * G * F; \
float denominator = 4.0f * NdotV * NdotL; \
vec3 specular = numerator / max(denominator, 0.1f); \
\
totalShadow += shadowValue; totalShadowSamples += 1.0f; \
Lo += (kD * albedo / PI + specular) * radiance * NdotL * shadowValue;\n \
\
vec3 calc_PBR_color(vec3 albedo, float metallic, float roughness, float ao, float shadowOcclusion, vec3 N, vec3 V, vec3 fragmentPosition) { \
	\
	float NdotV = clamp(dot(N, V), 0.0f, 1.0f); \
	\
	vec3 F0 = mix(vec3(0.04f), albedo, metallic); \
	float totalShadow = 0.0f; \
	float totalShadowSamples = 0.0f; \
	vec3 Lo = vec3(0.0f); \
	\
	for (uint i = 0u; i < numLights; i++) { \
		\
		if (dot(lightColors[i], lightColors[i]) < lightThreshold) continue; \
		\
		vec3 L = lightPositions[i] - fragmentPosition; \
		float distance = length(L); \
		L /= distance; \
		float NdotL = clamp(dot(N, L), 0.0f, 1.0f); \
		float shadowValue = getShadowValue(-L, distance, i, NdotL); \
		\
		float attenuation = radianceMultiplier / (distance * distance); \
		vec3 radiance = lightColors[i] * attenuation; \
		\
		addLightSource(L, radiance) \
		\
	} \
	\
	if (dot(overheadLightColor, overheadLightColor) >= lightThreshold) { \
		\
		float NdotL = clamp(dot(N, -overheadLightDirection), 0.0f, 1.0f); \
		float shadowValue = getOverheadShadowValue(); \
		addLightSource((-overheadLightDirection), overheadLightColor * overheadRadianceMultiplier) \
		\
	} \
	\
	\n#ifdef SKYBOX\n \
	vec3 irradiance = texture(irradianceSampler, N).rgb * bgBrightness; \
	vec3 specularBackground = textureLod(specularSampler, reflect(-V, N), roughness * maxReflectionLod).rgb * bgBrightness; \
	\n#else\n \
	vec3 irradiance = bgColor * bgBrightness; \
	vec3 specularBackground = bgColor * bgBrightness; \
	\n#endif\n \
	\
	vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness); \
	vec3 kS = F; \
	vec3 kD = 1.0f - kS; \
	kD *= 1.0f - metallic; \
	\
	vec3 diffuse = irradiance * albedo; \
	\
	vec2 envBRDF = texture(BRDFSampler, vec2(NdotV, roughness)).rg; \
	vec3 specular = specularBackground * (F * envBRDF.x + envBRDF.y); \
	\
	if (totalShadowSamples < 0.5f) totalShadow = 1.0f; \
	else totalShadow /= totalShadowSamples; \
	ao *= mix(totalShadow, 1.0f, shadowOcclusion); \
	\
	vec3 ambient = (kD * diffuse + specular) * ao; \
	return Lo + ambient; \
	\
} \
\
void main() { \
	\
	\n#ifdef ALBEDO_2D_TEXTURE\n \
	vec4 inColor = texture(albedoSampler, tCoords); \
	\n#elif defined ALBEDO_3D_TEXTURE\n \
	vec4 inColor = texture(albedoSampler, localFragPos * albedoStretch); \
	\n#endif\n \
	\
	\n#ifdef NORMAL_2D_TEXTURE\n \
	vec3 N_final = normalize(TBN * (2.0f * texture(normalSampler, tCoords).xyz - 1.0f)); \
	\n#elif defined NORMAL_3D_TEXTURE\n \
	vec3 N_final = normalize(N + texture(normalSampler, localFragPos * normalStretch).xyz); \
	\n#else\n \
	vec3 N_final = N; \
	\n#endif\n \
	\
	\n#ifdef METALLIC_ROUGHNESS_SAMPLER\n \
	vec2 metallicRoughness = texture(metallicRoughnessSampler, tCoords).rg; \
	float metallic = metallicRoughness.r; \
	float roughness = metallicRoughness.g; \
	\n#endif\n \
	\
	\n#ifdef METALLIC_2D_TEXTURE\n \
	float metallic = texture(metallicSampler, tCoords).r; \
	\n#elif defined METALLIC_3D_TEXTURE\n \
	float metallic = texture(metallicSampler, localFragPos * metallicStretch).r; \
	\n#endif\n \
	\
	\n#ifdef ROUGHNESS_2D_TEXTURE\n \
	float roughness = texture(roughnessSampler, tCoords).r; \
	\n#elif defined ROUGHNESS_3D_TEXTURE\n \
	float roughness = texture(roughnessSampler, localFragPos * roughnessStretch).r; \
	\n#endif\n \
	\
	vec3 albedo = pow(inColor.rgb, vec3(2.2f)); \
	vec3 V = normalize(camPos - fragPos); \
	\
	vec3 color = calc_PBR_color(albedo, metallic, roughness, ao, shadowOcclusion, N_final, V, fragPos); \
	color = color / (color + vec3(1.0f)); \
	color = pow(color, vec3(1.0f / 2.2f)); \
	fragColor = vec4(color, inColor.a); \
	\
}";
