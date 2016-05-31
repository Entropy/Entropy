#include "Mapping.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		const string & AbstractMapping::getName() const
		{
			return this->name;
		}

		//--------------------------------------------------------------
		const string & AbstractMapping::getPageName() const
		{
			return this->pageName;
		}

		//--------------------------------------------------------------
		const string & AbstractMapping::getTrackName() const
		{
			return this->trackName;
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		Mapping<ParameterType, TrackType>::Mapping()
		{
			this->track = nullptr;
			this->animated.set(false);
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		Mapping<ParameterType, TrackType>::~Mapping()
		{

		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		void Mapping<ParameterType, TrackType>::setup(shared_ptr<ofParameter<ParameterType>> parameter)
		{
			this->parameter = parameter;

			this->trackName = parameter->getName();

			const auto groupNames = parameter->getGroupHierarchyNames();

			// Use the last group name for the Page name.
			if (groupNames.size() > 2)
			{
				this->pageName = groupNames.at(groupNames.size() - 2);
			}

			// Skip first parent (which should be the Scene) for the GUI name.
			this->name = "";
			for (auto i = 1; i < groupNames.size() - 1; ++i)
			{
				this->name.append(groupNames[i] + "::");
			}
			this->name.append(this->trackName);

			this->animated.setName(this->name);
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		void Mapping<ParameterType, TrackType>::update()
		{
			if (this->track)
			{
				this->parameter->set(this->track->getValue());
			}
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		void Mapping<ParameterType, TrackType>::addTrack(ofxTimeline & timeline)
		{
			if (!timeline.hasPage(this->pageName))
			{
				timeline.addPage(this->pageName);
			}
			auto page = timeline.getPage(this->pageName);

			const auto pageTrackName = this->pageName + "_" + this->trackName;

			if (page->getTrack(pageTrackName))
			{
				ofLogWarning("Mapping::addTrack") << "Track for ofParameter " << this->trackName << " already exists!";
				return;
			}

			timeline.setCurrentPage(this->pageName);

			const auto & info = typeid(TrackType);
			if (info == typeid(ofxTLCurves))
			{
				this->track = timeline.addCurves(pageTrackName, ofRange(this->parameter->getMin(), this->parameter->getMax()));
			}
			else if (info == typeid(ofxTLSwitches))
			{
				this->track = timeline.addSwitches(pageTrackName);
			}
			this->track->setDisplayName(this->trackName);
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		void Mapping<ParameterType, TrackType>::removeTrack(ofxTimeline & timeline)
		{
			if (!this->track)
			{
				ofLogWarning("Mapping::removeTrack") << "Track for ofParameter " << this->trackName << " does not exist!";
				return;
			}
			
			timeline.removeTrack(this->track);
			this->track = nullptr;

			auto page = timeline.getPage(this->pageName);
			if (page && page->getTracks().empty())
			{
				timeline.removePage(page);
			}
		}
	}
}