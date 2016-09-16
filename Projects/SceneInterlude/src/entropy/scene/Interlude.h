#pragma once

#include "entropy/geom/Stripes.h"
#include "entropy/render/Layout.h"
#include "entropy/scene/Base.h"

namespace entropy
{
	namespace scene
	{
		class Interlude
			: public Base
		{
		public:
			string getName() const override
			{
				return "entropy::scene::Interlude";
			}

			Interlude();
			~Interlude();

			void init() override;
			void clear() override;

			void setup() override;
			void exit() override;

			void resizeBack(ofResizeEventArgs & args) override;
			void resizeFront(ofResizeEventArgs & args) override;

			void update(double dt) override;

			void drawFrontWorld() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void serialize(nlohmann::json & json) override;
			void deserialize(const nlohmann::json & json) override;

			static const int MAX_NUM_STRIPES = 8;

		protected:
			std::shared_ptr<geom::Stripes> addStripes();
			void removeStripes();

			std::vector<std::shared_ptr<geom::Stripes>> stripes;
			bool openGuis[MAX_NUM_STRIPES];  // Don't use vector<bool> because they're weird: http://en.cppreference.com/w/cpp/container/vector_bool

			virtual ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			ofParameterGroup parameters;
		};
	}
}
