const AWS = require('aws-sdk');
const AWSMqttClient = require('aws-mqtt/lib/NodeClient');
// const winston = require('winston');
// const { Loggly } = require('winston-loggly-bulk');

// require('winston-papertrail').Papertrail;
// var winstonPapertrail = new winston.transports.Papertrail({
//   host: 'logs2.papertrailapp.com', // you get this from papertrail account
//   port: 20817 //you get this from papertrail account
// });

// winstonPapertrail.on('error', function(err) {
//   // Handle, report, or silently ignore connection errors and failures
// });

// var logger = winston.createLogger({
//   transports: [winstonPapertrail]
// });

// winston.add(
//   new Loggly({
//     token: '8aab8a9c-a254-4e10-8f8b-f36f993c95d4',
//     subdomain: 'ravelab',
//     tags: ['Winston-NodeJS'],
//     json: true
//   })
// );

const { logEventsToConsole } = require('./utils');

AWS.config.region = 'us-east-2'; // Region
AWS.config.credentials = new AWS.CognitoIdentityCredentials({
  IdentityPoolId: 'us-east-2:a0e5ce63-b8a8-4037-ab0c-69a03a60c6cc'
});

const client = new AWSMqttClient({
  region: AWS.config.region,
  credentials: AWS.config.credentials,
  endpoint: 'a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com', // NOTE: get this value with `aws iot describe-endpoint`
  clientId: 'mqtt-client-' + Math.floor(Math.random() * 100000 + 1), // clientId to register with MQTT broker. Need to be unique per client
  will: {
    topic: 'WillMsg',
    payload: 'Connection Closed abnormally..!',
    qos: 0,
    retain: false
  }
});

client.on('connect', () => {
  client.subscribe('things/+/log');
});

client.on('message', (topic, message) => {
  const str = message.toString();
  console.log(topic, str);
  // winston.log('info', str);
  // logger.info(JSON.parse(str));
});

logEventsToConsole(client);
