const AWS = require('aws-sdk');
const crypto = require('crypto');
const express = require('express');

const router = express.Router();

const iot = new AWS.Iot({
  iot: '2015-05-28',
  region: 'us-east-2'
});

const getToken = thingName => {
  const now = new Date().getTime();
  const secret = crypto.randomBytes(12).toString('base64');
  const token =
    '{"secret":"' +
    secret +
    '","cmds":"lock,immo,can,inRide,reboot","start":' +
    Math.round(now / 1000) +
    ',"end":' +
    Math.round(now / 1000 + 10 * 60 * 60) +
    '}';
  return new Promise((resolve, reject) => {
    iot.listThingPrincipals({ thingName }, (err, data) => {
      if (err) return reject(err);
      // console.log(data);
      const tokens = data.principals[0].split('/');
      const certificateId = tokens[tokens.length - 1];
      iot.describeCertificate({ certificateId }, (err, data) => {
        if (err) return reject(err);
        const cert = data.certificateDescription.certificatePem.slice(743, 743 + 64);
        // console.log(cert);
        const buf = Buffer.from(cert, 'base64');
        const iv = buf.slice(0, 16);
        // console.log('iv: ', iv);
        const key = buf.slice(16, 48);
        // console.log('key: ', k);
        const cipher = crypto.createCipheriv('aes-256-cbc', key, iv);
        let encrypted = cipher.update(token);
        encrypted = Buffer.concat([encrypted, cipher.final()]);
        // console.log(encrypted);
        encrypted = encrypted.toString('base64');
        // console.log(encrypted);
        resolve({ token: encrypted, secret });
      });
    });
  });
};

// curl -X GET localhost:8080/token?thingName=0123B5829E389548EE
router.get('/token', async (req, res) => {
  const { thingName } = req.query;
  const jso = await getToken(thingName);
  res.status(200).send(jso);
});

module.exports = router;
