XREGEXP_PATH=angel-player/src/chrome/content/angelic/xregexp
cd $PROJECT_ROOT_DIR
git submodule update --init $XREGEXP_PATH
cd $PROJECT_ROOT_DIR
cd $XREGEXP_PATH
git apply ../xregexp.patch
