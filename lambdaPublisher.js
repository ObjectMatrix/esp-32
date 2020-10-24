const AWS = require('aws-sdk');
var iotdata = new AWS.IotData({endpoint: 'a3amyzmzkjgky3-ats.iot.us-east-1.amazonaws.com'});

exports.handler = async (event, context) => {
  var params = {
       topic: "esp32/sub",
       payload: event.headers.message,
       qos: 0
  }
  return {
    statusCode: 200,
            headers: {
            "Access-Control-Allow-Headers" : "*",
            "Access-Control-Allow-Origin": "*",
            "Access-Control-Allow-Methods": "OPTIONS,POST,GET,PUT"
          },
    body: JSON.stringify(await publishMessage(params))
  }
}

const publishMessage = async (params) => {
  return new Promise((resolve, reject) => {
    iotdata.publish(params, function(err, data){
      if(err){
        console.log(err);
        reject(err)
      }
      else{
        resolve(params)
      }
    })
  })
}
