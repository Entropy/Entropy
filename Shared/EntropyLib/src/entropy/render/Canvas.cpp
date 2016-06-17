#include "Canvas.h"

namespace entropy
{
	namespace render
	{
		//--------------------------------------------------------------
		Canvas::Canvas()
		{
			this->fboSettings.width = ofGetWidth();
			this->fboSettings.height = ofGetHeight();
			this->fboSettings.textureTarget = GL_TEXTURE_2D;
			this->fbo.allocate(this->fboSettings);

			this->fillWindow = false;
		}

		//--------------------------------------------------------------
		Canvas::~Canvas()
		{
			this->setFillWindow(false);
		}

		//--------------------------------------------------------------
		void Canvas::begin()
		{
			this->fbo.begin();
		}
		
		//--------------------------------------------------------------
		void Canvas::end()
		{
			this->fbo.end();
		}

		//--------------------------------------------------------------
		void Canvas::draw()
		{
			// TODO: Go through warps and fbo texture subsections and draw the whole thing.
			this->fbo.draw(0, 0);
		}

		//--------------------------------------------------------------
		float Canvas::getWidth() const
		{
			return this->fbo.getWidth();
		}

		//--------------------------------------------------------------
		float Canvas::getHeight() const
		{
			return this->fbo.getHeight();
		}

		//--------------------------------------------------------------
		void Canvas::setWidth(float width)
		{
			if (this->fbo.getWidth() == width) return;

			this->fboSettings.width = width;
			this->fbo.allocate(this->fboSettings);

			ofResizeEventArgs args;
			args.width = this->fbo.getWidth();
			args.height = this->fbo.getHeight();
			this->resizeEvent.notify(args);
		}

		//--------------------------------------------------------------
		void Canvas::setHeight(float height)
		{
			if (this->fbo.getHeight() == height) return;

			this->fboSettings.height = height;
			this->fbo.allocate(this->fboSettings);

			ofResizeEventArgs args;
			args.width = this->fbo.getWidth();
			args.height = this->fbo.getHeight();
			this->resizeEvent.notify(args);
		}

		//--------------------------------------------------------------
		bool Canvas::getFillWindow() const
		{
			return this->fillWindow;
		}
		
		//--------------------------------------------------------------
		void Canvas::setFillWindow(bool fillWindow)
		{
			if (this->fillWindow == fillWindow) return;

			this->fillWindow = fillWindow;
			if (this->fillWindow)
			{
				ofAddListener(ofEvents().windowResized, this, &Canvas::windowResized);

				// Force the first call to get in sync.
				ofResizeEventArgs args;
				args.width = ofGetWidth();
				args.height = ofGetHeight();
				this->windowResized(args);
			}
			else
			{
				ofRemoveListener(ofEvents().windowResized, this, &Canvas::windowResized);
			}
		}

		//--------------------------------------------------------------
		void Canvas::windowResized(ofResizeEventArgs & args)
		{
			if (this->fbo.getWidth() == args.width && this->fbo.getHeight() == args.height) return;

			this->fboSettings.width = args.width;
			this->fboSettings.height = args.height;
			this->fbo.allocate(this->fboSettings);

			this->resizeEvent.notify(args);
		}
	}
}