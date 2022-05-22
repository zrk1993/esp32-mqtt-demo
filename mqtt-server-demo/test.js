const mqtt = require('mqtt')
const client = mqtt.connect('mqtt://192.168.31.246')

client.on('connect', function () {
  client.subscribe('outTopic', () => {
    console.log('outTopic')
  })
})

client.on('message', function (topic, message) {
  // message is Buffer
  console.log(message.toString())

  client.publish("inTopic", "test pub");
})
