#pragma once

#include "entropy/bubbles/Constants.h"
#include "entropy/geom/Sphere.h"
#include "entropy/scene/Base.h"

#ifdef COMPUTE_GL_2D
#include "CmbSceneGL2D.h"
#elif defined(COMPUTE_GL_3D)
#include "entropy/bubbles/PoolGL3D.h"
#elif defined(COMPUTE_CL_2D)
#include "CmbSceneCL2D.h"
#elif defined(COMPUTE_CL_3D)
#include "CmbSceneCL3D.h"
#endif

namespace entropy
{
	namespace scene
	{
		class Bubbles
			: public Base
		{
		public:
			string getName() const override 
			{
				return "entropy::scene::Bubbles";
			}

			Bubbles();
			~Bubbles();

			void init() override;

			void setup() override;
			void resizeBack(ofResizeEventArgs & args) override;

			void update(double dt) override;

			void drawBackWorld() override;
			void drawBackOverlay() override;

			void drawFrontWorld() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

		protected:
			void drawPool();

#ifdef COMPUTE_GL_2D
			entropy::bubbles::PoolGL2D pool;
#elif defined(COMPUTE_GL_3D)
			entropy::bubbles::PoolGL3D pool;
#elif defined(COMPUTE_CL_2D)
			entropy::bubbles::PoolCL2D pool;
#elif defined(COMPUTE_CL_3D)
			entropy::bubbles::PoolCL3D pool;
#endif

			geom::Sphere sphereGeom;
			ofTexture sphereTexture;

			ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			struct : ofParameterGroup
			{
				ofParameter<ofFloatColor> tintColor{ "Tint Color", ofFloatColor::white };
				
				PARAM_DECLARE("Bubbles", tintColor);
			} parameters;
		};
	}
}