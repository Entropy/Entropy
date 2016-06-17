#pragma once

#include "ofMain.h"

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

			bool getFillWindow() const;
			void setFillWindow(bool fillWindow);

			ofEvent<ofResizeEventArgs> resizeEvent;

		protected:
			void windowResized(ofResizeEventArgs & args);

		private:
			ofFbo fbo;
			ofFbo::Settings fboSettings;

			bool fillWindow;
		};
	}
}