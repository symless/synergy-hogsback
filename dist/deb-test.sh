export PKG_CONFIG_PATH=$WORKSPACE/predist/lib/pkgconfig/:$PKG_CONFIG_PATH
export DEB_BUILD_OPTIONS="parallel=8"
export DEB_CPPFLAGS_SET=""
export BOOST_ROOT=/home/nick/boost_1_65_1

SYNERGY_VERSION="2.0.0"
SYNERGY_REVISION="snapshot"
SYNERGY_DEB_VERSION="2.0.0.snapshot~b1+1"

dch --create --package "synergy" --controlmaint --distribution unstable --newversion $SYNERGY_DEB_VERSION "Initial release"

export DEB_BUILD_OPTIONS="parallel=8"
debuild --preserve-envvar PKG_CONFIG_PATH --preserve-envvar GIT_COMMIT --preserve-envvar BOOST_ROOT --preserve-envvar BUILD_NUMBER --preserve-envvar SYNERGY_* 
