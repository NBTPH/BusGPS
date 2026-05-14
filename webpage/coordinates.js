//This script will runs on the client, not the server
async function update_data(){
    try{
        const response = await fetch('/get_data');
        const msg_string = await response.json();

        document.getElementById('Vehicle_ID').innerText = msg_string.ID;

        const date = msg_string.Date;
        document.getElementById('Date').innerText = `${date.Day}/${date.Month}/${date.Year}`;

        const t = msg_string.UTC;
        const hrs = String(t.Hours).padStart(2, '0'); //pad to make it look nicer
        const mins = String(t.Minutes).padStart(2, '0');
        const secs = t.Seconds.toFixed(2); //keep 2 decimal places
        document.getElementById('UTC').innerText = `${hrs}:${mins}:${secs}`;

        document.getElementById('Lat').innerText = msg_string.lat;
        document.getElementById('Lon').innerText = msg_string.lon;
        document.getElementById('Heading').innerText = msg_string.lon;
        //we have to implement the html to know what id to use

        document.getElementById('Ignition').innerText = msg_string.Ignition ? "ON" : "OFF";
        document.getElementById('Door').innerText = msg_string.Door_Open ? "OPEN" : "CLOSED";
        document.getElementById('AC').innerText = msg_string.AC ? "ON" : "OFF";
    }
    catch(err){
        console.error('Failed to parse data', err);
    }
}

async function update_status(){
    try{
        const response = await fetch('/get_status');
        const status = await response.text(); //respond will be a character "1" or "0"
        
        const statusEl = document.getElementById('Status');
        if(status === "1"){ //if status is ok
            statusEl.innerText = "GPS FIXED - Live Data";
            statusEl.className = "status-good";
            update_data(); //call to get and update data
        }
        else{//if not
            statusEl.innerText = "NO GPS FIX - Waiting for satellites...";
            statusEl.className = "status-bad"; //display that GPS is not good
        }
    }
    catch(err){
        console.error('Connection to ESP32 lost', err);
        const statusEl = document.getElementById('Status');
        statusEl.innerText = "ESP32 DISCONNECTED";
        statusEl.className = "status-bad";
    }
}

setInterval(update_status, 200); //get data every 200ms
update_status()