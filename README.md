# esp32 reflow-toaster

Reflow toaster oven controller for ESP32, targetting in particular the Heltec Wifi 32
development kit with OLED screen. 

## Development Notes

### Javascript front-end

A little React application provides the user interface via HTTP. It lives in the `js` 
directory, and is bundled using webpack into a single file. This file, as well as an 
`index.html` file are written to a SPIFFS partition to be served by the HTTP server. 

#### Building the webpack bundle

You'll need a recent-ish node environment, with yarn installed. 

`yarn install` to install the depdendencies. 

`yarn build` to build the productin bundle. 

This will create `dist/index.html` and `dist/main.js`, which are the files stored to
SPIFFS. The SPIFFS image is created during build, and programmed during `make flash`, 
using the files found in `clientjs-dist` directory. In order to update the client
to be programmed, you must manually copy the required files from `js/dist` to `jsclient-dist`. 

The files in jsclient-dist are committed, so that it is possible to build the C++
project without requiring a node environment as well.

NOTE: There is a line in the Makefile to build the jsclient partition image, however
I found that I needed to checkout the lastest `master` branch of ESP-IDF for this 
to work. The latest release as of this writing was `3.2-rc`, and this did not support 
the `spiffs_create_partition_image` correctly.

#### Webpack development server

For working on the javascript, it is convenient to run it locally. The webpack dev
server can be run by `yarn start`. See `webpack.dev.js` for configuratin options; you 
will need to setup the proxy target to forward API requests through to the target 
device

NOTE: My experience has been that using the mDNS host name (i.e. reflow.local) for
the proxy target results in very slow (>5s) request times for the proxied requests. 
I guess this is because the mDNS request is slow, and the proxy must not be caching 
the result. All I know is that setting the target by IP address results in much faster
requests.

