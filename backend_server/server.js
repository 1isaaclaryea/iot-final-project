const express = require('express');
const MongoClient = require('mongodb').MongoClient;
const app = express();
const cors = require('cors');
app.use(express.json());
app.use(cors());
const client = new MongoClient('mongodb://localhost:27017', { monitorCommands: true });

const sensorValsCollection = client.db('IoTFinalProject').collection('SensorVals');

app.get('/', function(req, res) {
    console.log(sensorValsCollection);
})

// search should be based on ID instead. Temporal implementation uses name
// app.get('/water-tanks/:name', async (req, res)=>{
//     let waterLevelReadings = await sen.find({"ownerName":req.params.name}).limit(5).toArray();
//     console.log(waterLevelReadings);
//     res.send(waterLevelReadings);
// })

app.post('/new-reading', async (req, res) => {
    console.log(req);
    const d = new Date();
    let _date = d.toUTCString();
    req.body.date = _date;
    let response = await sensorValsCollection.insertOne(req.body);
    res.status(200).send(response);
})

// app.post('/new-tank', async (req, res) => {
//     const tankCollection = client.db('WaterTank').collection('Tanks');
//     let response = await tankCollection.insertOne(req.body);
//     console.log(response);
//     res.send(response);
// })

// Modify to only return readings from one sensor. Or probably do that from the frontend side
app.get('/readings', async (req, res) => {
    let sensorData = await sensorValsCollection.find().toArray();
    console.log(sensorData);
    res.send(sensorData);
})

app.listen(8080, function() {
    console.log('Listening on port 8080 ...')
})