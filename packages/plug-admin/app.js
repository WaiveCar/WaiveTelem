const express = require('express');
const awsServerlessExpressMiddleware = require('aws-serverless-express/middleware');
const cors = require('cors');
const api = require('./api');

const app = express();
app.set('trust proxy', true);
app.use(cors());
app.use(awsServerlessExpressMiddleware.eventContext());
app.use('', api);

module.exports = app;
