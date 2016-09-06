#pragma once

#include "ofMain.h"
#include "ofxPreset.h"
#include "ofxTimeline.h"

namespace entropy
{
	namespace util
	{
		static string kMappingsTimelinePageName = "Mappings";

		class AbstractMapping
		{
		public:
			virtual void update() = 0;

			virtual void addTrack(std::shared_ptr<ofxTimeline> timeline) = 0;
			virtual void removeTrack(std::shared_ptr<ofxTimeline> timeline) = 0;

			const inline std::string & getName() const;
			//const inline std::string & getPageName() const;
			const inline std::string & getTrackName() const;

			ofParameter<bool> animated;

		protected:
			std::string name;
			std::string pageName;
			std::string trackName;
		};
		
		template<typename ParameterType, typename TrackType>
		class Mapping
			: public AbstractMapping
		{
		public:
			Mapping();
			~Mapping();

			void setup(std::shared_ptr<ofParameter<ParameterType>> parameter);
			void update() override;

			void addTrack(std::shared_ptr<ofxTimeline> timeline) override;
			void removeTrack(std::shared_ptr<ofxTimeline> timeline) override;

		protected:
			std::shared_ptr<ofParameter<ParameterType>> parameter;

			ofxTLKeyframes * track;
		};
	}
}

#include "Mapping.inl"
