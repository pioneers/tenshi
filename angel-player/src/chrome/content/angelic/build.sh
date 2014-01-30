ANGELIC_PATH=angel-player/src/chrome/content/angelic
XREGEXP_PATH=$ANGELIC_PATH/xregexp
cd $PROJECT_ROOT_DIR
git submodule update --init $XREGEXP_PATH
cd $ANGELIC_PATH
#cd $PROJECT_ROOT_DIR
#cd $XREGEXP_PATH
#git reset --hard
#git apply ../xregexp.patch
browserify robot.js -s angelic -o bundle.js
