const addon = require('../build/Release/object-wrap-demo-native');
console.log("Exposed CPP APIs", addon);
module.exports = addon;
