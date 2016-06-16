#pragma once

#include "ofxImGui.h"

namespace entropy
{
	namespace render
	{
		class Canvas
		{
		public:
			Canvas();
			~Canvas();

			void begin();
			void end();

			void draw();

			float getWidth() const;
			float getHeight() const;

			void setWidth(float width);
			void setHeight(float height);

			ofxImGui & getImGui();

			ofEvent<ofResizeEventArgs> resizeEvent;

		private:
			ofFbo fbo;
			ofFbo::Settings fboSettings;

			ofxImGui imGui;
		};
	}
}