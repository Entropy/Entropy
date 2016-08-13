#pragma once

#include "entropy/scene/Base.h"
#include "entropy/cmb/Constants.h"

#ifdef COMPUTE_GL_2D
#include "CmbSceneGL2D.h"
#elif defined(COMPUTE_GL_3D)
#include "entropy/cmb/PoolGL3D.h"
#elif defined(COMPUTE_CL_2D)
#include "CmbSceneCL2D.h"
#elif defined(COMPUTE_CL_3D)
#include "CmbSceneCL3D.h"
#endif

namespace entropy
{
	namespace scene
	{
		class CMB
			: public Base
		{
		public:
			string getName() const override 
			{
				return "entropy::scene::CMB";
			}

			CMB();
			~CMB();

			void setup();
			void exit();
			void resize(ofResizeEventArgs & args);

			void update(double & dt);

			void drawBack();
			void drawWorld();
			void drawFront();

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			void drawPool();

#ifdef COMPUTE_GL_2D
			entropy::cmb::PoolGL2D pool;
#elif defined(COMPUTE_GL_3D)
			entropy::cmb::PoolGL3D pool;
#elif defined(COMPUTE_CL_2D)
			entropy::cmb::PoolCL2D pool;
#elif defined(COMPUTE_CL_3D)
			entropy::cmb::PoolCL3D pool;
#endif

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<ofFloatColor> tintColor{ "Tint Color", ofFloatColor::white };
				
				PARAM_DECLARE("CMB", tintColor);
			} parameters;
		};
	}
}