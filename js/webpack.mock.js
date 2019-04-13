const path = require("path");
const webpack = require("webpack");
const merge = require('webpack-merge');
const dev = require('./webpack.dev.js');
const profiles = require('./mockapi/profiles.js');
var bodyParser = require('body-parser');    

module.exports = merge(dev, {  
  devServer: {
    before: function(app, server) {
      app.use(bodyParser.json());
      app.get('/api/profiles', profiles.index);
      app.post('/api/profiles/', bodyParser.json(), profiles.create);
      app.put('/api/profiles/:name', bodyParser.json(), profiles.update);
      app.delete('/api/profiles/:name', bodyParser.json(), profiles.destroy);
    }
  }
});