
#include "./RenderTarget.hpp"

bool GL::RenderTarget::isTexture() const { return isTex; }

bool GL::RenderTarget::isColorTarget() const { return isColor; }

bool GL::RenderTarget::isDepthTarget() const { return !isColor; }

bool GL::RenderTarget::isStencilTarget() const { return !isColor && hasStencil; }

bool GL::RenderTarget::isMultisampled() const { return isMulti; }

unsigned int GL::RenderTarget::width() const { return w; }

unsigned int GL::RenderTarget::height() const { return h; }