//This script will runs on the client, not the server
async function update_data() {
    try{
        const response = await fetch('/get_data');
        const msg_string = await response.json();
        document.getElementById('Lat').innerText = msg_string.lat;
        document.getElementById('Lon').innerText = msg_string.lon;
        document.getElementById('Heading').innerText = msg_string.lon;
    }
    catch(err){
        console.error('Connection to ESP32 lost', err);
    }
}

setInterval(update_data, 200);
updateVar();