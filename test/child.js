const ObjectWrapDemo = require("../lib/binding.js");

ObjectWrapDemo.startLoop();

process.on('message', message => {
  if (message.command === 'init') {
    ObjectWrapDemo.config(message.args);

    ObjectWrapDemo.init((args) => {
      process.send({ event: 'audio', args });
    });
  
    ObjectWrapDemo.auth();
  }
});