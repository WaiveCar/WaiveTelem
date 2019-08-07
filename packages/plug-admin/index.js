const AWS = require('aws-sdk');
const crypto = require('crypto');

const iot = new AWS.Iot({
  iot: '2015-05-28',
  region: 'us-east-2'
});

iot.listThingPrincipals({ thingName: '0123B5829E389548EE' }, (err, data) => {
  if (err) return console.log(err, err.stack);
  console.log(data);
  const tokens = data.principals[0].split('/');
  const certificateId = tokens[tokens.length - 1];
  iot.describeCertificate({ certificateId }, (err, data) => {
    if (err) return console.log(err, err.stack);
    const cert = data.certificateDescription.certificatePem.slice(743, 743 + 64);
    console.log(cert);
    const buf = Buffer.from(cert, 'base64');
    const iv = buf.slice(0, 16);
    console.log('iv: ', iv);
    const key = buf.slice(16, 48);
    console.log('key: ', key);
    const cipher = crypto.createCipheriv('aes-256-cbc', key, iv);
    let encrypted = cipher.update('{"cmds":"lock,immo","start":0,"end":1576186925}');
    encrypted = Buffer.concat([encrypted, cipher.final()]);
    console.log(encrypted);
    encrypted = encrypted.toString('base64');
    console.log(encrypted);
  });
});
