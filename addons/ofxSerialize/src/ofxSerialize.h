#pragma once

#include "ofEasyCam.h"
#include "ofJson.h"

static inline nlohmann::json & ofSerialize(nlohmann::json & json, const ofEasyCam & easyCam, const string & name = "");
static inline const nlohmann::json & ofDeserialize(const nlohmann::json & json, ofEasyCam & easyCam, const string & name = "");

static inline nlohmann::json & ofSerialize(nlohmann::json & json, const ofCamera & camera, const string & name = "");
static inline const nlohmann::json & ofDeserialize(const nlohmann::json & json, ofCamera & camera, const string & name = "");

static inline nlohmann::json & ofSerialize(nlohmann::json & json, const ofNode & node, const string & name = "");
static inline const nlohmann::json & ofDeserialize(const nlohmann::json & json, ofNode & node, const string & name = "");

#include "ofxSerialize.inl"
