all: debug

release: release-i386 release-ppc
	cd build/Release/SunPinyin.app/Contents/MacOS; \
	    lipo -create SunPinyin-i386 SunPinyin-ppc -o SunPinyin; \
	    rm -f SunPinyin-i386 SunPinyin-ppc
	rm -f build/SunPinyin.app
	cd build ; ln -s Release/SunPinyin.app SunPinyin.app  
	cd ../..; ./configure
	
release-i386: 
	cd ../..; CC='gcc -arch i386' CXX='g++ -arch i386' ./configure
	xcodebuild -project SunPinyin.xcodeproj -configuration Release-i386 build
	cd build/Release/SunPinyin.app/Contents/MacOS; mv SunPinyin SunPinyin-i386

release-ppc: 
	cd ../..; CC='gcc -arch ppc' CXX='g++ -arch ppc' ./configure
	xcodebuild -project SunPinyin.xcodeproj -configuration Release-ppc build
	cd build/Release/SunPinyin.app/Contents/MacOS; mv SunPinyin SunPinyin-ppc

debug:
	xcodebuild -project SunPinyin.xcodeproj -configuration Debug build
	rm -f build/SunPinyin.app
	cd build ; ln -s Debug/SunPinyin.app SunPinyin.app

clean:
	rm -rf build

install:
	rm -rf "/Library/Input Methods/SunPinyin.app"
	cp -R build/Debug/SunPinyin.app "/Library/Input Methods"

