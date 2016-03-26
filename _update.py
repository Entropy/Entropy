#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import os
from pprint import pprint
import subprocess

def update_dep(local_dir, data_node):
	saved_cwd = os.getcwd()
	os.chdir(local_dir)
	print os.getcwd()

	if os.path.isdir(data_node["name"]):
		# Dependency already exists
		print "Updating " + data_node["name"] + "..."
		os.chdir(data_node["name"])
		
		if "branch" in data_node:
			# Make sure we're on the right branch
			proc = subprocess.Popen(["git", "branch"], stdout=subprocess.PIPE)
			output = proc.stdout.read()
			if data_node["branch"] not in output:
				subprocess.call(["git", "checkout", data_node["branch"]])

		if "commit" in data_node:
			# Check out the specified commit
			subprocess.call(["git", "checkout", data_node["commit"]])
		else:
			# Check out the latest commit
			if "branch" not in data_node:
				data_node["branch"] = "master"
			subprocess.call(["git", "pull", "origin", data_node["branch"]])

	else:
		# New dependency, clone
		print "Cloning " + data_node["name"] + "..."
		if "branch" not in data_node:
			data_node["branch"] = "master"

		subprocess.call(["git", "clone", "-b", data_node["branch"], data_node["source"], data_node["name"]])

		if "commit" in data_node:
			subprocess.call(["git", "checkout", data_node["commit"]])

	os.chdir(saved_cwd)

if __name__ == "__main__":
	with open("_deps.json") as deps_file:    
		deps_data = json.load(deps_file)

	base_cwd = os.getcwd()

	if "oF" in deps_data:
		update_dep("../../", deps_data["oF"])

	if "addons" in deps_data:
		for node in deps_data["addons"]:
			update_dep("../addons/", node)
