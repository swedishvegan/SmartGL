#ifndef RENDERBUFFER_HPP
#define RENDERBUFFER_HPP

#include "./RenderTarget.hpp"

namespace GL {

	class Renderbuffer : public RenderTarget {
	public:

		void bind() const;

		GLuint getID() const;

	protected:

		GLuint ID;

		Renderbuffer(unsigned int width, unsigned int height, bool multisampled, ColorFormat colorFormat, DataType colorDataType, DepthStencilFormat depthStencilFormat, bool isColorBuffer);

		~Renderbuffer();

	};

	class ColorRenderbuffer : public Renderbuffer {
	public:

		ColorRenderbuffer(unsigned int width, unsigned int height, bool multisampled = false, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32);

		ColorFormat getColorFormat() const;

		DataType getInternalDataType() const;

	private:

		ColorFormat format;
		DataType dataType;

	};

	class DepthStencilRenderbuffer : public Renderbuffer {
	public:

		DepthStencilRenderbuffer(unsigned int width, unsigned int height, bool multisampled = false, DepthStencilFormat format = DepthStencilFormat::DEPTH_24);

		DepthStencilFormat getDepthStencilFormat() const;

	private:

		DepthStencilFormat format;

	};

}

#endif