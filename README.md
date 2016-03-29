This repository contains all the software developed for the Entropy presentation.

Please note that at this point, these applications are unfinished proofs-of-concept, so run at your own risk!

Project files are included for Xcode (OS X) or Visual Studio 2015 (Windows). Most apps should run on both platforms, but note that some may be Windows only because of OpenGL limitations on OS X. The final set of applications will run on Windows, so that's always your best bet. All apps should also theoretically work on Linux, if anyone wants to give that a go.

## Setup

1. Clone [openFrameworks](https://github.com/Entropy/openFrameworks). Make sure to use the `Entropy` organisation fork to ensure all apps will run correctly.
1. Clone [Entropy](https://github.com/Entropy/Entropy) (this project) into the `openFrameworks/` directory.
1. Download all necessary addons.
	1. The easy way: Make sure you have [Python](https://www.python.org/) installed and run the script `python _update.py` from the command line.
	1. The hard way: Look through the `_dependencies.json` file and manually clone all addons in that list.
1. Try running some projects!

## Update

1. Make sure you have [Python](https://www.python.org/) installed and run the script `python _update.py` from the command line.

## Contribute

We'll be using the gitflow model in order to collaborate while keeping things stable. For more info, check out these links:

* http://nvie.com/posts/a-successful-git-branching-model/
* https://www.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow

Make sure any addons you are using are forked in the Entropy organization, and properly referenced in the `_dependencies.json` file. This will ensure everyone works off the same codebase.
