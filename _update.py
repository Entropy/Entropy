#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import os
import subprocess
from sys import argv

def update_dep(local_path, data_node):
	# Save the working directory to return to it at the end of the function.
	saved_cwd = os.getcwd()
	os.chdir(local_path)
	print os.getcwd()

	if os.path.isdir(data_node['name']):
		# Dependency already exists, try updating.
		print 'Updating ' + data_node['name'] + '...'

		os.chdir(data_node['name'])
		
		if 'branch' in data_node:
			# Make sure we're on the right branch.
			proc = subprocess.Popen(['git', 'branch'], stdout=subprocess.PIPE)
			output = proc.stdout.read()
			if data_node['branch'] not in output:
				subprocess.call(['git', 'checkout', data_node['branch']])

		if 'commit' in data_node:
			# Check out the specified commit.
			subprocess.call(['git', 'checkout', data_node['commit']])
		else:
			# Pull the latest commit.
			if 'branch' not in data_node:
				data_node['branch'] = 'master'
			subprocess.call(['git', 'pull', 'origin', data_node['branch']])

	else:
		# New dependency, clone it.
		print 'Cloning ' + data_node['name'] + '...'
		if 'branch' not in data_node:
			data_node['branch'] = 'master'

		subprocess.call(['git', 'clone', '-b', data_node['branch'], data_node['source'], data_node['name']])

		if 'commit' in data_node:
			# Check out the specified commit.
			subprocess.call(['git', 'checkout', data_node['commit']])

	# Set the working directory back to its original value.
	os.chdir(saved_cwd)

if __name__ == '__main__':
	if len(argv) == 1:
		# Default parameters, assume this script is in oF/apps/myApps/myProject/
		script = argv
		root_path = '../../../'
		deps_path = '_dependencies.json'
	elif len(argv) == 2:
		script, root_path = argv
		deps_path = '_dependencies.json'
   	elif len(argv) == 3:
		script, root_path, deps_path = argv
	else:
		print 'Usage is [oF Root Path] [Dependencies]'
		exit()

	# Open the dependencies file.
	with open(deps_path) as deps_file:    
		deps_data = json.load(deps_file)

	# Update openFrameworks.
	if 'oF' in deps_data:
		update_dep(os.path.join(root_path, '..'), deps_data['oF'])

	# Update the addons.
	if 'addons' in deps_data:
		for node in deps_data['addons']:
			update_dep(os.path.join(root_path, 'addons'), node)
