#pragma once

#include "ofxPreset.h"
#include "ofShader.h"

namespace entropy
{
	namespace surveys
	{
		enum ExtraAttribute
		{
			Mass = 5,
			StarFormationRate = 6
		}; 
		
		class DataSet
		{
		public:
			struct SharedParams
				: ofParameterGroup
			{
				struct : ofParameterGroup
				{
					ofParameter<float> size{ "Size", 8.0f, 0.01f, 32.0f, ofParameterScale::Logarithmic };
					ofParameter<float> attenuation{ "Attenuation", 600.0f, 1.0f, 1000.0f };

					PARAM_DECLARE("Points",
						size,
						attenuation);
				} point;

				struct : ofParameterGroup
				{
					ofParameter<float> size{ "Size", 8.0f, 0.01f, 64.0f };
					ofParameter<float> attenuation{ "Attenuation", 600.0f, 1.0f, 1000.0f };
					ofParameter<float> alpha{ "Alpha", 1.0f, 0.0f, 1.0f };
					ofParameter<int> density{ "Density", 1000, 100, 100000 };

					PARAM_DECLARE("Shells",
						size,
						attenuation,
						alpha,
						density);
				} shell;

				struct : ofParameterGroup
				{
					ofParameter<float> geoScale{ "Geo Scale", 8.0f, 0.01f, 32.0f };
					ofParameter<float> alphaScale{ "Alpha Scale", 1.0f, 0.01f, 8.0f, ofParameterScale::Logarithmic };
					ofParameter<int> resolution{ "Resolution", 1, 1, 1000 };
					ofParameter<float> clipDistance{ "Clip Distance", 1000.0f, 0.0f, 5000.0f };
					ofParameter<float> clipSize{ "Clip Size", 0.0f, 1.0f, 1000.0f, ofParameterScale::Logarithmic };

					PARAM_DECLARE("Model",
						geoScale,
						alphaScale,
						resolution,
						clipDistance,
						clipSize);
				} model;

				struct : ofParameterGroup
				{
					ofParameter<float> maxDistance{ "Max Distance", 100.0f, 1.0f, 5000.0f };
					ofParameter<float> lockDistance{ "Min Lock Distance", 1.0f, 0.0f, 1000.0f };
					ofParameter<float> minMass{ "Min Mass", 1.0f, 0.0f, 1000.0f, ofParameterScale::Logarithmic };
					
					PARAM_DECLARE("Target",
						maxDistance,
						lockDistance,
						minMass);
				} target;

				PARAM_DECLARE("Shared",
					point,
					shell,
					model,
					target);
			};

		public:
			DataSet();
			~DataSet();

			void setup(const std::string & name, const std::string & format, size_t startIdx, size_t endIdx, const std::string & particleType);
			void clear();

			void drawPoints(ofShader & shader);
			void drawShells(ofShader & shader, SharedParams & sharedParams);
			void drawModels(ofShader & shader, const glm::mat4 & worldTransform, ofVboMesh & mesh, const ofCamera & camera, SharedParams & params);

			int getTargetIndex() const;
			glm::vec3 getTargetPosition() const;
			float getTargetMass() const;

			struct : ofParameterGroup
			{
				ofParameter<bool> renderPoints{ "Render Points", true };
				ofParameter<bool> renderShells{ "Render Shells", true };
				ofParameter<bool> renderModels{ "Render Models", false };
				ofParameter<float> cutRadius{ "Cut Radius", 0.0f, 0.0f, 1.0f };
				ofParameter<float> minRadius{ "Min Radius", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxRadius{ "Max Radius", 0.5f, 0.0f, 1.0f };
				ofParameter<float> minLatitude{ "Min Latitude", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxLatitude{ "Max Latitude", 0.5f, 0.0f, 1.0f };
				ofParameter<float> minLongitude{ "Min Longitude", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxLongitude{ "Max Longitude", 0.5f, 0.0f, 1.0f };
				ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };

				PARAM_DECLARE("DataSet", 
					renderPoints, renderShells, renderModels,
					cutRadius,
					minRadius, maxRadius, 
					minLatitude, maxLatitude,
					minLongitude, maxLongitude, 
					color);
			} parameters;

		protected:
			struct InstanceData
			{
				glm::mat4 transform;
				float alpha;
				float starFormationRate;
				glm::vec2 dummy;
			};

			std::size_t loadFragment(const std::string & filePath, const std::string & particleType);
			std::size_t updateFilteredData(const glm::mat4 & worldTransform, const ofCamera & camera, SharedParams & params);

			std::vector<glm::vec3> coordinates;
			std::vector<float> masses;
			std::vector<float> starFormationRates;

			float minRadius;
			float maxRadius;

			float minMass;
			float maxMass;
			float avgMass;

			int targetIndex;

			glm::vec3 mappedRadiusRange;
			glm::vec2 mappedLatitudeRange;
			glm::vec2 mappedLongitudeRange;

			ofVbo vbo;
			ofBufferObject bufferObj;

			std::vector<ofEventListener> paramListeners;
		};
	}
}