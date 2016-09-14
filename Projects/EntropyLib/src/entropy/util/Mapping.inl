#include "Mapping.h"

namespace entropy
{
	namespace util
	{
		//--------------------------------------------------------------
		const std::string & AbstractMapping::getShortName() const
		{
			return this->shortName;
		}

		//--------------------------------------------------------------
		const std::string & AbstractMapping::getGroupName() const
		{
			return this->groupName;
		}

		//--------------------------------------------------------------
		const std::string & AbstractMapping::getTrackName() const
		{
			return this->trackName;
		}

		//--------------------------------------------------------------
		const std::string & AbstractMapping::getPageName() const
		{
			return this->pageName;
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		Mapping<ParameterType, TrackType>::Mapping()
			: track(nullptr)
		{
			this->animated.set(false);
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		Mapping<ParameterType, TrackType>::~Mapping()
		{}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		void Mapping<ParameterType, TrackType>::setup(std::shared_ptr<ofParameter<ParameterType>> parameter, const std::string & pageName)
		{
			this->parameter = parameter;

			this->shortName = parameter->getName();

			const auto groupNames = parameter->getGroupHierarchyNames();

			// Use the top-level for the Group name.
			this->groupName = groupNames.front();

			// Cascade through the hierarchy for the GUI name.
			string paramName = "";
			this->trackName = "";
			for (auto i = 0; i < groupNames.size() - 1; ++i)
			{
				paramName.append(groupNames[i] + "::");
				this->trackName.append(groupNames[i] + "_");
			}
			paramName.append(this->shortName);
			this->trackName.append(this->shortName);

			this->animated.setName(paramName);

			this->pageName = pageName;
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		void Mapping<ParameterType, TrackType>::update()
		{
			if (this->track)
			{
				const auto & trackInfo = typeid(TrackType); 
				if (trackInfo == typeid(ofxTLSwitches))
				{
					auto trackSwitches = dynamic_cast<ofxTLSwitches *>(this->track);
					this->parameter->set(trackSwitches->isOn());
				}
				else if (trackInfo == typeid(ofxTLColorTrack))
				{
					auto trackColor = dynamic_cast<ofxTLColorTrack *>(this->track);
					auto parameterColor = dynamic_pointer_cast<ofParameter<ofFloatColor>>(this->parameter);
					parameterColor->set(trackColor->getColor());
				}
				else 
				{
					this->parameter->set(this->track->getValue());
				}
			}
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		void Mapping<ParameterType, TrackType>::addTrack(std::shared_ptr<ofxTimeline> timeline)
		{
			if (this->track)
			{
				//ofLogNotice(__FUNCTION__) << "Track for ofParameter " << this->trackName << " already exists!";
				return;
			}

			// Add Page if it doesn't already exist.
			if (!timeline->hasPage(pageName))
			{
				timeline->addPage(pageName);
			}
			timeline->setCurrentPage(pageName);
			
			if (timeline->hasTrack(this->trackName))
			{
				//ofLogWarning("Mapping::addTrack") << "Track for ofParameter " << this->trackName << " already exists!";
				return;
			}

			// Add Track and set default value and range where necessary.
			const auto & trackInfo = typeid(TrackType);
			if (trackInfo == typeid(ofxTLCurves))
			{
				this->track = timeline->addCurves(this->trackName);

				const auto & paramInfo = typeid(ParameterType);
				if (paramInfo == typeid(float))
				{
					auto parameterFloat = dynamic_pointer_cast<ofParameter<float>>(this->parameter);
					this->track->setValueRange(ofRange(parameterFloat->getMin(), parameterFloat->getMax()));
					this->track->setDefaultValue(parameterFloat->get());
				}
				else if (paramInfo == typeid(int))
				{
					auto parameterInt = dynamic_pointer_cast<ofParameter<int>>(this->parameter);
					this->track->setValueRange(ofRange(parameterInt->getMin(), parameterInt->getMax()));
					this->track->setDefaultValue(parameterInt->get());
				}
			}
			else if (trackInfo == typeid(ofxTLSwitches))
			{
				this->track = timeline->addSwitches(this->trackName);

				auto parameterBool = dynamic_pointer_cast<ofParameter<bool>>(this->parameter);
				this->track->setDefaultValue(parameterBool->get());
			}
			else if (trackInfo == typeid(ofxTLColorTrack))
			{
				auto trackColor = timeline->addColors(this->trackName);

				auto parameterColor = dynamic_pointer_cast<ofParameter<ofFloatColor>>(this->parameter);
				trackColor->setDefaultColor(parameterColor->get());

				this->track = trackColor;
			}

			//this->track->setDisplayName(this->shortName);
		}

		//--------------------------------------------------------------
		template<typename ParameterType, typename TrackType>
		void Mapping<ParameterType, TrackType>::removeTrack(std::shared_ptr<ofxTimeline> timeline)
		{
			if (!this->track)
			{
				//ofLogNotice(__FUNCTION__) << "Track for ofParameter " << this->trackName << " does not exist!";
				return;
			}
			
			timeline->removeTrack(this->track);
			this->track = nullptr;

			// EZ: This is broken
			//auto page = timeline->getPage(this->pageName);
			//if (page && page->getTracks().empty())
			//{
			//	timeline->removePage(page);
			//}
		}
	}
}