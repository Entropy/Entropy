#pragma once

#include "entropy/bubbles/Constants.h"
#include "entropy/geom/Sphere.h"
#include "entropy/scene/Base.h"

#ifdef COMPUTE_GL_2D
#include "entropy/bubbles/PoolGL2D.h"
#endif
#ifdef COMPUTE_GL_3D
#include "entropy/bubbles/PoolGL3D.h"
#endif
#ifdef COMPUTE_CL_2D
#include "CmbSceneCL2D.h"
#endif
#ifdef COMPUTE_CL_3D
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
			void resizeFront(ofResizeEventArgs & args) override;

			void update(double dt) override;
			void timelineBangFired(ofxTLBangEventArgs & args) override;

			void drawBackBase() override;
			void drawBackWorld() override;

			void drawFrontBase() override;
			void drawFrontWorld() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

		protected:
#ifdef COMPUTE_GL_2D
			entropy::bubbles::PoolGL2D pool2D;
#endif
#ifdef COMPUTE_GL_2D
			entropy::bubbles::PoolGL3D pool3D;
#endif
#ifdef COMPUTE_CL_2D
			entropy::bubbles::PoolCL2D pool2D;
#endif
#ifdef COMPUTE_CL_3D
			entropy::bubbles::PoolCL3D pool3D;
#endif

			geom::Sphere sphereGeom;
			ofTexture sphereTexture;
			ofShader sphereShader;

			ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			struct : ofParameterGroup
			{
				struct : ofParameterGroup
				{
					ofParameter<float> orientation{ "Orientation", 0.0f, 0.0f, 360.0f };
					ofParameter<float> maskMix{ "Mask Mix", 1.0f, 0.0f, 1.0f };

					PARAM_DECLARE("SphereExtra", orientation, maskMix);
				} sphere;
				
				PARAM_DECLARE("Bubbles", sphere);
			} parameters;
		};
	}
}