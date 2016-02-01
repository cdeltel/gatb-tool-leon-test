# GIT management

The GATB-TOOLS project **relies on the GIT submodule GATB-CORE**.

If you read this, it is likely you have cloned the GATB-TOOLS repository with the following:

	git clone git+ssh://username@scm.gforge.inria.fr//gitroot/gatb-tools/gatb-tools.git
	
where _username_ is your login on the GIT repository.

Now, you need to configure the submodule GATB-CORE (at the root of your cloned GATB-TOOLS project):
	
	git submodule init

After that, you have to update GATB-CORE the submodule. Before doing this, you need to configure your _login name_ 
for this update. Actually, the submodule repository address known by GATB-TOOLS is an address with a _generic login name_,
and not with your own login. Without any configuration, your _submodule init_ would fail due to a repository access error.
Therefore, you need to do:

	git config submodule.gatb-tools/thirdparty/gatb-core.url git+ssh://username@scm.gforge.inria.fr//gitroot/gatb-core/gatb-core.git  

where you have to replace _username_ by your GIT login.

Finally, you can update the submodule with:

    git submodule update
	git submodule -q foreach git pull -q origin master

which should eventually clone the GATB-CORE submodule.

You can also have a look at [this link](http://stackoverflow.com/questions/6041516/git-submodule-update-with-other-user) for more details.

# Directories architecture

The main directory here is the _tools_ directory, where each tool is located in its own directory.


# Compiling existing tools

To compile an existing tool, for instance _TakeABreak_, 

	cd gatb-tools/tools/TakeABreak
	mkdir build
	cd build
	cmake ..
	make


# Adding a new tool

You can use the script _InitProject.sh_ with the name of your project as argument.
This will create for you a directory in the _tools_ directory, with the following structure:

* src            : directory for the source of your tool
* CMakeLists.txt : default cmake file for building the binary of your tool

Note that the generated CMakeLists.txt is configured to use the GATB-CORE submodule.

Example of use:

    sh scripts/NewProject.sh MyProject

    




