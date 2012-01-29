all: debug

release:
	xcodebuild -project SunPinyin.xcodeproj -configuration Release build
	rm -f build/SunPinyin.app
	cd build ; ln -s Release/SunPinyin.app SunPinyin.app

debug:
	xcodebuild -project SunPinyin.xcodeproj -configuration Debug build
	rm -f build/SunPinyin.app
	cd build ; ln -s Debug/SunPinyin.app SunPinyin.app

install:
	rm -rf "/Library/Input Methods/SunPinyin.app"
	cp -R build/Debug/SunPinyin.app "/Library/Input Methods"

install-release:
	rm -rf "/Library/Input Methods/SunPinyin.app"
	cp -R build/Release/SunPinyin.app "/Library/Input Methods"

clean:
	rm -rf build


