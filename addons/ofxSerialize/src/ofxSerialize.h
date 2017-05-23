#pragma once

#include "ofEasyCam.h"
#include "ofJson.h"

template<typename DataType>
static nlohmann::json & ofSerialize(nlohmann::json & json, const vector<DataType> & values, const string & name = "");
template<typename DataType>
static const nlohmann::json & ofDeserialize(const nlohmann::json & json, vector<DataType> & values, const string & name = "");

static nlohmann::json & ofSerialize(nlohmann::json & json, const ofEasyCam & easyCam, const string & name = "");
static const nlohmann::json & ofDeserialize(const nlohmann::json & json, ofEasyCam & easyCam, const string & name = "");

static nlohmann::json & ofSerialize(nlohmann::json & json, const ofCamera & camera, const string & name = "");
static const nlohmann::json & ofDeserialize(const nlohmann::json & json, ofCamera & camera, const string & name = "");

static nlohmann::json & ofSerialize(nlohmann::json & json, const ofNode & node, const string & name = "");
static const nlohmann::json & ofDeserialize(const nlohmann::json & json, ofNode & node, const string & name = "");

#include "ofxSerialize.inl"
