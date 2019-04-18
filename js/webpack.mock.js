const path = require("path");
const webpack = require("webpack");
const merge = require('webpack-merge');
const dev = require('./webpack.dev.js');
const mockapi = require('./mockapi/mockapi.js');
var bodyParser = require('body-parser');    

module.exports = merge(dev, {  
  devServer: {
    before: function(app, server) {
      const profiles = mockapi.profiles;
      app.use(bodyParser.json());
      app.get('/api/profiles', profiles.index);
      app.post('/api/profiles/', bodyParser.json(), profiles.create);
      app.put('/api/profiles/:name', bodyParser.json(), profiles.update);
      app.delete('/api/profiles/:name', bodyParser.json(), profiles.destroy);
      app.get('/api/profiles/active', profiles.active)
      app.get('/api/status', mockapi.status);
      app.get('/api/activate/:name', mockapi.activate);
    }
  }
});