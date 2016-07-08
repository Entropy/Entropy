#pragma once

#include "ofMain.h"
#include "ofxPreset.h"
#include "ofxTimeline.h"

namespace entropy
{
	namespace util
	{
		class AbstractMapping
		{
		public:
			virtual void update() = 0;

			virtual void addTrack(ofxTimeline & timeline) = 0;
			virtual void removeTrack(ofxTimeline & timeline) = 0;

			const inline string & getName() const;
			const inline string & getPageName() const;
			const inline string & getTrackName() const;

			ofxPreset::Parameter<bool> animated;

		protected:
			string name;
			string pageName;
			string trackName;
		};
		
		template<typename ParameterType, typename TrackType>
		class Mapping
			: public AbstractMapping
		{
		public:
			Mapping();
			~Mapping();

			void setup(shared_ptr<ofParameter<ParameterType>> parameter);
			void update() override;

			void addTrack(ofxTimeline & timeline) override;
			void removeTrack(ofxTimeline & timeline) override;

		protected:
			shared_ptr<ofParameter<ParameterType>> parameter;

			ofxTLKeyframes * track;
		};
	}
}

#include "Mapping.inl"
