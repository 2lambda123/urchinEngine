#include <scene/renderer3d/lighting/shadow/light/LightSplitShadowMap.h>
#include <scene/renderer3d/lighting/shadow/light/LightShadowMap.h>
#include <scene/renderer3d/lighting/shadow/filter/ModelProduceShadowFilter.h>

namespace urchin {

    LightSplitShadowMap::LightSplitShadowMap(const LightShadowMap* lightShadowMap) :
            lightShadowMap(lightShadowMap),
            updateShadowMapThreshold(ConfigService::instance().getFloatValue("shadow.updateShadowMapThreshold")),
            useSceneDependentProjection(ConfigService::instance().getBoolValue("shadow.useSceneDependentProjection")),
            shadowCasterReceiverBoxUpdated(false),
            modelsRequireUpdate(false) {

    }

    void LightSplitShadowMap::update(const Frustum<float>& splitFrustums, bool bForceUpdateAllShadowMaps) {
        AABBox<float> aabboxSceneIndependent = createSceneIndependentBox(splitFrustums, lightShadowMap->getLightViewMatrix());
        OBBox<float> obboxSceneIndependentViewSpace = lightShadowMap->getLightViewMatrix().inverse() * OBBox<float>(aabboxSceneIndependent);

        obboxModels.clear();
        lightShadowMap->getModelOctreeManager().getOctreeablesIn(obboxSceneIndependentViewSpace, obboxModels, ModelProduceShadowFilter());
        updateModels(obboxModels);

        if (useSceneDependentProjection) {
            AABBox<float> aabboxSceneDependent = buildSceneDependentBox(aabboxSceneIndependent, obboxSceneIndependentViewSpace);
            updateShadowCasterReceiverBox(aabboxSceneDependent, bForceUpdateAllShadowMaps);
        } else {
            updateShadowCasterReceiverBox(aabboxSceneIndependent, bForceUpdateAllShadowMaps);
        }
    }

    const AABBox<float> &LightSplitShadowMap::getShadowCasterReceiverBox() const {
        return shadowCasterReceiverBox;
    }

    const Matrix4<float>& LightSplitShadowMap::getLightProjectionMatrix() const {
        return lightProjectionMatrix;
    }

    /**
     * @return Models visible from light in frustum split
     */
    std::span<Model* const> LightSplitShadowMap::getModels() const {
        return models;
    }

    bool LightSplitShadowMap::needShadowMapUpdate() const {
        return shadowCasterReceiverBoxUpdated || modelsRequireUpdate;
    }

    /**
     * @return Box in light space containing shadow caster and receiver (scene independent)
     */
    AABBox<float> LightSplitShadowMap::createSceneIndependentBox(const Frustum<float>& splitFrustum, const Matrix4<float>& lightViewMatrix) const {
        ScopeProfiler sp(Profiler::graphic(), "sceneIndepBox");

        const Frustum<float>& frustumLightSpace = lightViewMatrix * splitFrustum;

        //determine point belonging to shadow caster/receiver box
        std::array<Point3<float>, 16> shadowReceiverAndCasterVertex;
        float nearCapZ = computeNearZForSceneIndependentBox(frustumLightSpace);
        for (std::size_t i = 0; i < 8; ++i) {
            const Point3<float>& frustumPoint = frustumLightSpace.getFrustumPoints()[i];

            //add shadow receiver points
            shadowReceiverAndCasterVertex[i * 2] = frustumPoint;

            //add shadow caster points
            shadowReceiverAndCasterVertex[i * 2 + 1] = Point3<float>(frustumPoint.X, frustumPoint.Y, nearCapZ);
        }

        //build shadow receiver/caster bounding box from points
        return AABBox<float>(shadowReceiverAndCasterVertex);
    }

    float LightSplitShadowMap::computeNearZForSceneIndependentBox(const Frustum<float>& splitFrustumLightSpace) const {
        float nearestPointFromLight = splitFrustumLightSpace.getFrustumPoints()[0].Z;
        for (unsigned int i = 1; i < 8; ++i) {
            if (splitFrustumLightSpace.getFrustumPoints()[i].Z > nearestPointFromLight) {
                nearestPointFromLight = splitFrustumLightSpace.getFrustumPoints()[i].Z;
            }
        }
        return nearestPointFromLight + lightShadowMap->getViewingShadowDistance();
    }

    /**
     * Build box in light space containing shadow caster and receiver (scene dependent)
     */
    AABBox<float> LightSplitShadowMap::buildSceneDependentBox(const AABBox<float>& aabboxSceneIndependent, const OBBox<float>& obboxSceneIndependentViewSpace) {
        ScopeProfiler sp(Profiler::graphic(), "sceneDepBox");

        unsigned int modelsCount = 0;
        AABBox<float> modelsAabbox = AABBox<float>::initMergeableAABBox();

        for (const auto& model : models) {
            if (model->getShadowClass() == Model::RECEIVER_ONLY) {
                continue;
            }

            if (model->getSplitAABBoxes().size() == 1) {
                modelsAabbox = modelsAabbox.merge(lightShadowMap->getLightViewMatrix() * model->getSplitAABBoxes()[0]);
                modelsCount++;
            } else {
                for (const auto& splitAABBox : model->getSplitAABBoxes()) {
                    if (obboxSceneIndependentViewSpace.toAABBox().collideWithAABBox(splitAABBox)) {
                        modelsAabbox = modelsAabbox.merge(lightShadowMap->getLightViewMatrix() * splitAABBox);
                        modelsCount++;
                    }
                }
            }
        }

        AABBox<float> aabboxSceneDependent(Point3<float>(0.0f, 0.0f, 0.0f), Point3<float>(0.0f, 0.0f, 0.0f));
        if (modelsCount > 0) {
            Point3<float> cutMin(
                    std::min(std::max(modelsAabbox.getMin().X, aabboxSceneIndependent.getMin().X), aabboxSceneIndependent.getMax().X),
                    std::min(std::max(modelsAabbox.getMin().Y, aabboxSceneIndependent.getMin().Y), aabboxSceneIndependent.getMax().Y),
                    aabboxSceneIndependent.getMin().Z); //shadow can be projected outside the box: value cannot be capped

            Point3<float> cutMax(
                    std::max(std::min(modelsAabbox.getMax().X, aabboxSceneIndependent.getMax().X), aabboxSceneIndependent.getMin().X),
                    std::max(std::min(modelsAabbox.getMax().Y, aabboxSceneIndependent.getMax().Y), aabboxSceneIndependent.getMin().Y),
                    std::max(std::min(modelsAabbox.getMax().Z, aabboxSceneIndependent.getMax().Z), aabboxSceneIndependent.getMin().Z));

            aabboxSceneDependent = AABBox<float>(cutMin, cutMax);
        }

        //avoid aabbox of size zero when there is no model or when all models are outside the aabboxSceneIndependent
        return AABBox<float>(aabboxSceneDependent.getMin() - LIGHT_BOX_MARGIN, aabboxSceneDependent.getMax() + LIGHT_BOX_MARGIN);
    }

    void LightSplitShadowMap::updateShadowCasterReceiverBox(const AABBox<float>& shadowCasterReceiverBox, bool forceUpdateAllShadowMap) {
        if (!forceUpdateAllShadowMap && areAlmostIdenticalAABBox(shadowCasterReceiverBox, this->shadowCasterReceiverBox)) {
            this->shadowCasterReceiverBoxUpdated = false;
        } else {
            this->shadowCasterReceiverBox = shadowCasterReceiverBox;
            this->lightProjectionMatrix = shadowCasterReceiverBox.toProjectionMatrix();

            this->shadowCasterReceiverBoxUpdated = true;
        }
    }

    bool LightSplitShadowMap::areAlmostIdenticalAABBox(const AABBox<float>& shadowCasterReceiverBox1, const AABBox<float>& shadowCasterReceiverBox2) const {
        float updateShadowMapSquareThreshold = updateShadowMapThreshold * updateShadowMapThreshold;

        return shadowCasterReceiverBox1.getMin().squareDistance(shadowCasterReceiverBox2.getMin()) < updateShadowMapSquareThreshold
               && shadowCasterReceiverBox1.getMax().squareDistance(shadowCasterReceiverBox2.getMax()) < updateShadowMapSquareThreshold;
    }

    /**
     * @models Models visible from light in frustum split
     */
    void LightSplitShadowMap::updateModels(const std::vector<Model*>& models) {
        modelsRequireUpdate = false;

        if (models != this->models) {
            modelsRequireUpdate = true;
            this->models = models;
        } else {
            for (const auto* model : models) {
                if (model->isMovingInOctree() || (model->isAnimated() && model->getActiveAnimation()->isAnimationAffectShadow())) {
                    modelsRequireUpdate = true;
                    break;
                }
            }
        }
    }
}
