# Source this script to set up the CF4Hep installation that this script is part of.
#
# This script if for the csh like shells, see thisdd4hep.sh for bash like shells.
#
# Author: Pere Mato

set ARGS=($_)
set THIS="`dirname ${ARGS[2]}`"


#----LD_LIBRARY_PATH-----------------------------------------------------------------
if ($?LD_LIBRARY_PATH) then
   setenv LD_LIBRARY_PATH /opt/intel/tbb/tbb40_297oss/lib/lib:$LD_LIBRARY_PATH      # Linux, ELF HP-UX
else
   setenv LD_LIBRARY_PATH /opt/intel/tbb/tbb40_297oss/lib/lib
endif

#----DYLD_LIBRARY_PATH---------------------------------------------------------------
if ($?DYLD_LIBRARY_PATH) then
   setenv DYLD_LIBRARY_PATH /opt/intel/tbb/tbb40_297oss/lib/lib:$DYLD_LIBRARY_PATH  # Mac OS X
else
   setenv DYLD_LIBRARY_PATH /opt/intel/tbb/tbb40_297oss/lib/lib
endif
