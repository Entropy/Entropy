#include "ofxSerialize.h"

//--------------------------------------------------------------
inline nlohmann::json & ofSerialize(nlohmann::json & json, const ofEasyCam & easyCam, const string & name)
{
	auto & jsonGroup = ofSerialize(json, (ofCamera &)easyCam, name);

	jsonGroup["target"] = ofToString(easyCam.getTarget().getPosition());
	jsonGroup["distance"] = easyCam.getDistance();
	jsonGroup["drag"] = easyCam.getDrag();
	jsonGroup["mouseInputEnabled"] = easyCam.getMouseInputEnabled();
	jsonGroup["mouseMiddleButtonEnabled"] = easyCam.getMouseMiddleButtonEnabled();
	jsonGroup["translationKey"] = easyCam.getTranslationKey();
	jsonGroup["relativeYAxis"] = easyCam.getRelativeYAxis();
	jsonGroup["upAxis"] = ofToString(easyCam.getUpAxis());
	jsonGroup["inertiaEnabled"] = easyCam.getInertiaEnabled();

	return jsonGroup;
}

//--------------------------------------------------------------
inline const nlohmann::json & ofDeserialize(const nlohmann::json & json, ofEasyCam & easyCam, const string & name)
{
	if (!name.empty() && !json.count(name))
	{
		ofLogWarning(__FUNCTION__) << "Name " << name << " not found in JSON!";
		return json;
	}

	easyCam.setAutoDistance(false);

	const auto & jsonGroup = name.empty() ? json : json[name];

	try
	{
		const auto target = ofFromString<glm::vec3>(jsonGroup["target"]);
		easyCam.setTarget(target);
		easyCam.setDistance(jsonGroup["distance"]);
		easyCam.setDrag(jsonGroup["drag"]);
		jsonGroup["mouseInputEnabled"] ? easyCam.enableMouseInput() : easyCam.disableMouseInput();
		jsonGroup["mouseMiddleButtonEnabled"] ? easyCam.enableMouseMiddleButton() : easyCam.disableMouseMiddleButton();
		if (jsonGroup.count("translationKey"))
		{
			int translationKey = jsonGroup["translationKey"];
			easyCam.setTranslationKey(translationKey);
		}
		if (jsonGroup.count("relativeAxis")) easyCam.setRelativeYAxis(jsonGroup["relativeYAxis"]);
		if (jsonGroup.count("upAxis"))
		{
			const auto upAxis = ofFromString<glm::vec3>(jsonGroup["upAxis"]);
			easyCam.setUpAxis(upAxis);
		}
		if (jsonGroup.count("inertiaEnabled")) jsonGroup["inertiaEnabled"] ? easyCam.enableInertia() : easyCam.disableInertia();
	}
	catch (std::exception & exc)
	{
		ofLogError(__FUNCTION__) << exc.what();
	}

	ofDeserialize(jsonGroup, (ofCamera &)easyCam);

	return jsonGroup;
}

//--------------------------------------------------------------
inline nlohmann::json & ofSerialize(nlohmann::json & json, const ofCamera & camera, const string & name)
{
	auto & jsonGroup = ofSerialize(json, (ofNode &)camera, name);

	jsonGroup["fov"] = camera.getFov();
	jsonGroup["nearClip"] = camera.getNearClip();
	jsonGroup["farClip"] = camera.getFarClip();
	jsonGroup["lensOffset"] = ofToString(camera.getLensOffset());
	jsonGroup["aspectRatio"] = camera.getAspectRatio();
	jsonGroup["forceAspectRatio"] = camera.getForceAspectRatio();
	jsonGroup["ortho"] = camera.getOrtho();

	return jsonGroup;
}

//--------------------------------------------------------------
inline const nlohmann::json & ofDeserialize(const nlohmann::json & json, ofCamera & camera, const string & name)
{
	if (!name.empty() && !json.count(name))
	{
		ofLogWarning(__FUNCTION__) << "Name " << name << " not found in JSON!";
		return json;
	}

	const auto & jsonGroup = name.empty() ? json : json[name];

	try
	{
		camera.setFov(jsonGroup["fov"]);
		camera.setNearClip(jsonGroup["nearClip"]);
		camera.setFarClip(jsonGroup["farClip"]);
		const auto lensOffset = ofFromString<glm::vec2>(jsonGroup["lensOffset"]);
		camera.setLensOffset(lensOffset);
		camera.setForceAspectRatio(jsonGroup["forceAspectRatio"]);
		if (camera.getForceAspectRatio())
		{
			camera.setAspectRatio(jsonGroup["aspectRatio"]);
		}
		jsonGroup["ortho"] ? camera.enableOrtho() : camera.disableOrtho();
	}
	catch (std::exception & exc)
	{
		ofLogError(__FUNCTION__) << exc.what();
	}

	ofDeserialize(jsonGroup, (ofNode &)camera);

	return jsonGroup;
}

//--------------------------------------------------------------
inline nlohmann::json & ofSerialize(nlohmann::json & json, const ofNode & node, const string & name)
{
	auto & jsonGroup = name.empty() ? json : json[name];

	jsonGroup["position"] = ofToString(node.getPosition());
	jsonGroup["orientation"] = ofToString(node.getOrientationQuat());
	jsonGroup["scale"] = ofToString(node.getScale());

	return jsonGroup;
}

//--------------------------------------------------------------
inline const nlohmann::json & ofDeserialize(const nlohmann::json & json, ofNode & node, const string & name)
{
	if (!name.empty() && !json.count(name))
	{
		ofLogWarning(__FUNCTION__) << "Name " << name << " not found in JSON!";
		return json;
	}

	const auto & jsonGroup = name.empty() ? json : json[name];

	if (jsonGroup.count("position"))
	{
		const auto position = ofFromString<glm::vec3>(jsonGroup["position"]);
		node.setPosition(position);
	}
	if (jsonGroup.count("orientation"))
	{
		const auto orientation = ofFromString<glm::quat>(jsonGroup["orientation"]);
		node.setOrientation(orientation);
	}
	if (jsonGroup.count("scale"))
	{
		const auto scale = ofFromString<glm::vec3>(jsonGroup["scale"]);
		node.setScale(scale);
	}

	return jsonGroup;
}
