
#include "./Model_types.hpp"

GL::vec4 GL::Model_types::Animation::interpolateSpherical(vec4 a, vec4 b, float t) { 
    
    float cosTheta = dot(a, b); 
    if (cosTheta >= 0.999f) return mix(a, b, t); 
    
    vec4 diff = b - a; 
    float theta = std::acos(cosTheta); 
    vec2 pq = inverse(mat2( 
        dot(a, a), dot(b, a), 
        dot(a, diff), dot(b, diff) 
    )) * vec2(std::cos(theta * t), std::cos(theta * (1.0f - t))); 
    
    return pq.x * a + pq.y * diff; 
    
}

GL::mat4 GL::Model_types::Animation::quaternionToMatrix(vec4 q) { 
			
    float a1 = 1.0f - 2.0f * (q.y * q.y + q.z * q.z); 
    float a2 = 2.0f * (q.x * q.y - q.z * q.w); 
    float a3 = 2.0f * (q.x * q.z + q.y * q.w); 
    float b1 = 2.0f * (q.x * q.y + q.z * q.w); 
    float b2 = 1.0f - 2.0f * (q.x * q.x + q.z * q.z); 
    float b3 = 2.0f * (q.y * q.z - q.x * q.w); 
    float c1 = 2.0f * (q.x * q.z - q.y * q.w); 
    float c2 = 2.0f * (q.y * q.z + q.x * q.w); 
    float c3 = 1.0f - 2.0f * (q.x * q.x + q.y * q.y); 
    
    return mat4( 
        a1, b1, c1, 0.0f, 
        a2, b2, c2, 0.0f, 
        a3, b3, c3, 0.0f, 
        0.0f, 0.0f, 0.0f, 1.0f 
    ); 
    
}

float GL::Model_types::Animation::normaliz(float t, float start, float end) { return (t - start) / (end - start); } 

unsigned int GL::Model_types::Animation::findTimestampIndex(float t, float* timestamps, unsigned int numTimestamps) { 
    
    unsigned int idx = 0u; 
    for (unsigned int i = 0u; i < numTimestamps; i++) if (timestamps[i] > t) { idx = i; break; } 
    return idx; 
    
}

GL::mat4 GL::Model_types::Animation::getMatrix(float t) { 
			
    unsigned int transIdx = findTimestampIndex(t, translationTimes, numTranslations); 
    unsigned int rotIdx = findTimestampIndex(t, rotTimes, numRots); 
    unsigned int scaleIdx = findTimestampIndex(t, scalingTimes, numScalings); 
    
    if (transIdx > 0u) transIdx--; 
    if (rotIdx > 0u) rotIdx--; 
    if (scaleIdx > 0u) scaleIdx--; 
    
    float transT = normaliz(t, translationTimes[transIdx], translationTimes[transIdx + 1u]); 
    float rotT = normaliz(t, rotTimes[rotIdx], rotTimes[rotIdx + 1u]); 
    float scaleT = normaliz(t, scalingTimes[scaleIdx], scalingTimes[scaleIdx + 1u]); 
    
    vec3 translation = mix(translations[transIdx], translations[transIdx + 1u], transT); 
    vec4 rot = interpolateSpherical(rots[rotIdx], rots[rotIdx + 1u], rotT); 
    vec3 scaling = mix(scalings[scaleIdx], scalings[scaleIdx + 1u], scaleT); 
    
    return translate(translation) * quaternionToMatrix(rot) * scale(scaling); 
    
} 