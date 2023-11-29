
#include "./ModelStructs.hpp"
#include "./../util/GL-math.hpp"

GL::BoundingBox::BoundingBox() : start(0.0f), end(0.0f) { }

GL::BoundingBox::BoundingBox(vec3 s, vec3 e) : start(min(s, e)), end(max(s, e)) { }

GL::BoundingBox GL::BoundingBox::operator + (BoundingBox bb) { return BoundingBox(min(start, bb.start), max(end, bb.end)); }

GL::BoundingBox GL::BoundingBox::operator * (BoundingBox bb) { return BoundingBox(max(start, bb.start), min(end, bb.end)); }