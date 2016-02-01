#!/bin/bash

# We get the directory of the script (absolute path)
_scripts_dir=$( cd -P -- "$(dirname -- "$(command -v -- "$0")")" && pwd -P )

# We get the URL of the NewProject script in GATB-CORE
gatbcore_NewProject=$_scripts_dir/../thirdparty/gatb-core/gatb-core/scripts/NewProject/NewProject.sh

# We define the new project directory
if test -z "$2"

    then

        # We call the GATB-CORE NewProject script
        $gatbcore_NewProject  $1  "$_scripts_dir/../tools"  dummy 
                
    else
        # We call the GATB-CORE NewProject script
        $gatbcore_NewProject  $1  $2  
fi
