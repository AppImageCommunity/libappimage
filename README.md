# libappimage [![Build Status](https://travis-ci.org/libappimage/libappimage.svg?branch=master)](https://travis-ci.org/AppImage/libappimage) [![irc](https://img.shields.io/badge/IRC-%23AppImage%20on%20freenode-blue.svg)](https://webchat.freenode.net/?channels=AppImage) [![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=ZT9CL8M5TJU72)

This library is part of the [AppImage](https://github.com/AppImage/appimagekit/) project. It implements functionality for dealing with AppImage files. It is written in C++ and is using Boost.

## Availablility

libappimage is available in the following distributions:
https://repology.org/project/libappimage/versions

## Usage

As a user, you normally do not need to deal with this library. Tools that use it (like [the optional `appimaged` daemon](https://github.com/AppImage/appimaged)) usually come with a bundled copy of it.

## API documentation

As a developer interested in using libappimage in your projects, please find the API documentation here:
https://docs.appimage.org/api/libappimage/. Please note that if you are using libappimage in your project, we recommend bundling your private copy or linking statically to it, since the versions provided by distributions may be outdated.

## Building

If for some reason you need to do a local development build, on a deb-based system (tested on Ubuntu xenial) do:

```
sudo apt-get -y install automake cmake libtool libcairo-dev libfuse-dev git
git clone https://github.com/AppImage/libappimage
cd ./libappimage/
mkdir build
cmake .. -DBUILD_TESTING:bool=False
make
sudo make install
cd ..
```

## Contributing

Your contributions are welcome.

If you make or suggest changes to this code, please test that the resulting executables (like [the `appimaged` daemon](https://github.com/AppImage/appimaged)) are still working properly.


If you have questions, AppImage developers are on #AppImage on irc.freenode.net.
